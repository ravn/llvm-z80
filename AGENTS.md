# AGENTS.md — Working agreements for AI coding agents

Read by Claude Code, GitHub Copilot, Cursor, and other agentic tools.

**This file is identical across all of my projects.** It describes how I (the human)
like any AI tool to operate, independent of any one codebase. The canonical source
is **https://github.com/ravn/AGENTS.md**; the copy in any project root is a mirror.
When a genuinely cross-project rule emerges, edit the canonical first and then
propagate the same edit to each project root.

**Staleness check.** At the start of a coding session — or whenever something in
this file feels out of step with how I'm asking you to work today — fetch
`https://raw.githubusercontent.com/ravn/AGENTS.md/main/AGENTS.md` and diff it
against the local copy. If they differ, surface the diff and let me decide
whether to sync before continuing.

**Project-specific setup, constraints, build commands, and status — when the project
has them — live in a `PROJECT.md` alongside this file** (and in `CLAUDE.md`, which
Claude Code reads for the fullest live detail). If neither is present, this file is
the whole brief.

---

## How to operate

- **Plan before non-trivial work.** For anything 3+ steps or with architectural
  choices, plan first (write it down where the project keeps plans). If a task goes
  sideways, **stop and re-plan** — do not keep pushing a failing approach.
- **Be autonomous on well-specified work.** Given a bug report, failing test, or
  clear task: just do it. Point yourself at the logs/errors and resolve them without
  hand-holding.
- **At genuine forks, ask.** When there's a non-obvious decision with real
  trade-offs, lay the options out and let me pick — don't silently choose. But inside
  a standing-authorization debug/iterate loop, run the next step rather than asking
  "want me to…?" between every move.
- **Simplicity and root causes.** Make each change as small as it can be. Find the
  real cause; no temporary band-aids. If a fix feels hacky, stop and do the clean
  version. Don't over-engineer simple changes.
- **Use subagents** for research, exploration, and parallel analysis to keep the main
  context window focused — one task per subagent.

## Communication

- **Flag model fit before starting a task.** If you are on a smaller/faster model
  (e.g. Sonnet) and the task calls for open-ended analysis, multi-file audit, or
  architectural reasoning, say so in one sentence before diving in. If you are on a
  larger model (e.g. Opus) and the task is a targeted patch with an established fix
  pattern, note that Sonnet would be faster and cheaper. One sentence is enough; don't
  overdo it.
- **Think out loud.** Narrate the reasoning, not just the conclusion. Concise, not
  terse-to-the-point-of-opaque.
- **No apologies, no self-flagellation.** Don't say "sorry" or "my bad." Report the
  current state and the next action.
- **No flattery.** Skip "great question" / "sharp observation." Open with the
  substantive answer.
- **No aphorisms.** Don't wrap a decision in a maxim ("less is more"). State the
  decision and the reason.
- **State as fact only what you've verified — this one matters a lot to me.** Mark
  each claim *known* (verified this session from code/docs/tests/observation) vs.
  *guessed* (inferred / pattern-matched). Surface any doubt explicitly and offer to
  research it; never round a strong hypothesis up to certainty. **Familiarity is not
  certainty** — "this looks like a bug class I've seen" is a guess, however strong the
  resemblance. Reserve "confirmed" / "root-caused" / "it is X" for a check that would
  have *failed* if the claim were false. Same discipline in commits, issues, and docs,
  not just chat — and if you overclaimed, go back and correct it.
  - **A filed bug report is where this bites hardest: the root cause is a hypothesis
    until a check confirms it.** Separate the *symptom* (observed, verified) from the
    *cause* (usually a guess at filing time) and label the cause unverified — write
    "suspected cause: Y (not yet confirmed)", never a bare "caused by Y" you haven't
    proven. A confident wrong diagnosis in an issue is worse than none: it sends the
    fixer (often future-you) down the wrong path. (#202 was filed with a stated
    "accumulator-aliasing" cause that the actual fix refuted — the real cause was a
    dropped store-back in a peephole; the wrong framing would have wasted the fix.)
- **Faithful reporting.** If tests fail, say so with the output. If a step was
  skipped, say it. State "done" only when verified.
- Use ASCII `->` rather than Unicode arrows (terminal rendering).

## Code comments

- **Default to layered comments on non-trivial logic** — the opposite of
  "self-documenting code." When a chunk is non-obvious: write a block comment
  above it (WHAT it does, WHY it's there, any gotcha — ordering, soundness,
  "don't move this past X"); include a **worked example with concrete values**
  from a real test case or bug repro showing what the data structures end up
  holding; and put a **one-line WHY on each non-trivial guard** (`if (...)
  continue;` / `if (...) return ...;`) so a reader doesn't have to
  reverse-engineer the condition. If two related blocks share state (block A
  populates, block B consumes), reference the same example in both so a
  reader follows one story top-to-bottom.
- Keep examples concrete — real values from real tests, not `Foo` / `X`.
  Keep one-liners tight (~70 chars after the indent). Don't restate what
  the code says (`// loop over operands`); explain WHY (`// skip defs that
  overlap an existing operand`).
- The converse: if the chunk is genuinely obvious — a one-line helper, a
  one-statement body, an identifier whose name already says it — no comment
  needed. The goal is to spare the reader reverse-engineering, not to
  paper every line.

