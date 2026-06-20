import type { DemoDocument } from "./types";
import { generalWords } from "./lexicon";

const departments = [
  "Planning",
  "Fire Prevention",
  "Stormwater",
  "Transportation",
  "Public Works",
  "Building Safety",
  "Environmental Review",
  "Utilities",
  "Housing",
  "Parks",
  "Health",
  "Historic Preservation",
  "Economic Development",
  "Resilience"
];

const jurisdictions = [
  "Oak Meadow",
  "Northfield",
  "San Paloma",
  "Rivergate",
  "Lakeview",
  "Cedar Falls",
  "Westport",
  "Summit Ridge",
  "Marina Point",
  "Eastbank",
  "Prairie Crossing",
  "Granite Bay",
  "Willow Creek",
  "Harborview"
];

type Topic = {
  name: string;
  nouns: string[];
  verbs: string[];
  adjectives: string[];
  objects: string[];
};

const topics: Topic[] = [
  {
    name: "stormwater",
    nouns: ["basin", "outlet", "swale", "runoff", "inlet", "culvert", "watershed", "forebay", "underdrain", "weir", "spillway", "channel", "riprap", "sediment", "rainfall", "conveyance", "overflow", "embankment"],
    verbs: ["captures", "routes", "detains", "filters", "discharges", "stabilizes", "attenuates", "bypasses", "conveys", "meters", "ponds", "spreads", "releases", "treats"],
    adjectives: ["clean", "green", "shallow", "pervious", "temporary", "downstream", "upstream", "vegetated", "armored", "regional", "subsurface", "modular", "preliminary"],
    objects: ["hydrology report", "erosion plan", "detention calculation", "grading sheet", "drainage map", "maintenance covenant", "storm sewer profile", "floodplain exhibit", "soil boring log", "best management practice table"]
  },
  {
    name: "fire",
    nouns: ["riser", "alarm", "detector", "corridor", "stair", "hydrant", "occupancy", "standpipe", "strobe", "annunciator", "pull station", "fire lane", "hose", "extinguisher", "damper", "rating", "vestibule", "sprinkler"],
    verbs: ["protects", "annunciates", "separates", "verifies", "labels", "serves", "pressurizes", "illuminates", "supervises", "activates", "notifies", "isolates", "tests", "maintains"],
    adjectives: ["clear", "rated", "manual", "automatic", "visible", "accessible", "supervised", "monitored", "addressable", "redundant", "tamperproof", "illuminated", "dedicated"],
    objects: ["sprinkler schedule", "egress diagram", "alarm sequence", "riser detail", "life safety plan", "fire flow letter", "smoke control matrix", "occupancy analysis", "rated assembly schedule", "inspection checklist"]
  },
  {
    name: "zoning",
    nouns: ["parcel", "setback", "height", "frontage", "yard", "lot", "variance", "easement", "overlay", "density", "floor area", "use", "district", "boundary", "encroachment", "screening", "frontage", "massing"],
    verbs: ["matches", "exceeds", "reduces", "preserves", "confirms", "measures", "encroaches", "calculates", "classifies", "screens", "buffers", "establishes", "limits", "documents"],
    adjectives: ["clean", "nonconforming", "corner", "adjacent", "minimum", "existing", "proposed", "allowable", "conditional", "mixed-use", "residential", "commercial", "transitional"],
    objects: ["site plan", "survey exhibit", "zoning table", "variance narrative", "bulk regulation worksheet", "land use affidavit", "frontage diagram", "neighborhood compatibility memo", "legal description", "planning staff report"]
  },
  {
    name: "traffic",
    nouns: ["driveway", "crosswalk", "queue", "signal", "aisle", "trip", "parking", "median", "turn lane", "sidewalk", "bicycle lane", "loading bay", "curb", "intersection", "sightline", "platoon", "headway", "access"],
    verbs: ["aligns", "calms", "separates", "queues", "connects", "controls", "channels", "counts", "times", "models", "widens", "narrows", "prioritizes", "accommodates"],
    adjectives: ["safe", "clear", "shared", "peak", "striped", "protected", "signalized", "unsignalized", "two-way", "one-way", "accessible", "congested", "multimodal"],
    objects: ["traffic memo", "turning exhibit", "parking count", "access plan", "trip generation table", "signal warrant", "queue analysis", "pedestrian study", "loading management plan", "traffic impact report"]
  },
  {
    name: "utilities",
    nouns: ["main", "meter", "valve", "easement", "sewer", "service", "trench", "manhole", "hydrant", "lateral", "transformer", "duct bank", "backflow", "cleanout", "vault", "conduit", "pump", "tap"],
    verbs: ["connects", "isolates", "relocates", "serves", "crosses", "protects", "abandons", "sizes", "meters", "vents", "buries", "sleeves", "pressurizes", "coordinates"],
    adjectives: ["clean", "public", "private", "buried", "temporary", "abandoned", "pressurized", "gravity", "domestic", "sanitary", "overhead", "underground", "redundant"],
    objects: ["utility sheet", "service profile", "water plan", "easement exhibit", "sewer capacity letter", "meter bank detail", "backflow certificate", "dry utility plan", "trench restoration note", "connection permit"]
  },
  {
    name: "structure",
    nouns: ["beam", "column", "footing", "slab", "wall", "frame", "connection", "joist", "girder", "shear wall", "anchor", "diaphragm", "brace", "ledger", "pier", "mat", "truss", "lintel"],
    verbs: ["supports", "transfers", "braces", "anchors", "spans", "reinforces", "carries", "resists", "stiffens", "ties", "frames", "bears", "distributes", "retrofits"],
    adjectives: ["steel", "concrete", "temporary", "continuous", "lateral", "clean", "composite", "reinforced", "post-tensioned", "masonry", "seismic", "cantilevered", "prefabricated"],
    objects: ["structural note", "foundation plan", "inspection schedule", "load table", "framing plan", "special inspection form", "calculation package", "shop drawing", "seismic narrative", "connection detail"]
  },
  {
    name: "environmental",
    nouns: ["wetland", "tree", "buffer", "habitat", "soil", "noise", "species", "canopy", "sediment", "aquifer", "shade", "emission", "contaminant", "stream", "meadow", "root zone", "pollinator", "microclimate"],
    verbs: ["protects", "restores", "mitigates", "screens", "samples", "monitors", "preserves", "remediates", "replants", "shades", "absorbs", "reduces", "surveys", "documents"],
    adjectives: ["native", "clean", "sensitive", "restored", "seasonal", "mature", "disturbed", "contaminated", "low-impact", "riparian", "urban", "shaded", "resilient"],
    objects: ["tree inventory", "wetland report", "noise study", "mitigation plan", "soil management plan", "habitat assessment", "environmental checklist", "arborist letter", "remediation protocol", "planting schedule"]
  },
  {
    name: "accessibility",
    nouns: ["ramp", "landing", "route", "door", "handrail", "clearance", "threshold", "elevator", "fixture", "counter", "signage", "tactile warning", "slope", "maneuvering space", "restroom", "parking stall", "lift", "path"],
    verbs: ["provides", "maintains", "aligns", "slopes", "connects", "serves", "contrasts", "widens", "levels", "marks", "mounts", "reaches", "illuminates", "confirms"],
    adjectives: ["accessible", "clear", "level", "continuous", "detectable", "clean", "compliant", "sloped", "van-accessible", "reachable", "unobstructed", "flush", "automatic"],
    objects: ["accessibility sheet", "door schedule", "route diagram", "ramp detail", "fixture mounting table", "signage plan", "parking exhibit", "tactile warning note", "restroom enlargement", "ADA compliance matrix"]
  },
  {
    name: "energy",
    nouns: ["insulation", "envelope", "glazing", "solar", "inverter", "panel", "battery", "thermal bridge", "fixture", "sensor", "commissioning", "air barrier", "heat pump", "economizer", "meter", "duct", "roof", "wall"],
    verbs: ["models", "offsets", "seals", "insulates", "generates", "stores", "balances", "meters", "commissions", "dims", "ventilates", "recovers", "shades", "calibrates"],
    adjectives: ["efficient", "electric", "renewable", "low-carbon", "continuous", "high-performance", "daylit", "insulated", "airtight", "grid-ready", "clean", "passive", "operable"],
    objects: ["energy model", "lighting schedule", "commissioning report", "solar layout", "envelope section", "mechanical narrative", "load calculation", "air leakage test", "panel schedule", "equipment cut sheet"]
  },
  {
    name: "housing",
    nouns: ["unit", "bedroom", "lease", "tenant", "affordability", "amenity", "courtyard", "mailroom", "balcony", "laundry", "lobby", "management", "bedspace", "studio", "townhome", "density", "relocation", "screening"],
    verbs: ["provides", "reserves", "counts", "leases", "notices", "screens", "clusters", "orients", "phases", "allocates", "manages", "documents", "certifies", "maintains"],
    adjectives: ["affordable", "market-rate", "accessible", "family-sized", "senior", "transitional", "clean", "secure", "shared", "private", "livable", "mixed-income", "supportive"],
    objects: ["unit matrix", "affordability covenant", "tenant plan", "relocation memo", "amenity schedule", "management plan", "floor plan", "phasing exhibit", "mailroom detail", "housing compliance letter"]
  },
  {
    name: "health",
    nouns: ["clinic", "exam room", "ventilation", "sink", "waste", "storage", "refrigerator", "isolation", "corridor", "nurse", "patient", "specimen", "pharmacy", "lab", "intake", "cleaning", "handwash", "privacy"],
    verbs: ["separates", "stores", "ventilates", "sanitizes", "screens", "isolates", "documents", "locks", "labels", "disposes", "protects", "maintains", "monitors", "serves"],
    adjectives: ["clean", "sterile", "secure", "clinical", "private", "negative-pressure", "refrigerated", "washable", "sealed", "controlled", "public", "staff-only", "temporary"],
    objects: ["health plan", "infection control note", "waste management plan", "ventilation schedule", "clinic layout", "equipment list", "privacy narrative", "storage detail", "handwash diagram", "operations protocol"]
  },
  {
    name: "historic",
    nouns: ["facade", "cornice", "window", "masonry", "storefront", "district", "landmark", "trim", "brick", "arch", "sign", "canopy", "transom", "roofline", "terra cotta", "entry", "plaque", "alteration"],
    verbs: ["preserves", "restores", "repairs", "documents", "matches", "replaces", "retains", "cleans", "repoints", "salvages", "aligns", "reveals", "protects", "photographs"],
    adjectives: ["historic", "compatible", "original", "painted", "clean", "reversible", "decorative", "damaged", "restored", "period", "contributing", "noncontributing", "weathered"],
    objects: ["historic survey", "facade drawing", "materials palette", "window schedule", "preservation memo", "photographic log", "masonry repair note", "signage proposal", "storefront section", "landmark certificate"]
  },
  {
    name: "parks",
    nouns: ["playground", "trail", "bench", "field", "shade", "pavilion", "court", "tree", "garden", "lighting", "fence", "plaza", "restroom", "splash pad", "path", "lawn", "soil", "irrigation"],
    verbs: ["connects", "shades", "drains", "lights", "seats", "buffers", "programs", "maintains", "irrigates", "screens", "activates", "protects", "resurfaces", "plants"],
    adjectives: ["public", "clean", "durable", "shaded", "inclusive", "seasonal", "native", "flexible", "safe", "permeable", "low-maintenance", "recreational", "visible"],
    objects: ["park plan", "landscape schedule", "play equipment sheet", "trail profile", "lighting photometric", "irrigation plan", "maintenance note", "site furnishing list", "tree protection plan", "recreation program"]
  }
];

