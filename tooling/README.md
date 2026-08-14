# tooling/ — dev-machine guardrails

Utilities for the developer's Claude Code setup (not part of the addon build).

## `block-library-writes.sh` — block writes into `~/Library`

A Claude Code **`PreToolUse`** hook that **denies any tool call writing into
`~/Library`** (Bash copies / downloads / redirections / extractions, and
Write/Edit whose target file is under `~/Library`). Reads of `~/Library` are
allowed.

**Why it exists:** on 2026-08-13 a session downloaded a built `notes.dll`
straight into the CrossOver bottle under `~/Library/Application Support/…` and
**corrupted the running Guild Wars 2**. Placing build artifacts into the game
folder is the **user's manual step, never automated** (see
[ADR-0002](../docs/decisions/adr-0002-first-addon-repo-topology.md) hand-off
discipline and the CI artifact hand-off memory). This hook enforces that at the
tooling layer so no session can reason around it.

It is a **heuristic on shell commands** (covers the realistic write verbs +
redirection + downloaders + `tar` extract + `find -delete`); the Write/Edit
file-target check is exact. It errs toward blocking — a `cp`/`mv` that merely
reads *from* `~/Library` may also be caught. Safety over convenience; run such a
command by hand if you truly need it.

### Install on a machine (user-level — all projects, all sessions)

The hook must live in your user config (`~/.claude/`), which git does not manage
— so after pulling this repo, copy the script into place and wire it in
`~/.claude/settings.json`.

**Easiest (no terminal): paste this into a Claude Code session on that machine**
(from any folder) and approve its actions:

```
Install the ~/Library write-guard from this repo. Do it yourself, don't hand me terminal commands:
1. Copy tooling/block-library-writes.sh from this repo to ~/.claude/hooks/block-library-writes.sh and make it executable.
2. Read ~/.claude/settings.json (create as {} if missing). MERGE, preserving existing hooks. Under .hooks.PreToolUse add these two entries if not already present:
   { "matcher": "Bash", "hooks": [ { "type": "command", "command": "bash ~/.claude/hooks/block-library-writes.sh", "timeout": 10 } ] }
   { "matcher": "Write|Edit|MultiEdit|NotebookEdit", "hooks": [ { "type": "command", "command": "bash ~/.claude/hooks/block-library-writes.sh", "timeout": 10 } ] }
3. Validate the JSON parses.
4. Prove it fires with a SAFE probe (writes nothing even if inactive): run Bash `cp /tmp/__no_such_file_probe__ ~/Library/__probe__`. If live, the call is DENIED with the block message. If it instead runs and errors "No such file or directory", tell me to open /hooks once or restart Claude Code, then re-test.
5. Report what changed and the probe result. Never write into ~/Library yourself.
```

**Manual equivalent**, if you prefer to do it yourself:
1. `cp tooling/block-library-writes.sh ~/.claude/hooks/ && chmod +x ~/.claude/hooks/block-library-writes.sh`
2. Add the two `PreToolUse` entries above to `~/.claude/settings.json` (merge, don't overwrite).
3. Open `/hooks` once (or restart Claude Code) so the config reloads.
4. Verify: a Bash `cp <anything> ~/Library/x` is denied.

### Verifying it's live
A Bash command that writes into `~/Library` (e.g. `cp x ~/Library/y`) should be
**blocked** with the guardrail message; normal commands and reads of `~/Library`
are unaffected.