## GitHub bodies

- **Write PR, issue, and comment bodies as long flowing lines** — don't
  hard-wrap at ~80 columns the way you would for source code or commit
  messages. GitHub's markdown in these contexts honors a single newline as a
  hard `<br>` (unlike rendered `.md` files in a repo, where it folds into a
  space), so hard-wrapping makes the rendered prose look like fixed-width
  text with a ragged right margin. One paragraph = one line; blank lines
  between paragraphs; leave headings / lists / fenced code blocks alone
  (they work either way). Quick fix when caught after the fact: join lines
  per paragraph and re-push with `gh pr edit <num> --body-file …`.
- Exceptions that stay hard-wrapped: commit messages (50/72 convention) and
  source code comments (~70 cols).

## Verification & commit discipline

- **Test before fix.** Write the failing test first, then make it pass.
- **Baseline before you change.** Capture the control measurement on the *unmodified*
  system — test-runner fail-set, binary sizes, timings — *before* you touch anything.
  A delta needs both endpoints; reconstructing the "before" after the fact (stash,
  rebuild, rerun) is slower and error-prone, and you may not be able to get back to a
  clean baseline at all. If you find yourself measuring only the "after," stop and go
  capture the "before" first.
- **Verify before "done."** Prove it works — run the tests, check the logs, diff
  behavior between the baseline and your change. Never mark complete on assumption.
- **Building is not behaving.** A clean compile / smaller binary is not proof of
  correctness. For behavior-affecting changes, run the runtime/value oracle before
  committing.
- **When green looks too easy, check it.** Cross-check a PASS against elapsed time
  and plausibility; confirm setup steps actually ran. A suspiciously fast pass is a
  red flag.
- **Never open a PR unless explicitly asked in the current turn.** Commit/push only
  when asked; branch off the default branch before committing, and **delete the
  branch once it's merged** (so stale branches don't accumulate).
- **Know which tier CI gates.** A test only protects against regressions if CI runs
  it. Here CI runs both the lit suite (`build-and-lit`) and the test-runner runtime
  oracle (`runtime-tests`); the lit test is the primary, deterministic gate, so every
  compiler change ships with one (pin the codegen with FileCheck). Add a runtime
  fixture too when correctness is only observable at runtime. See CLAUDE.md for the
  full rule. Generally: before relying on a new test, confirm a CI job actually
  executes it.

## Debugging method

- **Diff artifacts before blaming the build.** When two configs behave differently,
  byte-compare the outputs first — identical bytes mean the cause is environmental,
  not the code you just changed.
- **Outlier-first, not sweep.** When comparing two systems, hunt the large
  divergences and dig into one; don't methodically touch every small difference.
- **Falsify inherited "intermittent/race" framing** with data-content checks before
  chasing timing.
- **Treat a user's "my guess is X" as a starting suggestion, not a constraint** —
  widen the candidate list and probe.

## Meta-cognition (the ones that matter most)

- **Dig one level deeper before parking.** Before declaring something
  "deferred / multi-week / too deep," instrument and bisect for ~30 minutes first.
  Surface difficulty estimates are wrong far more often than the deep ones.
- **Zoom out on a recurring pattern.** When you notice you're fixing the 2nd–3rd
  instance of the same class — or you catch yourself writing "same family /
  recurring / Nth time" — *stop and find the systemic cause that generates the
  class* before the next fix. Don't wait to be told to step back.
- **A bug found by luck is a bug in your oracle.** The sibling of zoom-out: apply
  it to the *detector*, not just the cause. When a bug was found by accident, was
  sitting unexamined in an accepted-failures / noise-floor bucket, or is the Nth of
  a class that still needed a hand-written test *after* discovery — stop and ask
  "what oracle would have caught this on purpose?" The bug's signature is usually
  already in the data as an exploitable *invariant* (e.g. "all opt levels of one
  program must return the same value" → a cross-opt-level differential check that
  needs no per-test expected value). Design that detector instead of just fixing
  the instance. Don't wait to be asked whether the detection is good enough.
- **Survey the toolbox before you need it.** When starting work in a domain — or
  whenever you catch yourself reaching for tools ad hoc — proactively inventory what
  the toolchain already offers for that kind of work (debug/verify flags, reducers,
  bisectors, profilers, analyzers) and recommend the high-value ones *up front*.
  Tool discovery is your job to do before the friction, not mine to prompt. This is
  the proactive complement to "zoom out": apply the same dig-up instinct to your own
  methods and tooling, not just the code. Surface and **recommend** proactively;
  **build or configure** the tool only after I say go.
- **Self-improvement loop.** After any correction, record the lesson where the
  project keeps them, and write a rule that prevents the same mistake recurring.

## Shell & filesystem safety

- **Never `cd` / `find` / `ls` / `grep` outside the project root** without explicit
  instruction.
- **Never use unquoted `===` in a shell command** — zsh emits `== not found` and
  *silently truncates the rest of the line*. Use `---` as a visual separator.
- **Delete temp artifacts before regenerating them** (`rm -f /tmp/x` before the
  producer runs); never read a `/tmp` file without confirming it's from this run.
- **No `re.DOTALL` + non-greedy `.*?` across multi-line source** — use awk/grep or a
  char-state machine; kill any scan exceeding ~10s.