const commonNouns = [
  "applicant", "reviewer", "sheet", "detail", "note", "condition", "submittal", "record", "area", "plan", "schedule", "calculation", "attachment", "revision", "comment", "deficiency", "narrative", "exhibit", "permit", "approval", "inspection", "coordination", "drawing", "matrix", "checklist", "photograph", "markup", "response", "standard", "policy", "deadline", "signature", "scope", "phase", "contractor", "owner", "consultant", "agency", "field", "room", "corridor", "site", "building", "operation", "maintenance", "access", "recording", "certificate", "waiver", "condition"
];

const extraNouns = [
  "ledger", "archive", "workflow", "parcel map", "dashboard", "sensor", "gateway", "rooftop", "staging", "courtyard", "ordinance", "landscape", "pipeline", "warehouse", "terminal", "canopy", "podium", "garage", "commons", "walkway", "terrace", "atrium", "boiler", "generator", "switchgear", "ductwork", "condenser", "clerestory", "awning", "escalator", "balustrade", "kiosk", "vestibule", "loading dock", "seating", "fixture", "assembly", "partition", "receptor", "spill", "survey", "forecast", "variance", "dataset", "index", "search", "latency", "throughput", "archive", "transcript", "appeal", "hearing", "bond", "escrow", "invoice", "receipt", "warranty", "sample", "laboratory", "hazard", "basement", "mezzanine", "penthouse", "alley", "promenade", "gateway", "monument", "elevation", "profile", "section", "legend", "symbol", "identifier", "packet", "recordset", "threshold", "scenario", "constraint",
  "man", "woman", "boy", "girl", "child", "adult", "parent", "mother", "father", "guardian", "neighbor", "resident", "visitor", "worker", "student", "teacher", "driver", "cyclist", "pedestrian", "patient", "nurse", "doctor", "clerk", "manager", "volunteer", "family", "household", "customer", "vendor", "operator", "engineer", "designer", "planner", "analyst", "coordinator", "assistant", "helper", "crew", "team", "group", "community", "public", "help", "support", "service", "request", "question", "answer", "message", "conversation", "translation", "language", "instruction", "guide", "manual", "training", "lesson", "story", "example", "case", "need", "problem", "solution", "care", "aid", "comfort", "shelter", "food", "water", "medicine", "transport", "appointment", "meeting", "office", "desk", "phone", "email", "form"
];

