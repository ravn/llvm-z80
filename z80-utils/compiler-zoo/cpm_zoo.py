#!/usr/bin/env python3
"""CP/M three-compiler oracle: dcc vs clang(llvm-z80) vs zsdcc.

Compiles the dcc C test corpus (github.com/davidly/dcc, tests/*.c) with three
Z80 compilers targeting CP/M 2.2 and reports, per test:

  * size      — final .COM bytes (code + the runtime each toolchain links)
  * raw       — the test translation unit's own code+rodata (NO runtime):
                  clang : sum of .text*/.rodata* in the test .o
                  zsdcc : code_*/rodata_* sections from z88dk-z80nm
                  dcc   : __BSSB (end of code+data) from the M80 listing
  * tstates   — execution time via z88dk-ticks (CP/M page-zero BDOS stub)
  * verdict   — output cross-check by CONSENSUS (AGREE / SOLO / DIFF / FAIL):
                dcc lacks %ld/%lu, so it is a speed/size oracle but NOT a
                trustworthy correctness oracle — a compiler is AGREE when its
                console output matches at least one OTHER compiler.

Why a separate driver from compare.py: the bench_*.c programs compare.py was
built for are bare-metal (CRT_ORG 0x0000, compute -> HALT, result in HL, Docker
toolchains).  The dcc corpus is CP/M apps (printf, main() returns, native dcc +
M80/L80).  This driver provides the CP/M run path; compare.py stays as-is.

Usage:
  ./cpm_zoo.py                      # curated default subset, table
  ./cpm_zoo.py --all                # the full curated runnable corpus
  ./cpm_zoo.py sieve e nqueens      # specific tests
  ./cpm_zoo.py --csv [tests...]     # CSV instead of a table
  ./cpm_zoo.py --compilers dcc,clang  # subset of compilers

Paths are overridable via env (defaults match the macbook layout):
  DCC_DIR     /Users/ravn/z80/dcc
  LLVM_Z80    /Users/ravn/z80/llvm-z80
  Z88DK       /Users/ravn/z80/z88dk
  VCPM_JAR    /Users/ravn/z80/cpnet-z80/tools/VirtualCpm.jar
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile

# --------------------------------------------------------------------------
# Paths
# --------------------------------------------------------------------------

HOME = os.path.expanduser("~")
def _root():
    for r in ("/Users/ravn/z80", "/home/ravn/z80"):
        if os.path.isdir(r):
            return r
    return os.path.join(HOME, "z80")
ROOT = _root()

DCC_DIR  = os.environ.get("DCC_DIR",  os.path.join(ROOT, "dcc"))
LLVM_Z80 = os.environ.get("LLVM_Z80", os.path.join(ROOT, "llvm-z80"))
Z88DK    = os.environ.get("Z88DK",    os.path.join(ROOT, "z88dk"))
VCPM_JAR = os.environ.get("VCPM_JAR", os.path.join(ROOT, "cpnet-z80/tools/VirtualCpm.jar"))

CPM_DIR  = os.path.join(LLVM_Z80, "z80-utils/cpm")
TICKS    = os.path.join(Z88DK, "bin/z88dk-ticks")
ZCC      = os.path.join(Z88DK, "bin/zcc")
Z80NM    = os.path.join(Z88DK, "bin/z88dk-z80nm")
ZCCCFG   = os.path.join(Z88DK, "lib/config")

def _clang_build():
    for d in ("build-macos/bin", "build-linux/bin", "build/bin"):
        p = os.path.join(LLVM_Z80, d)
        if os.path.isdir(p):
            return p
    return os.path.join(LLVM_Z80, "build/bin")
CLANG_BUILD = os.environ.get("CLANG_BUILD", _clang_build())
CLANG   = os.path.join(CLANG_BUILD, "clang")
LLD     = os.path.join(CLANG_BUILD, "ld.lld")
OBJCOPY = os.path.join(CLANG_BUILD, "llvm-objcopy")
OBJDUMP = os.path.join(CLANG_BUILD, "llvm-objdump")
Z80_RT  = os.path.join(os.path.dirname(CLANG_BUILD), "lib/z80/z80_rt.a")

WORK = os.path.join(os.path.dirname(os.path.abspath(__file__)), "output", "cpm")
MAX_TSTATES = int(os.environ.get("MAX_TSTATES", "300000000"))
VCPM_TIMEOUT = int(os.environ.get("VCPM_TIMEOUT", "25"))

# --------------------------------------------------------------------------
# Corpus: per-test command-line args and required fixture files (from dcc's
# runall.sh).  A test absent from ARGS runs with no args.
# --------------------------------------------------------------------------

ARGS = {
    "ttt": "10", "pint": "e.pas", "cobint": "e.cob", "forint": "e.for",
    "adaint": "e.ada", "bint": "e.bas", "fint": "e.f", "cint": "eu.cin",
    "wumpus": "-c", "tchess": "-c", "a1": "-l:HELLO.BAS",
    "targs": "a bb ccc dddd eeeee",
}
# Per-app C stack reserve overrides (dcc default 512).
STACK = {"triangle": 768, "cobint": 1536}

# Curated default subset: portable, no-fixture, all three compilers can attempt.
DEFAULT_TESTS = ["sieve", "e", "nqueens", "fact", "triangle", "ttt",
                 "tqsort", "tbsearch", "tsetjmp", "tmalloch", "tstring"]

# The full curated runnable corpus (dcc runall.sh APPLIST).
FULL_TESTS = """sieve e ttt tstruct trw tstr tbug tprintf ts tcmp tunary tlong
tpi mm tm tfio wumpus triangle fileops nqueens fact tsetjmp tenum tunion tgoto
tarray tchess targs tstdc tvariad tsprintf tscanf tdowhile tmuldiv tbdos tdirent
tc89core tc89uac tc89init tc89fp tc89ptr tc89size tc89decl tc89qual tc89comp
tc89swjt tc89bit tc89pp tc89fnty tc89flt tc89fltc tc89flta tc89fptr tc89fs
tc89fcmp tc89fcnv tc89fadd tc89fmul tc89fdiv tc89ffio tc89flng tc89fmat ttrig
tlog tphi tap cpmenumd tbits tfo pihex tstrify tlcont primes tpreproc trwold
tlimits spsmash tcrcfix trtl2 tsyntax tstr2 tstr3 tstring tlongaud tlongreg
tlongopt tctxops tppreg tinitreg ttypesr ttype2 tdecinit tmalloch tallocx
tstdlib tioerr tqsort tbsearch trw2 terrno tpostfld tswitch tppifcom tpostidx
tpostut tbug2 tlongsub treg tret tstructv tstructi tstructp tstri2 tunion2
tbitfld tcnstfld tpromo tkandr tc89ini2 tdecl tctype tifcom tptrdiff tmulpow2
toffset tc89fini tmod3216 tpromo2 tunaryp tstfield pint cobint forint bint fint
cint adaint tstretst tportio tlongidx tforsco tforblk tcmt99 tc99scpe tctxflt
tmathf tstrconv tfarrsub t2darr too tzpad tesc tkbd tstackov tasm tcodegen a1""".split()

# --------------------------------------------------------------------------
# Shell helpers
# --------------------------------------------------------------------------

def run(cmd, **kw):
    """Run a command list, capture output, never raise."""
    kw.setdefault("capture_output", True)
    kw.setdefault("text", True)
    try:
        return subprocess.run(cmd, **kw)
    except Exception as e:  # noqa
        class R:  # minimal stand-in
            returncode = 1; stdout = ""; stderr = str(e)
        return R()

def src_path(name):
    p = os.path.join(DCC_DIR, "tests", f"{name}.c")
    return p if os.path.isfile(p) else None

def stage_fixtures(rundir):
    """Copy every non-source fixture from tests/ into rundir under UPPERCASE
    names (CP/M uppercases opened filenames)."""
    tdir = os.path.join(DCC_DIR, "tests")
    for f in os.listdir(tdir):
        base = f
        if base.endswith((".c", ".json", ".md")) or base.startswith("."):
            continue
        src = os.path.join(tdir, f)
        if os.path.isfile(src):
            shutil.copyfile(src, os.path.join(rundir, base.upper()))

# --------------------------------------------------------------------------
# CP/M run helpers (ticks for T-states, vcpm for output)
# --------------------------------------------------------------------------

PAGE_ZERO = None
def _page_zero():
    """65536-byte CP/M image prefix: warm-boot at 0, BDOS->0xDC00 mini-BDOS
    (fn 0 terminates -> JP 0 so ticks -end 0 stops; others RET)."""
    mem = bytearray(65536)
    mem[0x0000:0x0003] = bytes([0xC3, 0x00, 0x00])         # JP 0
    mem[0x0005:0x0008] = bytes([0xC3, 0x00, 0xDC])         # JP 0xDC00
    mem[0xDC00:0xDC06] = bytes([0x79, 0xB7, 0xCA, 0x00, 0x00, 0xC9])
    return mem

def measure_tstates(com_path):
    mem = bytearray(_page_zero())
    with open(com_path, "rb") as f:
        data = f.read()
    mem[0x0100:0x0100 + len(data)] = data
    img = com_path + ".img"
    with open(img, "wb") as f:
        f.write(mem)
    r = run([TICKS, "-pc", "100", "-end", "0", "-counter", str(MAX_TSTATES), img])
    last = (r.stdout or "").strip().splitlines()
    if last and last[-1].strip().isdigit():
        return int(last[-1].strip())
    return None

def run_vcpm(com_path, args=""):
    """Run a .COM via vcpm; return normalized console output (CR stripped,
    prompt line dropped).  Hard timeout + stdin=/dev/null so it never hangs."""
    stem = os.path.basename(com_path)
    if stem.upper().endswith(".COM"):
        stem = stem[:-4]
    root = tempfile.mkdtemp(prefix="vcpmroot_")
    home = tempfile.mkdtemp(prefix="vcpmhome_")
    try:
        os.symlink(os.path.dirname(com_path), os.path.join(root, "a"))
        with open(os.path.join(home, ".vcpmrc"), "w") as f:
            f.write(f"vcpm_root_dir = {root}\nvcpm_dso = def,a:,b,c\nsilent\n")
        cmd = ["java", f"-Duser.home={home}", "-jar", VCPM_JAR, stem]
        if args:
            cmd += args.split()
        try:
            r = subprocess.run(cmd, capture_output=True, text=True,
                               stdin=subprocess.DEVNULL, timeout=VCPM_TIMEOUT)
            out = r.stdout
        except Exception:  # noqa  (timeout etc.)
            out = ""
        lines = out.replace("\r", "").splitlines()
        return "\n".join(lines[1:]) if len(lines) > 1 else ""
    finally:
        shutil.rmtree(root, ignore_errors=True)
        shutil.rmtree(home, ignore_errors=True)

# --------------------------------------------------------------------------
# Build recipes (one .COM per compiler)
# --------------------------------------------------------------------------

CLANG_CFLAGS = ["--target=z80", "-Os", "-fno-builtin",
                "-ffunction-sections", "-fdata-sections",
                "-nostdlib", "-nostartfiles", "-I", CPM_DIR]

def build_clang(name, outdir):
    src = src_path(name)
    if not src:
        return None
    objs = {}
    for unit, path in (("crt0", os.path.join(CPM_DIR, "cpm_crt0.s")),
                       ("io",   os.path.join(CPM_DIR, "cpm_io.c")),
                       ("std",  os.path.join(CPM_DIR, "cpm_stdlib.c")),
                       ("test", src)):
        o = os.path.join(outdir, f"clang_{unit}.o")
        r = run([CLANG] + CLANG_CFLAGS + ["-c", path, "-o", o])
        if r.returncode != 0 or not os.path.exists(o):
            return None
        objs[unit] = o
    elf = os.path.join(outdir, "clang.elf")
    com = os.path.join(outdir, f"CLANG_{name.upper()}.COM")
    r = run([LLD, "--gc-sections", "-T", os.path.join(CPM_DIR, "cpm.ld"),
             objs["crt0"], objs["io"], objs["std"], objs["test"], Z80_RT,
             "-o", elf])
    if r.returncode != 0 or not os.path.exists(elf):
        return None
    r = run([OBJCOPY, "-O", "binary", "--only-section=.text", elf, com])
    return com if (r.returncode == 0 and os.path.exists(com)) else None

def build_zsdcc(name, outdir):
    src = src_path(name)
    if not src:
        return None
    out = os.path.join(outdir, f"ZSDCC_{name.upper()}")
    com = out + ".COM"
    env = dict(os.environ, PATH=os.path.join(Z88DK, "bin") + ":" + os.environ["PATH"],
               ZCCCFG=ZCCCFG)
    r = run([ZCC, "+cpm", "-compiler=sdcc", "--opt-code-size", "-o", out, src],
            env=env, cwd=outdir)
    # zcc writes the binary with no extension; normalize to .COM
    if os.path.exists(out):
        shutil.move(out, com)
        return com
    return None

def build_dcc(name, outdir):
    src = src_path(name)
    if not src:
        return None
    upper = name.upper()
    for tool in ("m80.com", "l80.com", "DCCRTL.MAC"):
        dst = os.path.join(outdir, tool.upper() if tool.endswith(".com") else tool)
        shutil.copyfile(os.path.join(DCC_DIR, tool), dst)
    mac = os.path.join(outdir, f"{upper}.MAC")
    dcc = os.path.join(DCC_DIR, "dcc")
    dccpeep = os.path.join(DCC_DIR, "dccpeep")
    dccrtlstrip = os.path.join(DCC_DIR, "dccrtlstrip")
    stack = str(STACK.get(name, 512))
    flags = []
    src_text = open(src, errors="ignore").read()
    if re.search(r"%[-+ #0-9.*]*[fF]", src_text):
        flags += ["-ffloatio"]
    if re.search(r"%[-+ #0-9.*]*l[duxXs]", src_text):
        flags += ["-flongio"]
    # dcc resolves its bundled headers (stdbool.h, etc.) relative to its CWD,
    # so compile from the dcc repo root.
    if run([dcc] + flags + ["-stack", stack, src, "-o", mac],
           cwd=DCC_DIR).returncode != 0:
        return None
    peep = os.path.join(outdir, "_PEEP.MAC")
    if run([dccpeep, mac, peep]).returncode == 0 and os.path.exists(peep):
        shutil.move(peep, mac)
    _crlf(mac)
    runcpm = os.path.join(DCC_DIR, "runcpm.sh")
    if not _m80(runcpm, outdir, f"={upper}.MAC /X /O /Z /L"):
        return None
    rtl = os.path.join(outdir, "RTLMIN.MAC")
    keep = []
    if "-ffloatio" in flags: keep += ["-k", "_pffio"]
    if "-flongio" in flags:  keep += ["-k", "_pflng"]
    if run([dccrtlstrip] + keep + ["-r", os.path.join(outdir, "DCCRTL.MAC"),
                                   "-o", rtl, mac]).returncode != 0:
        return None
    _crlf(rtl)
    _m80(runcpm, outdir, "=RTLMIN.MAC /X /O /Z")
    _m80(runcpm, outdir, f"/P:100,RTLMIN,{upper},{upper}/N/E", linker="L80.COM")
    com = os.path.join(outdir, f"{upper}.COM")
    if os.path.exists(com):
        dst = os.path.join(outdir, f"DCC_{upper}.COM")
        shutil.copyfile(com, dst)
        return dst
    return None

def _crlf(path):
    data = open(path, "rb").read()
    data = re.sub(rb"\r?\n", b"\r\n", data)
    open(path, "wb").write(data)

def _m80(runcpm, outdir, cmdline, linker="M80.COM"):
    """Invoke M80/L80 via runcpm.sh (vcpm) inside outdir; timeout-guarded."""
    try:
        subprocess.run(["bash", runcpm, linker, cmdline], cwd=outdir,
                       stdin=subprocess.DEVNULL,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                       timeout=40)
    except Exception:  # noqa
        return False
    return True

# --------------------------------------------------------------------------
# Raw codegen size (test TU only, no runtime)
# --------------------------------------------------------------------------

def raw_clang(name, outdir):
    src = src_path(name)
    if not src:
        return None
    o = os.path.join(outdir, "raw_clang.o")
    if run([CLANG] + CLANG_CFLAGS + ["-c", src, "-o", o]).returncode != 0:
        return None
    r = run([OBJDUMP, "--section-headers", o])
    total = 0
    for line in r.stdout.splitlines():
        p = line.split()
        if len(p) >= 4 and p[0].isdigit():
            n = p[1]
            if (n.startswith(".text") or n.startswith(".rodata")) and not n.startswith(".rela"):
                total += int(p[2], 16)
    return total

def raw_zsdcc(name, outdir):
    src = src_path(name)
    if not src:
        return None
    o = os.path.join(outdir, "raw_zsdcc.o")
    env = dict(os.environ, PATH=os.path.join(Z88DK, "bin") + ":" + os.environ["PATH"],
               ZCCCFG=ZCCCFG)
    if run([ZCC, "+cpm", "-compiler=sdcc", "--opt-code-size", "-c", src, "-o", o],
           env=env, cwd=outdir).returncode != 0 or not os.path.exists(o):
        return None
    total = 0
    for line in run([Z80NM, o]).stdout.splitlines():
        m = re.search(r"Section (code\w*|rodata\w*): (\d+) bytes", line)
        if m:
            total += int(m.group(2))
    return total

def raw_dcc(name, outdir):
    """Assemble the dcc .MAC standalone (no RTL) and read __BSSB = end of
    code+data from the M80 listing's symbol table."""
    src = src_path(name)
    if not src:
        return None
    upper = name.upper()
    shutil.copyfile(os.path.join(DCC_DIR, "m80.com"), os.path.join(outdir, "M80.COM"))
    mac = os.path.join(outdir, f"R{upper}.MAC")
    dcc = os.path.join(DCC_DIR, "dcc")
    dccpeep = os.path.join(DCC_DIR, "dccpeep")
    if run([dcc, src, "-o", mac], cwd=DCC_DIR).returncode != 0:
        return None
    peep = os.path.join(outdir, "_RP.MAC")
    if run([dccpeep, mac, peep]).returncode == 0 and os.path.exists(peep):
        shutil.move(peep, mac)
    _crlf(mac)
    runcpm = os.path.join(DCC_DIR, "runcpm.sh")
    _m80(runcpm, outdir, f"R{upper},R{upper}=R{upper}.MAC /Z")
    prn = os.path.join(outdir, f"r{name.lower()}.prn")  # M80 lowercases the listing
    if not os.path.exists(prn):
        # fall back to any matching prn
        for f in os.listdir(outdir):
            if f.lower() == f"r{name.lower()}.prn":
                prn = os.path.join(outdir, f); break
    if not os.path.exists(prn):
        return None
    txt = open(prn, errors="ignore").read()
    m = re.search(r"([0-9A-Fa-f]{1,4})[I]?'?\s+__BSSB", txt)
    return int(m.group(1), 16) if m else None

