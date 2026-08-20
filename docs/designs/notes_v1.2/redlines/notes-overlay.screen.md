---
base: base.md
---

# notes-overlay — screen

## 0. Meta

- **What it is:** the Guild Wars 2 "Notes" HUD overlay in its default resting
  state — panel open, **All notes** selected, all categories expanded, the full
  seed list rendered, no editor or popup active.
- **Frame:** 1920 × 1080 au, density 1×, FIXED (see base *Frame defaults*).
- **Driving axis:** width.
- **Regions:** left toolbar **rail** (always present) + floating **panel**
  (title bar, left category pane, right note pane).
- **Default-hidden elements** (listed in §3, revealed by state files):
  `new-note-form`, `edit-form`, `delete-confirm`, `empty-state`,
  `loading-placeholder`, `coord-menu`, `toast`.
- Non-default states: closed · loading · empty · new-note · edit-note ·
  delete-confirm · coord-menu · toast.

## 3. Elements

Size is `w × h` au, or `hug` / `fill`. Fills/borders named by base token
(`@ NN%` = alpha). Position is relative to the named neighbour.

| id | size | fix/scale | position & notes |
|---|---|---|---|
| overlay-root | fill × fill | FIXED | Covers the frame. Fill: radial world-gradient world-sky → world-mid → world-deep over world-void; inset vignette. |
| rail | {rail-width} × hug | FIXED | Pinned to frame left edge, top {rail-top} from top. Vertical column, {xxs} gap, padding {lg} top/bottom. Fill: gradient world-deep→panel @ ~70%, brass @ 22% right/top/bottom borders. |
| rail-notes-btn | {rail-btn} × {rail-btn} | FIXED | First item in `rail`. Active (panel open): btn-fill gradient, gold-line @ 60% border, gold-btn glyph (20 icon). |
| rail-divider | 22 × 1 | FIXED | Below `rail-notes-btn`, {sm}/…{xs} gap. brass @ 20%. |
| rail-events-btn | {rail-btn} × {rail-btn} | FIXED | Below divider. Inert; 19 icon, muted @ 40%. |
| rail-settings-btn | {rail-btn} × {rail-btn} | FIXED | Below `rail-events-btn`, {xs} gap. Inert; 19 icon, muted @ 40%. |
| panel | {panel-width} × hug | FIXED | Floats at {panel-left} from frame left, {panel-top} from top (clears the rail). max-width = frame − 120. Fill: layered panel radials; border gold-line @ 40%; radius {radius}; deep outer + inset shadow. |
| titlebar | fill × hug | FIXED | Top of `panel`. Padding {xl} 14 {xl} {h1}; row, {xl} gap; brass @ 28% bottom border; gradient titlebar-top→titlebar-bottom. |
| title-icon | {title-icon} × {title-icon} | FIXED | Left of titlebar. Radius {radius}; gold-line @ 50% border; gold glyph (19 icon). |
| title-block | fill × hug | FIXED | Right of `title-icon`, {xl} gap. Holds `title-text` over `title-sub`. |
| hotkey-badge | hug × hug | FIXED | Right side of titlebar, before `close-btn`. Padding {xs} {md}; brass @ 35% border; radius {radius}; "HOTKEY N" ("N" in gold). |
| close-btn | {close-btn} × {close-btn} | FIXED | Titlebar far right, {xxs} left margin. brass @ 30% border; muted-2 X (14). Hover: close-hover. |
| body-row | fill × hug | FIXED | Below titlebar. Two-pane flex row, panes stretch to equal height. |
| left-pane | {left-pane-width} × fill | FIXED | Left of `body-row`. brass @ 24% right border; black @ 20% fill; column. |
| search | fill × hug | FIXED | Top of `left-pane`, padding {xl} {xl} {lg}. Contains `search-input`. |
| search-input | fill × ~34 | FIXED | Inside `search`. Padding {md} {lg} {md} 30 (leaves room for 14 search icon at left {lg}); black @ 34% fill; brass @ 32% border; radius {radius}. |
| cat-tree | fill × fill | FIXED | Below `search`. Scrolls; max-height {scroll-cap}, min-height {list-min}; padding {xxs} {md} {xl}. |
| cat-tree-label | hug × hug | FIXED | Top of `cat-tree`, padding {sm} {sm} {md}. `label` style, "CATEGORIES". |
| all-notes-row | fill × ~34 | FIXED | Below `cat-tree-label`. nav row: menu icon (14) + "All notes" + `count-badge`. Active here → gold-line @ 55% border, btn-fill @ 55% fill, gold-btn text. |
| category-group | fill × hug | FIXED | Repeats ×5 (crafting, gathering, legendaries, dailies, characters) below `all-notes-row`, {xxs} top gap each. Holds `cat-header` + `cat-children`. |
| cat-header | fill × hug | FIXED | Top of each `category-group`. Row: `cat-chevron` (11) + `cat-dot` + label (nav) + `count-badge`. Padding 7 {md}. |
| cat-dot | {cat-dot} × {cat-dot} | FIXED | Left of the category label, radius {xxs}. Fill = that category's token. |
| cat-children | fill × hug | FIXED | Under `cat-header` when expanded. Left inset 8 + {md} margin; brass @ 18% left rule. |
| cat-note-item | fill × hug | FIXED | Repeats inside `cat-children`. Row: `scope-dot` (6, scope token) + title (note-row) + optional auto-icon (11, coord). Padding {sm} {md}. Selected → gold-line @ 50% border, btn-fill @ 40% fill. |
| right-pane | fill × fill | FIXED | Right of `left-pane`. Column. |
| toolbar | fill × hug | FIXED | Top of `right-pane`. Row, {lg} gap, wraps; padding 11 {h1}; brass @ 18% bottom border. |
| toolbar-icon | {toolbar-icon} × {toolbar-icon} | FIXED | Left of toolbar. Radius {radius}; brass @ 30% border; glyph 13. |
| toolbar-heading | hug × hug | FIXED | Right of `toolbar-icon`, {lg} gap. heading style, "All notes". |
| toolbar-count | hug × hug | FIXED | Right of heading. caption style, "10 notes". |
| new-note-btn | hug × hug | FIXED | Toolbar far right (pushed by a flex spacer). Padding 7 13, {sm} gap; plus icon (14) + "New note" (button style); btn-fill gradient; gold-line @ 55% border. |
| context-strip | fill × hug | FIXED | Below `toolbar`. Row, 9 gap; padding 7 {h1}; black @ 24% fill; brass @ 12% bottom border. |
| ctx-char | hug × hug | FIXED | In `context-strip`: person icon (13, char-green) + "Kytalia Moonbind" (caption, char-green, weight 500). |
| ctx-zone | hug × hug | FIXED | After a "·" divider in `context-strip`: pin icon (13, zone-blue) + "Diessa Plateau" (caption, zone-blue, weight 500). |
| note-area | fill × fill | FIXED | Below `context-strip`. Scrolls; max-height {scroll-cap} − 34, min-height {list-min}; padding {xxl} {h1} {h2}. |
| note-list | fill × hug | FIXED | Inside `note-area`. Column, {xl} gap between cards. |
| note-card | fill × hug | FIXED | Repeats in `note-list`. Padding {h1} 15 {xxl}; card-top→card-bottom gradient; brass @ 26% border (selected → gold-bright @ 70% + ring); radius {radius}. |
| note-tack | {tack} × {tack} | FIXED | Circle, centred on the card's top edge (offset −6 up). Radial gold-btn→pin-dark; drop shadow. |
| note-title | fill × hug | FIXED | Top of `note-card`. heading style at size=15.5, line-height 1.25. |
| note-badges | fill × hug | FIXED | Below `note-title`, 7 top gap; wrapping row of pills, 7 gap. |
| note-cat-pill | hug × hug | FIXED | First in `note-badges`. Padding {xxs} 9; radius 11; badge style; 6 dot + label; border = category token @ ~44%, text = category token. |
| note-scope-pill | hug × hug | FIXED | After `note-cat-pill`. Same pill shape; 6 dot + scope label (+ " · target" when scoped); color = scope token. |
| note-auto-pill | hug × hug | FIXED | Present only when the note auto-surfaces in the current context. Pill with coord 10 icon + "Auto-surfacing here"; coord @ 40% border, coord text. |
| note-actions | hug × hug | FIXED | Top-right of `note-card`, aligned to `note-title` row, {xs} gap. Two `icon-btn`s. |
| note-edit-btn | {icon-btn} × {icon-btn} | FIXED | First action; pencil (14); brass @ 28% border, muted-2. Hover → gold-btn. |
| note-delete-btn | {icon-btn} × {icon-btn} | FIXED | Second action; trash (14). Hover → delete-hover. |
| note-body | fill × hug | FIXED | Below the title/badges block, 11 top gap. body style; inline text runs interleaved with `coord-chip`s. |
| coord-chip | hug × hug | FIXED | Inline within `note-body`. Padding 1 7 2, {xs} gap; pin icon (11) + "Place — x, y"; coord @ 42% border, coord @ 10% fill, coord-chip style. Hover → coord-hover. |
| new-note-form | fill × hug | FIXED | **Hidden by default.** When shown, sits at the very top of `note-area` above `note-list`. See §Elements/form below. |
| edit-form | fill × hug | FIXED | **Hidden by default.** Replaces one `note-card`'s content in edit-note state. Same layout as `new-note-form`; label "EDIT NOTE", primary button "Save changes". |
| delete-confirm | fill × hug | FIXED | **Hidden by default.** Strip appended inside a `note-card`, 12 top gap; padding 9 {xl}; danger-line @ 40% border, danger-bg fill; "Delete this note? This can't be undone." + Delete + Keep buttons. |
| empty-state | hug × hug | FIXED | **Hidden by default.** Centred column in `note-area`: 60×60 dashed icon box (30 glyph), title (Cinzel 16, parchment-dim), body (body-sm, max 320 wide), "Create a note" button. |
| loading-placeholder | fill × hug | FIXED | **Hidden by default.** Centred italic "Loading notes…" (muted), padding 40 top/bottom, inside `note-area`. |
| coord-menu | {coord-menu-width} × hug | FIXED | **Hidden by default.** Frame-fixed popup anchored under a clicked `coord-chip`. Header (kicker "COORDINATE" + coord-menu-title label) + "Show on map" + "Share to chat" rows. titlebar gradient; gold-line @ 55% border. |
| toast | hug (max {toast-max}) × hug | FIXED | **Hidden by default.** Frame-fixed, bottom-centre, 96 from bottom. Padding 11 {h2}, 9 gap; check icon (16, coord) + message (body-sm); titlebar gradient; gold-line @ 50% border. |