const sharedAdjectives = [
  "clear", "complete", "missing", "revised", "coordinated", "consistent", "final", "draft", "legible", "clean", "accurate", "current", "conflicting", "dated", "scanned", "digital", "signed", "sealed", "measurable", "minor", "major", "open", "resolved", "urgent", "routine", "conditional", "formal", "informal", "temporary", "permanent", "visible", "internal", "external", "public", "private", "shared", "separate", "verified", "unverified", "numbered", "referenced"
];

const extraAdjectives = [
  "granular", "redundant", "archived", "portable", "dense", "sparse", "linear", "curved", "modular", "compact", "remote", "onsite", "offline", "online", "batch", "live", "quiet", "noisy", "elevated", "submerged", "glazed", "opaque", "transparent", "rough", "smooth", "narrow", "wide", "shaded", "exposed", "phased", "deferred", "accelerated", "manual", "automated", "scalable", "local", "regional", "citywide", "temporary", "permanent", "primary", "secondary", "tertiary", "nominal", "critical", "optional", "mandatory", "adjacent", "distant", "central", "peripheral", "mapped", "unmapped", "indexed", "compressed", "ranked", "synthetic", "natural", "semantic"
];

const extraVerbs = [
  "compares", "merges", "splits", "summarizes", "extracts", "indexes", "compresses", "ranks", "loads", "streams", "parses", "checks", "audits", "scores", "traces", "projects", "samples", "estimates", "balances", "normalizes", "groups", "filters", "sorts", "allocates", "resolves", "defers", "approves", "rejects", "queues", "escalates", "notices", "publishes", "archives", "imports", "exports", "maps", "links", "binds", "reconciles", "predicts", "classifies", "segments", "labels", "annotates", "measures", "observes", "rotates", "raises", "lowers", "extends", "shortens", "locks", "unlocks",
  "helps", "guides", "teaches", "calls", "asks", "answers", "serves", "meets", "walks", "drives", "carries", "shares", "explains", "reads", "writes", "translates", "visits", "greets", "supports", "assists", "trains", "comforts", "protects", "finds", "uses", "opens", "closes", "sends", "receives", "records", "reminds", "invites", "joins", "leaves"
];