# --------------------------------------------------------------------------
# Per-test measurement
# --------------------------------------------------------------------------

BUILDERS = {"dcc": build_dcc, "clang": build_clang, "zsdcc": build_zsdcc}
RAW      = {"dcc": raw_dcc,   "clang": raw_clang,   "zsdcc": raw_zsdcc}

def measure(name, compilers):
    """Return {compiler: {size, raw, tstates, out, built}}."""
    res = {}
    args = ARGS.get(name, "")
    for c in compilers:
        outdir = os.path.join(WORK, f"{name}_{c}")
        shutil.rmtree(outdir, ignore_errors=True)
        os.makedirs(outdir, exist_ok=True)
        stage_fixtures(outdir)
        com = BUILDERS[c](name, outdir)
        if not com or not os.path.exists(com):
            res[c] = {"built": False}
            continue
        res[c] = {
            "built": True,
            "size": os.path.getsize(com),
            "raw": RAW[c](name, outdir),
            "tstates": measure_tstates(com),
            "out": run_vcpm(com, args),
        }
    # Consensus verdict
    built = [c for c in compilers if res[c].get("built")]
    for c in built:
        peers = [o for o in built if o != c]
        if not peers:
            res[c]["verdict"] = "SOLO"
        elif any(res[c]["out"] == res[o]["out"] for o in peers):
            res[c]["verdict"] = "AGREE"
        else:
            res[c]["verdict"] = "DIFF"
    return res