**`new-note-form` / `edit-form` internals** (shared): padding {h1} 15;
form-top→form-bottom gradient; gold-line @ 50% border; radius {radius}. Order,
top to bottom:
`form-label` (Cinzel 13, gold) → title input (input style, black @ 32% fill) →
"CATEGORY" field-label → 5 `category-chip`s (Cinzel 12, wrapping row, {sm}/5 gap;
active = btn-fill @ 80% + gold-line @ 70%) → "SCOPE" field-label → 3
`scope-btn`s (equal flex, {sm} gap; Global/Character/Zone; active = btn-fill @ 80%)
→ optional target input (shown only when scope ≠ global; 10 top gap) →
body textarea (input fill, body-sm text, min-height 92, resize vertical, 10 top
gap) → action row (10 top gap): `save-btn` (button style, btn-fill gradient) +
`cancel-btn` (ghost) + right-aligned hint "Wrap coords like [Place — 1234, 5678]"
(caption italic).

## 4. Text elements

Each row names a base *Type scale* style; a per-element change is written
`style (field=value)`.

| id | text | style | color |
|---|---|---|---|
| title-text | "Notes" | panel-title | gold |
| title-sub | "Personal organiser" | caption | muted @ 62% |
| hotkey-badge | "HOTKEY N" | label (size=11) | muted-2 @ 75% ("N" gold) |
| cat-tree-label | "CATEGORIES" | label | muted @ 50% |
| all-notes-row | "All notes" | nav | gold-btn (active) |
| cat-header | category label | nav | muted-3 (active → gold-btn) |
| count-badge | note count | caption (weight=600, size=11) | muted @ 60% (active → gold-btn) |
| cat-note-item | note title | note-row | muted-3 (selected → parchment) |
| toolbar-heading | "All notes" | heading | parchment |
| toolbar-count | "10 notes" | caption | muted @ 50% |
| new-note-btn | "New note" | button | gold-btn |
| ctx-char | "Kytalia Moonbind" | caption (weight=500) | char-green |
| ctx-zone | "Diessa Plateau" | caption (weight=500) | zone-blue |
| note-title | note title | heading (size=15.5, line-height=1.25) | parchment |
| note-cat-pill | category label | badge | category token |
| note-scope-pill | scope (+ target) | badge | scope token |
| note-auto-pill | "Auto-surfacing here" | badge (size=10.5) | coord |
| note-body | note prose | body | body-text |
| coord-chip | "Place — x, y" | coord-chip | coord-bright |
| form-label | "NEW NOTE" | button (size=13, letter-spacing=0.5) | gold |
| field-label | "CATEGORY" / "SCOPE" | field-label (uppercase) | muted @ 50% |
| category-chip | category label | button (size=12) | muted @ 66% (active → gold-btn) |
| scope-btn | "Global"/"Character"/"Zone" | button (size=12.5) | muted @ 62% (active → gold-btn) |
| save-btn | "Save note" | button | gold-btn |
| cancel-btn | "Cancel" | button (weight=400) | muted-2 @ 80% |
| form-hint | "Wrap coords like [Place — 1234, 5678]" | caption (style=italic) | muted @ 45% |
| empty-title | "No notes yet" | heading (family=Cinzel, size=16, weight=500) | parchment-dim |
| empty-body | empty explanation | body-sm | muted @ 60% |
| loading-placeholder | "Loading notes…" | body-sm (style=italic) | muted @ 50% |
| coord-menu-kicker | "COORDINATE" | field-label (size=10.5, letter-spacing=0.63) | muted @ 55% |
| coord-menu-label | coord label | body-sm (size=13.5) | coord-menu-title |
| coord-menu-item | "Show on map" / "Share to chat" | body-sm (size=13.5) | muted-3 (hover coord-menu-title) |
| toast | confirmation message | body-sm (size=13.5) | parchment @ ~85% |

## 5. Rings / arcs / curves

No rings, arcs, or progress tracks. Non-text graphics present:

- **note-tack:** filled circle Ø {tack}, radial fill gold-btn (35%,30% origin) →
  pin-dark; centred on the card's top edge, nudged 6 au above it.
- **cat-dot:** rounded square {cat-dot} × {cat-dot}, corner radius {xxs}, solid
  category token.
- **scope-dot / pill-dot:** filled circle Ø {scope-dot}, solid scope/category
  token (some pill dots are rounded-square radius {xxs}).
- **Line icons** — all drawn from a 24×24 stroke grid, `stroke-linecap`/`join`
  round, no fill, `currentColor`. Rendered sizes: rail-notes 20; rail
  events/settings & title-icon glyph 19; toolbar 13; new-note plus, context,
  note edit/delete, close 14; search, chevron, cat-note auto, coord-chip 11;
  note-auto-pill 10; coord-menu items 15; toast check 16; empty-state 30. Stroke
  weight ~1.6–2.4 scaled to the glyph.

## 6. Reference render

`notes-overlay.render.png` — default state at 1× (1 au = 1 px). Shows the rail,
the open panel, both panes, and the top of the scrolling note list.
