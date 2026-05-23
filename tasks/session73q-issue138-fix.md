# Session 73q — #138 fix: liveness-driven 1B compensation

Cross-MBB BSS-spill peephole now uses `POP rr` (1 B) for SP
compensation when a register pair is dead at the escape MBB's
live-in set, instead of `INC SP; INC SP` (2 B).

Result: lit test #132 updated to expect `pop af` at the
small-test compensation site; cpnos PROM1 2028 -> 2029 B
(+1 B; pipeline-ordering side effect overrides the per-site
saving for cpnos specifically); AES + lit + test-runner sweep
zero per-test diff.

Closes #138.
