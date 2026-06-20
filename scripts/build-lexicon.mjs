import words from "an-array-of-english-words" with { type: "json" };
import { mkdirSync, writeFileSync } from "node:fs";

const targetSize = 320000;

const pinned = [
  "man", "woman", "boy", "girl", "help", "mother", "father", "student", "teacher",
  "doctor", "nurse", "phone", "medicine", "airport", "banana", "zebra", "music",
  "garden", "library", "school", "family", "friend", "home", "work", "money",
  "food", "water", "story", "letter", "window", "street", "morning", "night"
  , "hello", "hi", "hey", "thanks", "please", "yes", "no", "maybe", "everything",
  "anything", "nothing", "something", "someone", "everyone", "anyone", "today",
  "tomorrow", "yesterday", "soon", "later", "early", "late", "good", "bad",
  "better", "best", "new", "old", "first", "last", "big", "small", "long",
  "short", "fast", "slow", "easy", "hard", "simple", "weird", "normal",
  "computer", "internet", "screen", "keyboard", "mouse", "camera", "picture",
  "video", "book", "paper", "table", "chair", "car", "bus", "train", "bike",
  "city", "village", "country", "river", "mountain", "beach", "forest", "animal",
  "dog", "cat", "bird", "horse", "fish", "apple", "orange", "bread", "coffee",
  "tea", "house", "apartment", "room", "door", "floor", "ceiling", "wall",
  "shirt", "shoe", "coat", "bag", "box", "game", "movie", "song", "news",
  "idea", "plan", "goal", "job", "skill", "tool", "code", "data", "model",
  "search", "engine", "word", "sentence", "document", "result", "answer",
  "question", "person", "people", "child", "adult", "baby", "brother", "sister",
  "husband", "wife", "neighbor", "doctor", "lawyer", "artist", "writer", "chef",
  "police", "firefighter", "driver", "pilot", "farmer", "worker", "manager"
];

function hashWord(word) {
  let hash = 2166136261;
  for (let i = 0; i < word.length; i += 1) {
    hash ^= word.charCodeAt(i);
    hash = Math.imul(hash, 16777619);
  }
  return hash >>> 0;
}

const filtered = [...new Set(words)]
  .filter((word) => /^[a-z]+$/.test(word))
  .filter((word) => word.length >= 3 && word.length <= 12)
  .filter((word) => !/(.)\1\1/.test(word))
  .filter((word) => !word.endsWith("ing") || word.length <= 9);

function variants(word) {
  if (word.length < 4 || word.length > 10) {
    return [];
  }
  const suffixes = ["s", "ed", "er", "ers", "ing", "ly", "ness", "able"];
  const prefixes = ["re", "un", "pre", "post", "anti", "micro"];
  return [
    ...suffixes.map((suffix) => `${word}${suffix}`),
    ...prefixes.map((prefix) => `${prefix}${word}`)
  ].filter((candidate) => candidate.length >= 3 && candidate.length <= 16);
}

const expanded = new Set([...pinned, ...filtered]);
const variantSeeds = [...filtered].sort((a, b) => hashWord(a) - hashWord(b));
for (const word of variantSeeds) {
  for (const variant of variants(word)) {
    expanded.add(variant);
    if (expanded.size >= targetSize) {
      break;
    }
  }
  if (expanded.size >= targetSize) {
    break;
  }
}

const lexicon = [...expanded].sort((a, b) => a.localeCompare(b));
mkdirSync(new URL("../data", import.meta.url), { recursive: true });
writeFileSync(new URL("../data/lexicon.txt", import.meta.url), `${lexicon.join("\n")}\n`);
console.log(`wrote ${lexicon.length} words to data/lexicon.txt`);
