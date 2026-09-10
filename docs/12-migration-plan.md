# 12 - Migration plan (phases & acceptance)

Follow the master prompt's phases. Deliver for each phase: implementation +
tests + docs. Stop at the end of each phase for review.

## Phase 0 — Scaffolding
- CMake Qt6 project with `src/`, `ui/`, `tests/`; CI-friendly build
  (Linux first), Windows/macOS stubs later (Phase 8).
- Vendored resources (`default/`, `images/`, `dicts/`) as Qt resources copied
  from `Dev/` for tests only.

## Phase 1 — Research (DONE → this docs/ set)
- Files: docs serie completa (ver `docs/00-index.md`), en especial
  `09-qt-architecture.md` (normativo), `05` (.idmnd), `06` (learning).

## Phase 2 — Domain model (`core/`) + `storage/`
- Pure C++ `Topic`, `Item`, `ItemType`, `ReviewState (stts)`,
  `ReviewCalculator` with exact `notice[]`/percent logic.
- `ConfigRepository`, `TopicRepository` (tpc DB, shrdb, tlngdb, cfgdb) with
  same schema DDL as `mkdb.sh`; **no schema migration**; tolerate legacy
  `Data`/`PractN` tables.
- Equivalence tests: given fixture `.conf/data` + `tpc` DB →
  `data/index/reviews` byte-compatible with Bash-produced output.

## Phase 3 — Import/Export + repair (`check_index`)
- `.idmnd` importer = exact `check_format_1` rules + 200-line cap + post-import
  rebuild; exporter = mods/export idmnd (fields/order identical).
- `TopicRepairService` mirroring check_index `_check`/`_restore` detection and
  rebuild paths (incl. the `type` index 13/24 fallback).
- Golden tests against `Web/idiomind.com/public/english/**/*.idmnd`.

## Phase 4 — Learning engine wiring
- `ReviewEngine` orchestrates `mark_as_learned` / `mark_to_learn` /
  `calculate_review` / `mkmn` and the T1..T10 lists; `colorize` index builder
  ported (python → C++ text/regex), same output (span/color/TRUE/line triple).
- integrate `practice/strt.sh` modes 1–5 into `PracticeService` + logs.

## Phase 5 — Services (add-note pipeline)
- `AddNoteFlow` port of `mods/add` `process()`:
  clipboard → OCR → translate → TTS(word cache + sentences) → images → write.
- Provider interfaces; built-ins: Google translate (v3 endpoint used by
  original) + TTS list; addon-script compatibility layer (execute legacy addon
  scripts for parity when present).

## Phase 6 — UI (QML)
- Notebook (Info/Index/Manage), Add dialog (dlg_form_1/2), sentence/word
  pickers (dlg_checklist_3/2), viewer, preferences (`cnfg`), tray, clip watcher,
  stats, about/help/update. Keep dialog *labels/buttons* equal (msgids).

## Phase 7 — Compatibility pass
- Import real user topics from a Bash install (folder containing old `.conf`s);
  run both, compare `index`, `reviews`, stts transitions, exported `.idmnd`
  byte-diff (allow date/nsze drift).

## Phase 8 — Cross-platform
- Windows/macOS dirs, QMediaPlayer backend, native clipboard/screen-clip,
  notifications; drop mplayer/xclip/espeak dependencies behind interfaces;
  keep dictionaries & legacy addons optional.

## Non-goals / guardrails (from master prompt)
- No Bash→C++ line translation; behavior-quirk preservation only.
- No "fixing" of original bugs in Phase 1.
- No new DB format; `.idmnd` legacy is the interchange format.
- No ui-first iteration; domain validated by tests before QML.