const extraObjects = [
  "change log", "review queue", "permit ledger", "document archive", "field tablet", "inspection photo", "cost estimate", "project dashboard", "routing slip", "response packet", "appeal file", "public comment", "service ticket", "asset register", "sensor feed", "data extract", "search trace", "ranking sample", "latency chart", "compression report", "batch manifest", "map layer", "parcel export", "work order", "maintenance ticket", "closeout binder", "ordinance excerpt", "meeting agenda", "hearing notice", "approval letter", "denial letter", "material receipt", "warranty file", "lab result", "sampling plan", "safety bulletin", "operations log", "handoff note", "scenario model", "risk register",
  "help request", "service request", "family record", "visitor log", "student form", "patient note", "translation guide", "training manual", "phone message", "email thread", "support ticket", "community survey", "resident letter", "appointment reminder", "case note", "care plan", "intake form", "question list", "answer sheet", "instruction page", "office memo", "conversation transcript", "volunteer roster", "crew assignment", "teacher note", "driver schedule"
];

const determiners = ["the", "a", "this", "each", "one", "another", "that"];
const prepositions = ["near", "within", "beside", "across", "under", "over", "behind", "before", "after", "through", "around", "between", "inside", "outside", "along"];
const conjunctions = ["and", "while", "as", "but", "because", "unless", "when"];
const adverbs = ["clearly", "partly", "quietly", "formally", "accurately", "recently", "visibly", "roughly", "carefully", "directly", "separately", "consistently"];

function articleFor(noun: string, rand: () => number) {
  if (rand() < 0.28) {
    return pick(["this", "each", "that"], rand);
  }
  return /^[aeiou]/i.test(noun) ? pick(["the", "an"], rand) : pick(["the", "a"], rand);
}

