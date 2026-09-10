# 09 - Practice modes & statistics

## Entry (`practice/strt.sh`, launch via items_list manage tab)
5 practice modes (names from the notebook):

1. **Index practice** — the main list (`index`) with auto-check styles;
   checkbox `acheck` used to auto-check from log1.
2. **Practice (multiple-choice / writing)** — uses `Pract1..Pract3`,
   per-item `srce`-hidden writing test, TTS (`play.sh play_word`).
3. **Flashcard-style review** — `Pract4`.
4. **Listening practice** — audio-only (`rplay`, `loop`) — `Pract5`.
5. **Audio + replay** — re-listen to previously played items with controlled
   pauses (`pause_rep`), overlay OSD (`notify-send`, `pause_osd`).

Common practices:
- Items are selected from `learning` (+ optionally `learnt`) according to
  current `stts` (see 06), shuffled/queued, saved to `$DC_tlt/practice/log*`.
- `log1` — index-practice outcomes (checked/auto), `log2`, `log3` — marked
  failed items each cycle; the three logs feed `colorize` (red/orange index
  highlighting for items failed in the most recent cycles).
- After a practice the topic re-runs `calculate_review` / `mark_as_learned`
  and `mngr.sh mkmn`.

## Statistics (`ifs/stats.sh`)
- Builds `pg_stats.html` (web-rendered) summarizing:
  - counts by category; by learning level;
  - items per `stts`; interval progress (label_serie vs notice);
  - words vs sentences; marked items; recent reviews;
  - per-day activity from `reviews.date*` & practice logs;
- `stats_dlg` (tls.sh) opens the generated HTML.

## Topic statistics in DB
- `shrdb` `topics` and per-topic `id` row track `nwrd nsnt nimg naud nsze`
  (updated after add/remove/import) — displayed in `tpc_view`.
- `Practice_stats` table/log view records per-session statistics.

## Level & complexity gating
`labels_level[0..9]`: Fresh (0..3), Familiar (4..8), Mastered (9). Sentence
complexity guard: `sentence_words_level0=12`, `sentence_chars=180`,
`sentence_lines=2` — too-long/complex sentences at level 0 are flagged in
`dlg_checklist_3` and can be rejected/edit instructions shown.