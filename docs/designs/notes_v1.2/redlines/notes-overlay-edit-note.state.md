---
screen: notes-overlay.screen.md
---

# notes-overlay — edit-note

## 0. Meta

Editing an existing note. Same as normal except the targeted `note-card` is
replaced in place by the editor form (identical layout to new-note), pre-filled
with that note's title, body, category, scope, and target. All other cards are
unchanged.

## State deltas

| element | op | value |
|---|---|---|
| note-card | replace-content | swap the edited card for `edit-form` (pre-filled) |
| form-label | set | "EDIT NOTE" |
| save-btn | set | text="Save changes" |

Note: the target input shows when the pre-filled scope is Character or Zone.
