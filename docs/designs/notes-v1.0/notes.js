// notes v1.0
// Sticky Notes Overlay — data + helpers for the Guild Wars 2 Notes panel.
// Plain ES module: default seed notes, current game context, and a coordinate parser.

export const CURRENT_CONTEXT = {
  character: "Kytalia Moonbind",
  zone: "Diessa Plateau",
};

export const SCOPES = {
  global: { key: "global", label: "Global", short: "Everywhere" },
  character: { key: "character", label: "Character", short: "This character" },
  zone: { key: "zone", label: "Zone", short: "This map" },
};

// Seed notes — real Guild Wars 2 content.
export const SEED_NOTES = [
  {
    id: "n-rice",
    title: "Cook: Delicious Rice Ball",
    scope: "global",
    target: null,
    body:
      "Daily craft XP. Bowl of Rice \u00d72, Head of Lettuce \u00d71, Sesame Oil \u00d71. " +
      "Buy Rice from the Master Chef vendor. Station: [Divinity's Reach \u2014 3180, 4460].",
  },
  {
    id: "n-ore",
    title: "Iron & Platinum run",
    scope: "zone",
    target: "Diessa Plateau",
    body:
      "Ore circuit before reset. Iron cluster near the training ground " +
      "[Diessa Plateau \u2014 4200, 8600]. Platinum vein above the ridge " +
      "[Diessa Plateau \u2014 5100, 7300]. Share to guild before the run.",
  },
  {
    id: "n-twilight",
    title: "Twilight \u2014 still missing",
    scope: "global",
    target: null,
    body:
      "Gift of Twilight checklist. Gift of Metal \u2014 done. Dawn (precursor) \u2014 not yet. " +
      "Mystic Clovers \u2014 40/250. Gift of Darkness \u2014 need 250 Vials of Powerful Blood.",
  },
  {
    id: "n-ranger",
    title: "Ranger alt \u2014 to-do",
    scope: "character",
    target: "Kytalia Moonbind",
    body:
      "Turn in the Gendarran renown hearts. Swap to Superior Rune of the Pack. " +
      "Bank has 68 Ancient Wood Logs waiting for the staff craft.",
  },
  {
    id: "n-home",
    title: "Home instance daily",
    scope: "character",
    target: "Kytalia Moonbind",
    body:
      "Ori node, quartz node, then the garden plot. Mine at [Home \u2014 2400, 3100].",
  },
];

// Parse a note body into an ordered list of segments:
//   { type: "text", value } | { type: "coord", label, place, x, y, raw }
// A coordinate looks like: [Place Name — 1234, 5678]
export function parseBody(body) {
  const re = /\[([^\]\u2014\-]+?)\s*[\u2014\-]\s*(\d+)\s*,\s*(\d+)\]/g;
  const segments = [];
  let last = 0;
  let m;
  while ((m = re.exec(body)) !== null) {
    if (m.index > last) {
      segments.push({ type: "text", value: body.slice(last, m.index) });
    }
    const place = m[1].trim();
    const x = parseInt(m[2], 10);
    const y = parseInt(m[3], 10);
    segments.push({
      type: "coord",
      label: `${place} \u2014 ${x}, ${y}`,
      place,
      x,
      y,
      raw: m[0],
    });
    last = re.lastIndex;
  }
  if (last < body.length) {
    segments.push({ type: "text", value: body.slice(last) });
  }
  return segments;
}

// Does this note auto-surface given the current game context?
export function autoSurfaces(note, ctx = CURRENT_CONTEXT) {
  if (note.scope === "zone") return note.target === ctx.zone;
  if (note.scope === "character") return note.target === ctx.character;
  return false;
}
