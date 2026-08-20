// notes v2.0
// Sticky Notes Overlay — data + helpers for the Guild Wars 2 Notes panel.
// Plain ES module: categories, seed notes, current game context, coordinate parser.

export const CURRENT_CONTEXT = {
  character: "Kytalia Moonbind",
  zone: "Diessa Plateau",
};

export const SCOPES = {
  global: { key: "global", label: "Global", short: "Everywhere", color: "#e6c86a" },
  character: { key: "character", label: "Character", short: "This character", color: "#9fd8b0" },
  zone: { key: "zone", label: "Zone", short: "This map", color: "#a7c4ea" },
};

// Collapsible groupings shown in the left pane.
export const CATEGORIES = [
  { key: "crafting", label: "Crafting", color: "#e6c86a" },
  { key: "gathering", label: "Gathering", color: "#9fd8b0" },
  { key: "legendaries", label: "Legendaries", color: "#c9a3e6" },
  { key: "dailies", label: "Dailies", color: "#a7c4ea" },
  { key: "characters", label: "Characters", color: "#e0a58a" },
];

// Seed notes — real Guild Wars 2 content.
export const SEED_NOTES = [
  {
    id: "n-rice",
    title: "Cook: Delicious Rice Ball",
    category: "crafting",
    scope: "global",
    target: null,
    body:
      "Daily craft XP. Bowl of Rice \u00d72, Head of Lettuce \u00d71, Sesame Oil \u00d71. " +
      "Buy Rice from the Master Chef vendor. Station: [Divinity's Reach \u2014 3180, 4460].",
  },
  {
    id: "n-food",
    title: "Ascended food batch",
    category: "crafting",
    scope: "global",
    target: null,
    body:
      "Craft Bowl of Sweet & Spicy Butternut Squash Soup for the raid. " +
      "Need Butternut Squash \u00d76, Coconut Milk \u00d73. Chef 500 station in the guild hall.",
  },
  {
    id: "n-ore",
    title: "Iron & Platinum run",
    category: "gathering",
    scope: "zone",
    target: "Diessa Plateau",
    body:
      "Ore circuit before reset. Iron cluster near the training ground " +
      "[Diessa Plateau \u2014 4200, 8600]. Platinum vein above the ridge " +
      "[Diessa Plateau \u2014 5100, 7300]. Share to guild before the run.",
  },
  {
    id: "n-ori",
    title: "Rich Orichalcum circuit",
    category: "gathering",
    scope: "global",
    target: null,
    body:
      "Guild rich Ori nodes, home-instance mining pick equipped. " +
      "Start at [Malchor's Leap \u2014 6400, 5200], then loop the two hidden nodes east.",
  },
  {
    id: "n-twilight",
    title: "Twilight \u2014 still missing",
    category: "legendaries",
    scope: "global",
    target: null,
    body:
      "Gift of Twilight checklist. Gift of Metal \u2014 done. Dawn (precursor) \u2014 not yet. " +
      "Mystic Clovers \u2014 40/250. Gift of Darkness \u2014 need 250 Vials of Powerful Blood.",
  },
  {
    id: "n-bifrost",
    title: "The Bifrost \u2014 dye materials",
    category: "legendaries",
    scope: "global",
    target: null,
    body:
      "Gift of Color needs 100 of each basic dye pigment. Missing Yellow and Blue. " +
      "Buy from the dye trader or farm charr in Iron Marches. Mystic Clovers 62/250.",
  },
  {
    id: "n-home",
    title: "Home instance daily",
    category: "dailies",
    scope: "character",
    target: "Kytalia Moonbind",
    body:
      "Ori node, quartz node, then the garden plot. Mine at [Home \u2014 2400, 3100].",
  },
  {
    id: "n-fractal",
    title: "Daily T4 fractals + recs",
    category: "dailies",
    scope: "global",
    target: null,
    body:
      "Three daily T4 + three recommended for the pristine relics. " +
      "Bring +9 agony infusions. Meet party at [Lion's Arch \u2014 6100, 4900].",
  },
  {
    id: "n-ranger",
    title: "Ranger alt \u2014 to-do",
    category: "characters",
    scope: "character",
    target: "Kytalia Moonbind",
    body:
      "Turn in the Gendarran renown hearts. Swap to Superior Rune of the Pack. " +
      "Bank has 68 Ancient Wood Logs waiting for the staff craft.",
  },
  {
    id: "n-guardian",
    title: "Guardian \u2014 WvW build",
    category: "characters",
    scope: "character",
    target: "Bront Ironhide",
    body:
      "Respec to support Firebrand for zerg nights. Need Diviner's trinkets from the vendor. " +
      "Stash the marauder set in the bank for roaming.",
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
