---
screen: notes-overlay.screen.md
---

# notes-overlay — loading

## 0. Meta

Transient state before the note data resolves. Panel is open but the left tree
has no categories yet, counts read 0, the context strip has no values, and the
note area shows a loading line.

## State deltas

| element | op | value |
|---|---|---|
| loading-placeholder | show | centred italic "Loading notes…" in `note-area` |
| note-list | hide | — |
| category-group | hide | — (no categories rendered yet) |
| all-notes-row | set | count=0 |
| toolbar-count | hide | — |
| ctx-char | set | text="" (hidden until context loads) |
| ctx-zone | set | text="" |
