---
screen: notes-overlay.screen.md
---

# notes-overlay — closed

## 0. Meta

Panel dismissed (hotkey **N** or the close button). Only the left rail remains
over the game world. Same as normal except the whole `panel` is gone and the
rail's Notes button drops to its resting look.

## State deltas

| element | op | value |
|---|---|---|
| panel | hide | — |
| rail-notes-btn | set | background=none, border=none, color=muted @ 70% (inactive resting state) |
