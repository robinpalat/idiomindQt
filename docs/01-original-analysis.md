# 01 - Original analysis (Idiomind Bash/YAD)

## What this document is
Reverse-engineering notes of the original **Idiomind 0.3.2** implemented in
Bash + YAD (GTK) + Python3 + SQLite. Ground truth for the C++/Qt6/QML
reimplementation. Behaviors documented here are the reference; the new app must
reproduce them (compatibility target), not "improve" them in Phase 1.

## Repository layout

| Path | Purpose |
|---|---|
| `Dev/` | Live source tree of the Bash implementation (~7800 LoC) |
| `Dev/idiomind` | CLI launcher |
| `Dev/main.sh` | Main entry point / session |
| `Dev/mngr.sh` | Learning manager (reviews, marks, index rebuild) |
| `Dev/add.sh` | "Add note" entry + dispatch to `mods/add/add.sh` |
| `Dev/chng.sh` | Topic selector |
| `Dev/cnfg.sh` | Preferences |
| `Dev/play.sh` | Audio playback |
| `Dev/stop.sh` | Stop/cleanup of running processes |
| `Dev/bcle.sh` | Player bootstrap |
| `Dev/vwr.sh` | Word/sentence viewer |
| `Dev/ifs/*` | Support functions and services |
| `Dev/practice/strt.sh` | Practice module |
| `Dev/addons/Resources/scripts/` | External service scripts (TTS, translation, images, dictionaries) |
| `Dev/default/` | Config, sets, templates, dicts, css |
| `Dev/locale/` | Translations (gettext) |
| `Dev/mime/`, `Dev/icons/`, `Dev/pixmaps/`, `Dev/images/` | App integration assets |
| `Repository/` | Packaged source (`idiomind-0.3.2/`) and `.deb` sources |
| `Web/idiomind.com/public/english/*/*.idmnd` | Real exported topics (test fixtures) |

## The launcher (`Dev/idiomind`)
```sh
Usage: idiomind [-s] [-t|tasks] [add <TEXT>] [-v]
  -s            start a new session (main.sh)
  tasks         run task file
  add <TEXT>    add a note from command line (add.sh)
  -v            print version
  x             debug mode: bash -x main.sh
```
Launcher also forces `GDK_BACKEND=x11` when running under Wayland.

## Runtime model
- Everything is a **dispatcher over subprocesses**. Commands are dispatched by
  case/select on `$1` (see `Dev/ifs/tls.sh` tail; `Dev/mngr.sh`; `Dev/add.sh`).
- UI is **YAD** (zenity-like GTK dialog generator) invoked as external
  process; results parsed from its stdout (`|`-separated fields).
- A **tray indicator** is a Python3 script (`itray` in `tls.sh`) using GTK3
  via `gi` — AppIndicator3 or `Gtk.StatusIcon`.
- Inter-process synchronization uses **lock/pid/task files** in
  `$DT=/tmp/.idiomind-$USER`.
- HTML rendering of topic info/attachments uses a C++ helper
  `/usr/lib/idiomind/idiomind-htmlview`.

## Dependencies (from README)
- GTK+ desktop environment, `bash`, `yad`, `python3`, `sqlite3`
- `mplayer` (playback), `imagemagick` (`convert`/`import`/`identify`),
  `wkhtmltopdf`, `curl`, `wget`, `xclip`, `espeak`, `sox`
- X11 screenshot tooling (`gnome-screenshot`/`import`) for screen clipping
- Optional: `notify-send`, gettext (`msgfmt`), `xdg-open`

## Version/limits
- `_version='0.3.2'`, requires `yad >= 0.39.0` (checked by `ifs/1u.sh`).
- Soft limits: max 200 items/topic; `nwrd|nsnt|nimg <= 200`, `naud <= 1000`,
  `nsze` len <= 6, `info` <= 10240 bytes, `levl` <= 2 digits.

## Key quirks / bugs (document, do NOT fix in Phase 1)
1. `dwld1/dwld2` MIME validation uses `grep 'audio|mpeg|mp3|'` (no `-E`), so the
   regex never matches; the file is kept if its size > 120 bytes regardless of
   MIME. `tts_sentence` uses the correct `grep -E 'audio|mpeg|mp3|ogg|wav'`.
2. `colorize` index lines group by 3 lines `<text> / TRUE|FALSE / <translation>`;
   `check_index` infers item count = index lines / 3.
3. Topics whose stts is "reviewing" (5/6) have their index string built inline
   in `mngr.sh` (not via colorize).
4. `mngr.sh mkmn` index mtime ordering: newest first (uses `find -printf %T@`).
5. `.idmnd` dates `dtec/dteu/dtei` are `%Y-%m-%d`; `nsze` in the fixture uses a
   Spanish-locale decimal comma (`"8,6M"`).