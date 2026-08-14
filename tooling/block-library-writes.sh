#!/usr/bin/env bash
# PreToolUse guard — block WRITES into ~/Library. Reads are allowed.
#
# Why: on 2026-08-13 a session copied a built DLL into the CrossOver bottle under
# ~/Library/Application Support and corrupted the user's running Guild Wars 2.
# Placing build artifacts into the game folder is the USER's manual step, never
# automated. This hook denies Bash writes/copies/downloads into ~/Library and any
# Write/Edit whose target file is under ~/Library.
set -u

command -v jq >/dev/null 2>&1 || exit 0   # can't parse input -> don't interfere

input=$(cat)
tool=$(printf '%s' "$input" | jq -r '.tool_name // empty')
home="$HOME"
lib="$home/Library"

msg="BLOCKED: writing into ~/Library is not allowed by a safety guardrail. Placing build artifacts (e.g. a .dll) into the CrossOver bottle or the Guild Wars 2 addons folder is the USER's manual step — a prior automated write there corrupted the running game. If this is genuinely needed, ask the user to do it by hand. Reads of ~/Library are fine; this blocks writes/copies/downloads/extractions into it."

deny() {
  jq -n --arg r "$msg" \
    '{hookSpecificOutput:{hookEventName:"PreToolUse",permissionDecision:"deny",permissionDecisionReason:$r}}'
  exit 0
}

case "$tool" in
  Write|Edit|MultiEdit|NotebookEdit)
    fp=$(printf '%s' "$input" | jq -r '.tool_input.file_path // .tool_input.notebook_path // empty')
    fp=${fp/#\~/$home}
    case "$fp" in
      "$lib"/*|"$lib") deny ;;
    esac
    exit 0
    ;;
  Bash)
    cmd=$(printf '%s' "$input" | jq -r '.tool_input.command // empty')
    [ -z "$cmd" ] && exit 0
    # Normalize home references so ~/Library, $HOME/Library, ${HOME}/Library match.
    norm=$cmd
    norm=${norm//\$\{HOME\}/$home}
    norm=${norm//\$HOME/$home}
    norm=${norm//\~/$home}
    # No Library reference at all -> allow (fast path).
    case "$norm" in
      *"$lib"*) : ;;
      *) exit 0 ;;
    esac
    # Library is referenced. Deny on write intent; allow pure reads.
    # (1) redirection into Library:  > <lib>  /  >> <lib>  /  2> <lib>
    printf '%s' "$norm" | grep -Eq '>>?[[:space:]]*"?'"$lib" && deny
    # (2) unambiguous file-mutating commands (word-matched)
    printf '%s' "$norm" | grep -Eqw '(cp|mv|rsync|install|ln|dd|tee|touch|mkdir|rmdir|rm|unzip|scp|ditto)' && deny
    # (3) in-place sed
    printf '%s' "$norm" | grep -Eq 'sed[[:space:]]+-[a-zA-Z]*i' && deny
    # (4) downloaders that write files
    printf '%s' "$norm" | grep -Eq '(gh[[:space:]]+run[[:space:]]+download|curl[^|]*(-[a-zA-Z]*[oO]|--output)|wget)' && deny
    # (5) tar extract
    printf '%s' "$norm" | grep -Eq 'tar[[:space:]]+-?[a-zA-Z]*x' && deny
    # (6) find ... -delete / -exec rm
    printf '%s' "$norm" | grep -Eq 'find[[:space:]].*(-delete|-exec[[:space:]]+rm)' && deny
    exit 0
    ;;
  *)
    exit 0
    ;;
esac
