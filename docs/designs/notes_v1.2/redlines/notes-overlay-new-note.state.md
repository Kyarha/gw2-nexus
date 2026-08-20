---
screen: notes-overlay.screen.md
---

# notes-overlay — new-note

## 0. Meta

Creating a note. Same as normal except the editor form appears at the top of the
note area, above the existing list. Reached from either "New note" button. The
list below stays visible and scrolls under the form.

## State deltas

| element | op | value |
|---|---|---|
| new-note-form | show | full editor at top of `note-area`, above `note-list` |
| form-label | set | "NEW NOTE" |
| save-btn | set | text="Save note" |

Notes: the target input inside the form stays hidden while scope = Global (its
default); it appears when scope is set to Character or Zone. `empty-state` never
shows while the form is open, even if the list is otherwise empty.
