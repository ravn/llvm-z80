use std::path::PathBuf;
use std::process::Command;

use crate::config::{OptLevel, Paths, Target};
use crate::emulator;
use crate::runtime::{self, ElfRuntime};
use crate::suite::*;

const COMPILE_TIMEOUT: u64 = 30;

pub struct ClangConfig {
    pub target: Target,
    pub opt_levels: Vec<OptLevel>,
    pub fast_math: bool,
    pub omit_fp: bool,
    pub inline_runtime: bool,
    pub static_stack: bool,
    /// Append `-mllvm -verify-machineinstrs`: run the MachineVerifier after
    /// every pass and fail the compile on invalid MIR (undefined physreg,
    /// stale liveins, bad reg classes). Catches the peephole-liveness bug
    /// family (e.g. ravn/llvm-z80#199) that otherwise stays latent because
    /// the shipped build never verifies. Works on the Release build; point
    /// BUILD_DIR at an assertions build to add the internal assert() layer.
    pub verify: bool,
    /// Cross-opt-level differential oracle: a correct program returns the SAME
    /// value at every optimization level, so any disagreement among O0..Oz for
    /// the same test is a miscompile -- independent of the hardcoded `expect`
    /// directive (which may itself be wrong/stale). Emits a `<name>_DIFFOPT`
    /// failure naming the disagreeing levels. Would have caught #202 (test_54:
    /// O0_ss=0x0080 vs O1+_ss=0x00FF) with no hand-written test. Strongest with
    /// `-full` (all opt levels).
    pub diff_opt: bool,
    pub pattern: Option<String>,
}

impl ClangConfig {
    fn extra_flags(&self) -> Vec<&str> {
        let mut flags = Vec::new();
        if self.fast_math {
            flags.push("-ffast-math");
        }
        if self.omit_fp {
            flags.push("-fomit-frame-pointer");
        }
        if self.inline_runtime {
            flags.push("-Xclang");
            flags.push("-target-feature");
            flags.push("-Xclang");
            flags.push("+inline-i16-runtime");
        }
        if self.static_stack {
            flags.push("-Xclang");
            flags.push("-target-feature");
            flags.push("-Xclang");
            flags.push("+static-stack");
        }
        if self.verify {
            flags.push("-mllvm");
            flags.push("-verify-machineinstrs");
        }
        flags
    }

    fn active_options(&self) -> Vec<&str> {
        self.extra_flags()
    }

    fn tag_suffix(&self) -> String {
        let mut s = String::new();
        if self.fast_math {
            s.push_str("_ffast");
        }
        if self.omit_fp {
            s.push_str("_nofp");
        }
        if self.inline_runtime {
            s.push_str("_inlrt");
        }
        if self.static_stack {
            s.push_str("_ss");
        }
        s
    }
}

pub fn run(paths: &Paths, config: &ClangConfig, on_result: &mut OnResult) -> SuiteResult {
    let test_dir = paths.clang_test_dir();
    let clang = paths.clang();
    let mut result = SuiteResult::default();
    let reg_name = config.target.reg_name();

    // Build crt0 + compiler-rt builtin objects once for this run.  The
    // suite is unusable without them, so a failure here aborts every test
    // up front rather than silently producing "_halt symbol not found".
    let elf_rt = match runtime::ensure_elf(paths, config.target, &clang) {
        Ok(rt) => rt,
        Err(e) => {
            let tag = format!("clang_{}_runtime", config.target.triple());
            result.add(TestResult::fatal(&tag, format!("ELF runtime: {e}")), on_result, reg_name);
            return result;
        }
    };

    let tests = discover_tests(&test_dir, "test_", "c");
    let suffix = config.tag_suffix();
    let active = config.active_options();

    for test_file in &tests {
        let name = test_file
            .file_stem()
            .unwrap()
            .to_string_lossy()
            .to_string();

        if let Some(ref pat) = config.pattern {
            if !name.contains(pat.as_str()) {
                continue;
            }
        }

        let source = std::fs::read_to_string(test_file).unwrap_or_default();

        // Parse per-test EXTRA-FLAGS from source comments.
        let per_test_flags = parse_extra_flags_c(&source);

        // For the cross-opt-level differential oracle: the observed value at
        // each opt level (only Pass/Fail carry a value; Fatal/Skip don't).
        let mut opt_values: Vec<(OptLevel, String)> = Vec::new();

        for &opt in &config.opt_levels {
            let tag = format!("{name}_{opt}{suffix}");

            // Check SKIP-IF
            if let Some(reason) = check_skip_c(&source, config.target, &active, opt.clang_flag()) {
                result.add(TestResult::skip(&tag, reason), on_result, reg_name);
                continue;
            }

            let mut flags = config.extra_flags();
            for f in &per_test_flags {
                flags.push(f.as_str());
            }

            let r = run_single(
                &clang,
                test_file,
                &tag,
                config.target,
                opt,
                &flags,
                &test_dir,
                &source,
                &elf_rt,
            );
            // Record the observed value for the differential check.
            match &r.outcome {
                TestOutcome::Pass { reg_value } => opt_values.push((opt, normalize_hex(reg_value))),
                TestOutcome::Fail { got, .. } => opt_values.push((opt, normalize_hex(got))),
                _ => {}
            }
            result.add(r, on_result, reg_name);
        }

        // Cross-opt-level differential: every opt level of the same program
        // must return the same value (optimization is semantics-preserving).
        // A disagreement is a miscompile regardless of the `expect` directive.
        if config.diff_opt && opt_values.len() >= 2 {
            let first = &opt_values[0].1;
            if opt_values.iter().any(|(_, v)| v != first) {
                let detail = opt_values
                    .iter()
                    .map(|(o, v)| format!("{o}=0x{v}"))
                    .collect::<Vec<_>>()
                    .join(" ");
                let tag = format!("{name}_DIFFOPT{suffix}");
                result.add(
                    TestResult::fail(tag, detail, "all opt levels agree"),
                    on_result,
                    reg_name,
                );
            }
        }
    }

    result
}

