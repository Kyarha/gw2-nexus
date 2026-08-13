---
target: notes
scope: app
version: v1.0
base: scratch
product: gw2-nexus — in-game quality-of-life addons for Guild Wars 2
tool: Claude design
status: iterating
---

# Notes — design brief (v1.0)

Create a folder `design/notes/notes-v1.0/` and build every file of this design inside it — the
JavaScript file and each HTML page. Begin each file with a version header on its first line:
`<!-- notes v1.0 -->` for HTML files, `// notes v1.0` for the JavaScript file. Keep any existing
`design/notes/notes-v*` folders as they are.

**What to build** — A sticky-notes panel that lives inside Guild Wars 2 as an on-screen overlay,
opened from a toolbar button or a hotkey. It's a personal organiser a player keeps open while they
play: jot notes, keep crafting lists, and click coordinates to show them on the map or share them to
chat. For solo-PvE players levelling crafting and chasing legendaries who don't want to alt-tab to
their own notes.

**What it must contain**

- A **Notes panel** the player opens over the game. It holds a **collection of sticky notes** the
  player can browse, plus a way to **create a new note**, **edit a note in place**, and **delete a
  note**.
- Each note has a **short title**, a **freeform text body**, and a **scope** shown on the note —
  one of: **Global** (everywhere), **Character** (only this character), or **Zone** (a named map).
  Zone and Character notes can **auto-surface** — appear on their own when the player is on that map
  or that character.
- Notes contain **clickable coordinates**. Wherever a coordinate appears in a note's text, it's
  visibly interactive and offers two actions: **Show on map** and **Share to chat**.
- An **empty state** for when there are no notes yet, and a visible **new-note** affordance.

Real content the notes must show (use these, not lorem):

- **Note — "Cook: Delicious Rice Ball"** · *Global*
  Daily craft XP. Bowl of Rice ×2, Head of Lettuce ×1, Sesame Oil ×1. Buy Rice from the Master Chef
  vendor. Station: [Divinity's Reach — 3180, 4460].

- **Note — "Iron & Platinum run"** · *Zone — Diessa Plateau*
  Ore circuit before reset. Iron cluster near the training ground [Diessa Plateau — 4200, 8600].
  Platinum vein above the ridge [Diessa Plateau — 5100, 7300]. Share to guild before the run.

- **Note — "Twilight — still missing"** · *Global*
  Gift of Twilight checklist. Gift of Metal — done. Dawn (precursor) — not yet. Mystic Clovers —
  40/250. Gift of Darkness — need 250 Vials of Powerful Blood.

- **Note — "Ranger alt — to-do"** · *Character — Kytalia Moonbind*
  Turn in the Gendarran renown hearts. Swap to Superior Rune of the Pack. Bank has 68 Ancient Wood
  Logs waiting for the staff craft.

- **Note — "Home instance daily"** · *Character — Kytalia Moonbind*
  Ori node, quartz node, then the garden plot. Mine at [Home — 2400, 3100].

**Done when**

- The player can open the panel, read the notes, create a new one, edit a note, and delete a note.
- Each note clearly shows its scope (Global / Character / Zone), and Zone/Character notes show they
  can auto-surface.
- Coordinates inside a note read as clickable and expose both **Show on map** and **Share to chat**.
- Real Guild Wars 2 content is shown throughout — never placeholder text.
- It reads as an overlay that belongs inside the Guild Wars 2 interface, opened from a toolbar
  button / hotkey.

You own the visual design — mood, colour, type, layout, imagery. Show me 3 distinct directions, and
ask me if you'd like any direction.

## Revisions

### R1 — GW2 chrome pass · design/notes/notes-v1.0
Push the panel to authentic Guild Wars 2 / Blish HUD chrome, closely matching the attached GW2 reference images:
- Frame the panel in a heavy engraved metal / brass border with beveled, layered edges and squared outer corners, and add ornamental filigree cornerpieces at the four corners.
- Give the panel background a weathered stone / aged-parchment grain, and make the whole panel translucent so a hint of the game scene shows through behind it.
- Add an engraved divider rule under the title bar, in the style of a Guild Wars 2 window header.
- Render coordinates as plain colored-text links with a small pin glyph that brighten and underline on hover, sitting inline in the sentence.
- Give the New Note button and the All / Global / Character / Zone filter tabs a brass, beveled button treatment with an inset engraved look.
- Style the scrollbar as a thin brass rail matching the frame.
- Warm the accents toward aged brass / gold and deepen the panel translucency so it reads as part of the Guild Wars 2 interface.
