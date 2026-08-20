---
screen: notes-overlay.screen.md
---

# notes-overlay — toast

## 0. Meta

Confirmation shown after a coord-menu action ("Show on map" or "Share to chat").
Same as normal except a frame-fixed toast appears bottom-centre for ~3s. The
coord-menu that triggered it has already closed.

## State deltas

| element | op | value |
|---|---|---|
| toast | show | bottom-centre confirmation with check icon + message |
| toast | set | text = e.g. "Marker placed on the map — {place} ({x}, {y})." or "Sent to chat: [{label}]" |
