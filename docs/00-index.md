# docs/ index — Idiomind reimplementation research (Phase 1)

| # | File | Content |
|---|---|---|
| 0 | `00-index.md` | this index |
| 1 | `01-original-analysis.md` | overview, inventory, runtime model, deps, version, quirks/bugs |
| 2 | `02-filesystem.md` | c.conf paths, topic dir matrix, global config, addons layout |
| 3 | `03-data-model.md` | `data` line format, `index`, `.tra`, backups, metadata |
| 4 | `04-databases.md` | tpcdb/sharedb/tlngdb/cfgdb schemas + invariants |
| 5 | `05-topic-format.md` | legacy `.idmnd` 3-line JSON, validation, real fixture, import/export |
| 6 | `06-learning-system.md` | stts state machine, notice[] schedule, mark_as_learned/to_learn |
| 7 | `07-services.md` | translation, TTS, images, definition/update/addons; endpoints & URLs |
| 8 | `08-text-processing.md` | cleaners, word lists, sentence partition, grammar colors & dicts |
| 9 | `09-qt-architecture.md` | **[NORMATIVO]** arquitectura Qt6/QML: clases, repos, servicios, models, QML, CMake, tests, compat |
| 10 | `10-practice.md` | 5 practice modes, logs, stats |
| 11 | `11-ui-flows.md` | YAD dialogs, notebooks, tray, helpers, interaction contract |
| 12 | `12-migration-plan.md` | Phases 0–8 with acceptance criteria & guardrails |
| 13 | `13-qt-architecture-draft.md` | ~~early draft of architecture~~ RETIRADO / superseded by `09` |

Ground-truth locations outside `docs/`:
- Original source: `../` (Dev tree).
- Packaged source/deb: `../../Repository/`.
- Real `.idmnd` fixtures: `../../Web/idiomind.com/public/english/**/*.idmnd`.

Companion artifacts still to generate in later phases (F0+): dependency
matrix script→function→data and golden fixtures (`tests/`).