---
screen: notes-overlay.screen.md
---

# notes-overlay — empty

## 0. Meta

The note area has nothing to list — either a search with no matches or a selected
category with no notes. Same as normal except the card list is replaced by the
centred empty-state block. The left tree still renders (with 0 counts where
relevant), and the heading/context strip are unchanged.

## State deltas

| element | op | value |
|---|---|---|
| note-list | hide | — |
| empty-state | show | dashed icon box + title + body + "Create a note" button |
| empty-title | set | "No notes yet" · "No matches" (search) · "Nothing in {Category}" |
| empty-body | set | context-appropriate line (default / no-match / empty-category) |
| toolbar-count | set | "0 notes" |
