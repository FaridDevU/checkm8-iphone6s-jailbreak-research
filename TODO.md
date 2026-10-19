# Lessons learned

## Technical

- `-B` (partial fakefs) broken on 16 GB devices: only sets `setup_partial_root`,
  never `setup_rootful`, so `prelaunchd.c` skips `setup_fakefs()` entirely.
  Workaround: push `palera1n_flags 0x80000015` through PongoOS console
  directly, then `bootx`. See `docs/bugs/01-palera1n-B-flag-noop.md`.
- Partial fakefs skips 14 directories to fit 16 GB free space, but leaves no
  empty mount points for them. First boot panics in `fbi()`
  (`include/fakedyld/utils.h`): the "skipping" log line is not an early
  return, so it binds anyway and panics with ENOENT. Fix: mkdir the 14 dirs
  inside the fakefs before booting. See `docs/bugs/02-jbinit-fbi-bind-panic.md`.
- Activation bypass needs entitlements matched to the exact same iOS build.
  AMFI off bypasses trust checks, not entitlement validation, so a mismatched
  `ldid -S` still fails codesign.
- palera1n v2.4 stalls intermittently on AMD USB4/xHCI hosts during checkm8;
  v3.0.0-beta.2 did not on the same hardware.

## Process / publishing

- Don't ship an AI co-author line in commits unless asked; strip it if it
  slips in.
- Strip device-identifying data (ECID, UDID, serial, account info) before
  publishing anything.
- GPL-3.0 chosen over MIT, user's explicit call.
- A GitHub org can fully block an account (`GraphQL: User is blocked`) after
  a maintainer closes/mocks an issue. Once blocked, no edit/comment/delete on
  issues in that org — not fixable from this side. Don't keep retrying there.
- "Does this look AI-written" has two different kinds of answer:
  - Statistical (em-dash density, header density, sentence length/CV,
    contraction ratio) — useful as a sanity check against real baselines,
    but for a single unique long document there's no real population to
    match against, so this converges to diminishing returns fast. Don't loop
    on it.
  - Qualitative — this is the actual signal. Grep for:
    - literal tool-session language leaking into prose ("the shell tool
      (Bash)", "the execution environment", "the harness", "this
      conversation's accumulated content")
    - third-person self-reference ("the user" / "USER DIRECTIVE") where it
      should be first-person ("I")
  These are deterministic, checkable with grep, and were the real fixes made
  in this repo's research-log.md (10+ instances found and corrected).
- When a measurement methodology looks too clean, check sample size before
  trusting it (n=8 bug-report sample gave a falsely tight range; n=24 showed
  the real one was much wider — don't shorten real content to match a small
  sample).