function mulberry32(seed: number) {
  return () => {
    let t = (seed += 0x6d2b79f5);
    t = Math.imul(t ^ (t >>> 15), t | 1);
    t ^= t + Math.imul(t ^ (t >>> 7), t | 61);
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

function pick<T>(items: readonly T[], rand: () => number) {
  return items[Math.floor(rand() * items.length) % items.length];
}

function wordBank(topic: Topic, secondary: Topic) {
  return {
    nouns: [...topic.nouns, ...secondary.nouns, ...commonNouns, ...extraNouns],
    verbs: [...topic.verbs, ...secondary.verbs, ...extraVerbs],
    adjectives: [...topic.adjectives, ...secondary.adjectives, ...sharedAdjectives, ...extraAdjectives],
    objects: [...topic.objects, ...secondary.objects, ...extraObjects],
    general: generalWords
  };
}

function nounPhrase(bank: ReturnType<typeof wordBank>, rand: () => number) {
  const noun = rand() < 0.42 ? pick(bank.general, rand) : pick(bank.nouns, rand);
  const modifiers: string[] = [];
  const adjectiveCount = rand() < 0.22 ? 2 : rand() < 0.82 ? 1 : 0;
  const usedAdjectives = new Set<string>();

  for (let i = 0; i < adjectiveCount; i += 1) {
    let adjective = rand() < 0.22 ? pick(bank.general, rand) : pick(bank.adjectives, rand);
    for (let attempts = 0; usedAdjectives.has(adjective) && attempts < 4; attempts += 1) {
      adjective = pick(bank.adjectives, rand);
    }
    usedAdjectives.add(adjective);
    modifiers.push(adjective);
  }

  const firstContentWord = modifiers[0] ?? noun;
  const determiner = rand() < 0.7 ? articleFor(firstContentWord, rand) : pick(determiners, rand);
  return [determiner, ...modifiers, noun].join(" ");
}

function objectPhrase(bank: ReturnType<typeof wordBank>, rand: () => number) {
  if (rand() < 0.34) {
    const object = rand() < 0.28 ? `${pick(bank.general, rand)} ${pick(bank.general, rand)}` : pick(bank.objects, rand);
    return `${articleFor(object, rand)} ${object}`;
  }
  return nounPhrase(bank, rand);
}

function predicate(bank: ReturnType<typeof wordBank>, rand: () => number) {
  const words: string[] = [];
  if (rand() < 0.32) {
    words.push(pick(adverbs, rand));
  }
  words.push(pick(bank.verbs, rand));
  words.push(objectPhrase(bank, rand));

  const phraseCount = rand() < 0.2 ? 2 : rand() < 0.72 ? 1 : 0;
  for (let i = 0; i < phraseCount; i += 1) {
    words.push(pick(prepositions, rand));
    words.push(objectPhrase(bank, rand));
  }

  return words.join(" ");
}

function sentence(topic: Topic, secondary: Topic, rand: () => number) {
  const bank = wordBank(topic, secondary);
  const clauses = [`${nounPhrase(bank, rand)} ${predicate(bank, rand)}`];

  if (rand() < 0.42) {
    clauses.push(`${pick(conjunctions, rand)} ${nounPhrase(bank, rand)} ${predicate(bank, rand)}`);
  }
  if (rand() < 0.18) {
    clauses.push(`${pick(conjunctions, rand)} ${nounPhrase(bank, rand)} ${predicate(bank, rand)}`);
  }

  const text = clauses.join(" ");
  return `${text.charAt(0).toUpperCase()}${text.slice(1)}.`;
}

function paragraph(topic: Topic, secondary: Topic, rand: () => number) {
  const sentenceCount = 5 + Math.floor(rand() * 4);
  const sentences = Array.from({ length: sentenceCount }, () => sentence(topic, secondary, rand));
  return sentences.join(" ");
}

export function generateDocuments(count: number): DemoDocument[] {
  const rand = mulberry32(20260619);
  const documents: DemoDocument[] = [];

  for (let i = 0; i < count; i += 1) {
    const topic = topics[i % topics.length];
    const secondary = topics[(i * 5 + 3) % topics.length];
    const department = pick(departments, rand);
    const jurisdiction = pick(jurisdictions, rand);
    const year = 2020 + (i % 7);
    const pages = 12 + Math.floor(rand() * 240);
    const titleFocus = pick([...topic.nouns, ...topic.objects], rand);
    const title = `${jurisdiction} ${titleFocus} review packet ${year}`;
    const sectionCount = 2;
    const body = Array.from({ length: sectionCount }, (_, sectionIndex) =>
      paragraph(sectionIndex === 0 ? topic : secondary, sectionIndex === 0 ? secondary : topic, rand)
    ).join("\n\n");

    documents.push({
      id: i,
      externalId: `AT-${year}-${String(i + 1).padStart(6, "0")}`,
      title,
      department,
      jurisdiction,
      year,
      pages,
      body
    });
  }

  return documents;
}