/// Normalize a "0x..." value string for cross-opt-level comparison: drop the
/// "0x" prefix, leading zeros, and case (the Pass path formats the raw value
/// while the Fail path zero-pads to the expected width, so the strings must be
/// canonicalized before comparing). All-zero canonicalizes to "0".
fn normalize_hex(s: &str) -> String {
    let t = s.trim().trim_start_matches("0x").trim_start_matches("0X");
    let t = t.trim_start_matches('0').to_lowercase();
    if t.is_empty() { "0".to_string() } else { t }
}

fn run_single(
    clang: &PathBuf,
    test_file: &PathBuf,
    tag: &str,
    target: Target,
    opt: OptLevel,
    extra_flags: &[&str],
    work_dir: &PathBuf,
    source: &str,
    elf_rt: &ElfRuntime,
) -> TestResult {
    let tmp_dir = unique_tmp_dir(work_dir);
    let _ = std::fs::create_dir_all(&tmp_dir);

    let test_obj = tmp_dir.join(format!("{tag}.o"));
    let elf = tmp_dir.join(format!("{tag}.elf"));
    let bin = tmp_dir.join(format!("{tag}.bin"));

    // Compile to object only — the link step injects crt0 + compiler-rt
    // builtins + linker script explicitly so that _start, _halt, .bss
    // layout, and ___mulhi3 / ___udivhi3 / etc. are all resolved.
    let mut cmd = Command::new(clang.as_os_str());
    cmd.arg(format!("--target={}", target.triple()));
    cmd.arg(format!("-{}", opt.clang_flag()));
    cmd.arg("-c");
    cmd.arg("-nostdlib");
    cmd.arg("-ffreestanding");
    for flag in extra_flags {
        cmd.arg(flag);
    }
    cmd.arg(test_file.as_os_str());
    cmd.arg("-o");
    cmd.arg(test_obj.as_os_str());

    match run_cmd_timeout(&mut cmd, COMPILE_TIMEOUT) {
        Err(e) => {
            remove_tmp_dir(&tmp_dir);
            return TestResult::fatal(tag, format!("compile {e}"));
        }
        Ok((code, _, stderr)) if code != 0 => {
            let err = extract_error(&stderr);
            remove_tmp_dir(&tmp_dir);
            return TestResult::fatal(tag, err);
        }
        _ => {}
    }

    // Link: ld.lld -T <triple>.ld --gc-sections crt0.o test.o builtins/*.o
    let lld = clang.parent().unwrap().join("ld.lld");
    let mut link = Command::new(lld.as_os_str());
    link.arg("--gc-sections");
    link.arg("-T");
    link.arg(elf_rt.linker_script.as_os_str());
    link.arg(elf_rt.crt0_obj.as_os_str());
    link.arg(test_obj.as_os_str());
    for obj in &elf_rt.builtin_objs {
        link.arg(obj.as_os_str());
    }
    link.arg("-o");
    link.arg(elf.as_os_str());

    match run_cmd_timeout(&mut link, COMPILE_TIMEOUT) {
        Err(e) => {
            remove_tmp_dir(&tmp_dir);
            return TestResult::fatal(tag, format!("link {e}"));
        }
        Ok((code, _, stderr)) if code != 0 => {
            let err = extract_error(&stderr);
            remove_tmp_dir(&tmp_dir);
            return TestResult::fatal(tag, format!("link: {err}"));
        }
        _ => {}
    }

    // ELF → flat binary
    let objcopy = clang.parent().unwrap().join("llvm-objcopy");
    if let Err(e) = emulator::elf_to_bin(&objcopy, &elf, &bin) {
        remove_tmp_dir(&tmp_dir);
        return TestResult::fatal(tag, e);
    }

    // Emulate
    let halt_addr = match emulator::halt_addr_from_elf(
        &clang.parent().unwrap().join("llvm-nm"), &elf) {
        Some(addr) => addr,
        None => return TestResult::fatal(tag, "_halt symbol not found in ELF"),
    };
    let result = match emulator::emulate(&bin, target, &halt_addr) {
        Err(e) => TestResult::fatal(tag, e),
        Ok(got) => {
            let expected = emulator::parse_expected(source);
            match emulator::check_result(&got, &expected) {
                Ok(()) => TestResult::pass(tag, format!("0x{got}")),
                Err((got_padded, exp_padded)) => {
                    TestResult::fail(tag, format!("0x{got_padded}"), format!("0x{exp_padded}"))
                }
            }
        }
    };
    // Keep temp files on failure for debugging; clean up on pass.
    if result.is_pass() {
        remove_tmp_dir(&tmp_dir);
    }
    result
}

