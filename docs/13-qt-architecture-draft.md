> **RETIRADO / SUPERSEDED** — sustituido por `docs/09-qt-architecture.md`
> (documento normativo y completo). Consérvese solo como historial del primer
> borrador de capas.

# 11 - Proposed Qt6/QML architecture (draft)

Target: **C++ + Qt 6 + QML (Qt Quick)** + CMake, runnable on Linux/Windows/macOS.
Original Bash quirks are preserved as behavior; plain C++ logic, QML only for
presentation.

## Layered layout (src/)
```
src/
  app/          main.cpp, App init (Qt.applicationDirPath, config paths, logging)
  core/         Domain entities + pure logic
    topic/      Topic, TopicInfo, Item (trgt/srce/...), ItemType, StateMachine (stts)
    learning/   ReviewCalculator (notice[] intervals, days_to_review_porcent,
                T1..T10 list builders, mark_as_learned / mark_to_learn)
    settings/   Settings (c.conf-like), Language maps (slangs[]/tlangs[]),
                Sets (notice, labels_level, sentence limits)
    config/     ConfigRepository (cfgdb)
  storage/      Repository layer: SQLite (QSqlDatabase) — topic db, shrdb, tlngdb,
                cfgdb; FileStore (data/index/stts/translations), Backup (.bk),
                .idmnd Importer/Exporter (3-line JSON, check_format_1 validator),
                Inotify/file watchers (mirror of $DT locks, tray refresh)
  services/     Interfaces + adapters (net via QNetworkAccessManager)
    ITranslationProvider, IPronunciationProvider, IImageProvider,
    IDictionaryProvider (grammar/sentence_p), IUpdateService, IClipWatcher
    impl/ GoogleTranslateProvider, TTS providers (online/offline)
  player/       Audio playback abstraction (cross-platform; original used mplayer)
  platform/     multi-platform dirs (XDG/AppData), audio, notifications,
                screen-clip, browser-open, tray
  ui/qml/       QML views: Home, TopicList, Notebook(Info/Index/Manage),
                AddNoteDialog, WordItemForm, SentenceForm, PracticeViews,
                Preferences, About; delegates for index rows
tests/          equivalence tests vs Bash fixtures (data/idmnd artifacts)
```

## Mapping original → Qt
| Original (Bash) | Qt component |
|---|---|
| `c.conf` env vars | `Settings`/`PlatformPaths` (Qt StandardPaths for XDG) |
| `sets.cfg` arrays | `Sets` structs (constrained arrays, not env) |
| `cmns.sh` dialogs | QML `Dialog` subclasses / reusable `MessageBox.qml` |
| `items_list.sh notebook_*` | `NotebookView.qml` (3 tabs) |
| `add.sh / mods/add` `process` | `AddNoteFlow` controller (async task graph) |
| `mngr.sh` review | `ReviewEngine` (pure logic, unit-testable) |
| `tls.sh` service dispatch (case) | Service registry / command bus |
| addons scripts | Plugin-style provider registries + defaults built-in |
| `check_index` repair | `TopicRepairService` (validators in C++, not python) |
| `play.sh` mplayer | `AudioPlayer` (QMediaPlayer) |
| `itray` python | QSystemTrayIcon + QMenu |
| `.idmnd` validation python | C++ JSON validator + regexes |

## Concurrency & timing
Original relies on sleeps/locks (`sleep 1`, `> lock`, pidfiles). Qt replaces
with:
- `QNetworkAccessManager` (async, no sleeps). Where the original **deliberately**
  serialized two network calls (`translate` then sleep), preserve ordering but
  with explicit sequencing; wrap original pause semantics in the provider.
- Lock/pid semantics (`msg_4` "Wait/Stop") become `QFutureWatcher` /
  cancellable `Task` objects; keep the "translations/active" shared-folder
  contract for *data compatibility* (the folder layout must remain readable by
  a parallel Bash install during migration).

## Language & i18n
- gettext catalogs in `locale/` → Qt `QTranslator` + `tr()`. Preserve msgids.

## Cross-platform notes
- `$DM/$DC/DS/DT` map to `QStandardPaths::AppDataLocation`/`ConfigLocation`,
  plus an emulated `/usr/share/idiomind` resource bundle for Qt resources
  (images, default dicts, gtkrc → replaced by QML theme).
- X11-only pieces (import/gnome-screenshot, xclip) abstracted behind
  `IScreenClip`/`IClipboard` interfaces; Windows hooks implemented natively.
- mplayer → QMediaPlayer; `wkhtmltopdf` → `QTextDocument::print` or `QPdfWriter`.