# --------------------------------------------------------------------------
# Main
# --------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("tests", nargs="*", help="test names (default: curated subset)")
    ap.add_argument("--all", action="store_true", help="full curated corpus")
    ap.add_argument("--csv", action="store_true", help="CSV output")
    ap.add_argument("--compilers", default="dcc,clang,zsdcc",
                    help="comma list (default dcc,clang,zsdcc)")
    a = ap.parse_args()

    compilers = [c.strip() for c in a.compilers.split(",") if c.strip()]
    tests = a.tests or (FULL_TESTS if a.all else DEFAULT_TESTS)
    os.makedirs(WORK, exist_ok=True)

    if a.csv:
        print("test,compiler,size,raw,tstates,verdict")
    else:
        print(f"{'test':<11}{'compiler':<8}{'size':>8}{'raw':>8}{'tstates':>13}  verdict")
        print("-" * 58)

    for name in tests:
        if not src_path(name):
            sys.stderr.write(f"SKIP {name}: no source\n")
            continue
        res = measure(name, compilers)
        for c in compilers:
            r = res[c]
            if not r.get("built"):
                if a.csv:
                    print(f"{name},{c},BUILD_FAIL,,,")
                else:
                    print(f"{name:<11}{c:<8}{'BUILD_FAIL':>8}{'':>8}{'':>13}")
                continue
            sz, raw, ts, v = r["size"], r.get("raw"), r.get("tstates"), r["verdict"]
            raw_s = str(raw) if raw is not None else "?"
            ts_s = str(ts) if ts is not None else "?"
            if a.csv:
                print(f"{name},{c},{sz},{raw_s},{ts_s},{v}")
            else:
                print(f"{name:<11}{c:<8}{sz:>8}{raw_s:>8}{ts_s:>13}  {v}")
        if not a.csv:
            print()


if __name__ == "__main__":
    main()
