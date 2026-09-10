# 08 - Text processing

## Item extraction from raw text (adding ranges)
`mods/add/add.sh` `process()` and friends split pasted/added text with:
- paragraph regexes for sentences/words;
- whitespace + lookup database (`Words`) and frequency list to keep *known*
  words only when adding words from a text (per learning level);
- `[`.txt`]`, `[audio]`, `[… ]`, HTML tag stripping (`<[^>]*>`).

## Cleaning functions (mods/add/add.sh)
- `clean_7`/`clean_8`: HTML tag removal incl. multiline (`:a;N;$!ba`), strip
  `&quot; &#039; &amp;`, collapse spaces, sentence-split on `. ? ! … [A-Z]`.
- `clean_9`: `"${1%%[,.-]*}"` — VB-style `lenght()` keeps text up to first
  `,` `.` `-` (NOT for CJK); then uppercase first char, trim, strip `*|[]&<>+`.
- Dedup: `awk 'BEGIN{RS=ORS=" "}!a[$0]++'` (word frequency dedup).
- `tr -d '…“”"'`, `tr '_' '\n'`, `sed -n 1~2p` (odd lines) for word guessing.
- Splitting long lines: byte-size 140 threshold with queues split on
  `,"` (comma-quote) → `,` → `;` → em-dash.

## Word list generation
Given target text (its `wrds` field) the app extracts unique words via:
- `list_words_2` / `list_words_3`: for non-CJK uses whitespace tokenizer,
  strips `* / “ ” "` `:` `,` `;` `(` `)`, lower+Upper first, caps 11 tokens,
  removes TRUE/FALSE literals; for CJK uses underscore split + dedup.

## Word/translation pairing (`wrds` field)
- `w.tmp` pairs: for each line `t` (target word) and `s` (translation) write
  `t_s` joined by newlines → `wrds` field (apostrophe-escaped).
- Transliteration via `tr '|'` etc. — see `transl_batch`.

## Grammar colorization & dictionaries
`ifs/mods/add/add.sh` `sentence_p` (also invoked from double-click on a note):
- Splits a sentence into token window; looks up each token in **primary
  dictionary DB** `$DS/default/dicts/$lgt` (SQLite table `Word`).
- Per-part-of-speech colored output table `sentence_parts`:
  | Part | table | color |
  |---|---|---|
  | pronouns | pronouns | `#3E539A` |
  | nouns_adjetives | nouns_adjetives | `#496E60` |
  | nouns_verbs | nouns_verbs | `#62426A` |
  | conjunctions | conjunctions | `#90B33B` |
  | prepositions | prepositions | `#D67B2D` |
  | adverbs | adverbs | `#9C68BD` |
  | adjetives | adjetives | `#3E8A3B` |
  | verbs | verbs | `#CF387F` |
- `$2` argument switches to "build" mode: writes dictionary words into result.
- The `grmr` field of an item stores the tags; colored inline in viewer via
  `<span color='#...'>`.

## Topic info / notes
`promp_topic_info` copies `note` into `$DC_tlt/note.inf`; slng mismatch text
into `slng.inf`; viewer (`notebook_1` → `tpc_view`) renders note + image +
links; editing via `dlg_text_info_1`.

## OCR
- `mods/add_process/ocr.sh` (invoked from the "OCR" button): uses
  `gocr`/`tesseract`-style pipeline on the screen clip or pasted image to
  produce text; the candidate text feeds `list_words_*`/`process`.
  (Confirmed existence in add's dispatch; exact binary flags vary by distro
  and are a candidate for service abstraction.)

## Storage format details
- Separators inside the flat `data` line: literal `{` `}` around each value.
- Apostrophes escaped for sqlite with `sed "s|'|''|g"`.
- `item` JSON (for .idmnd) uses `"key":{...}` with values JSON-escaped.