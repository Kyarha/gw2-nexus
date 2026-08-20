---
screen: notes-overlay.screen.md
---

# notes-overlay — delete-confirm

## 0. Meta

A note's delete was requested. Same as normal except a confirmation strip is
appended at the bottom of that one `note-card`. The card's normal content stays
visible above it; other cards are unchanged.

## State deltas

| element | op | value |
|---|---|---|
| delete-confirm | show | strip inside the targeted `note-card`: "Delete this note? This can't be undone." + Delete (danger) + Keep (ghost) |
