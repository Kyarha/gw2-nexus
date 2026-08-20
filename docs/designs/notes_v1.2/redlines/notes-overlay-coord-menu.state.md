---
screen: notes-overlay.screen.md
---

# notes-overlay — coord-menu

## 0. Meta

A coordinate chip in a note body was clicked. Same as normal except a small
frame-fixed popup opens, anchored just below the clicked `coord-chip` (clamped to
stay within the frame's right edge). Dismissed by clicking elsewhere or Escape.

## State deltas

| element | op | value |
|---|---|---|
| coord-menu | show | popup below the clicked `coord-chip`: kicker "COORDINATE", the coord label, then "Show on map" and "Share to chat" rows |
| coord-menu-label | set | text = the clicked coord's label |
