# 02 - Filesystem layout & configuration

Source: `Dev/default/c.conf`. Evaluated by `source` in every script.

## Variables
```sh
DT    = /tmp/.idiomind-$USER          # runtime temp (locks, pid, built dialogs)
DS    = /usr/share/idiomind          # read-only install dir
DM    = $HOME/.idiomind              # per-user data root
DM_t  = $HOME/.idiomind/topics
DM_tl = $HOME/.idiomind/topics/$tlng       # per target-language topics
DM_tls= $HOME/.idiomind/topics/$tlng/.share       # shared per-language data
DM_tlt= $HOME/.idiomind/topics/$tlng/$tpc        # one topic's data
DC    = $HOME/.config/idiomind
DC_s  = $HOME/.config/idiomind
DC_a  = $HOME/.config/idiomind/addons
DC_d  = $HOME/.config/idiomind/addons/resources/enables   # enabled addon scripts
DC_tlt= $HOME/.config/idiomind/$tpc/.conf                  # topic config dir
DS_a  = /usr/share/idiomind/addons
cfgdb = $DC_s/config                  # sqlite (config, opts, lang, updt)
shrdb = $DM_tls/data/config           # sqlite (shared per-language)
tlngdb= $DM_tls/data/$tlng.db         # sqlite (word translation memory)
tpcdb = $DC_tlt/tpc                   # sqlite (per-topic)
```

## Runtime state files (config side)
- `$DC_s/tpc` — first line = current active topic name (`tpc`)
- `$DC_s/topics_first_run`, `$DC_s/recommended_scripts_first_run`,
  `$DC_s/Resources_first_run` — first-run markers
- `$DT/tpe` — first line = current topic "edit" target (trimmed)

## Topic directory matrix
| Path (under `$DM_tl/$tpc`) | Content |
|---|---|
| `$DM_tlt` | topic root |
| `.conf/` | config dir = `$DC_tlt` |
| `.conf/data` | items (one per line, flat) |
| `.conf/index` | display index (3 lines/item: span, checkbox flag, translation) |
| `.conf/stts` | learning state (see 06) |
| `.conf/stts.bk` | backup of state (used when stts=13 corrupted) |
| `.conf/tpc` | sqlite db per topic |
| `.conf/practice/` | logs or DB per mode (log1..log3 exist for intro) |
| `.conf/note` | topic info file |
| `.conf/att.html` | rendered attachments HTML |
| `.conf/translations/` | `.tra` per translation + `active` + `.bk` |
| `$DM_tlt/images/` | per-topic images (`<word>.jpg`) |
| `$DM_tls/images/` | shared word images (`<word>-1.jpg`) |
| `$DM_tls/audio/` | shared word audio (`<word>.mp3`) |
| `$DM_tlt/files/` | attachments (mp3/ogg/mp4/m4v/jpg/png/txt/gif/`.url`) |
| `~/.idiomind/backup/<topic>.bk` | flat-file revisioned backup |

## Global config DB (`$DC_s/config`)
Tables (from `cnfg.sh`/`mngr.sh`/`mkdb.sh`):
- `opts` — ui + engine switches: gramr, trans, dlaud, ttrgt, itray, swind,
  stsks, intrf, synth, txaud, tlang, level, slang
- `lang` — tlng (learning language), slng (native language) names
- `updt` — last update date, ignore flag
- `config` — misc (yadversion check, etc.)

## Addons
- Installed under `$DS_a` (system) mirroring `Dev/addons/`.
- Enabled/listed as `.ini`-like files in `$DC_d`, named
  `<Name>.<Category>.<Lang>` (e.g.
  `Google translate.Traslator online.Translate.various`).
- Per-category enable lists also exist in `mods/Resources/` UI
  (`Resources_first_run`, `recommended_scripts_first_run`).
- `update_addons()` regenerates `/usr/share/idiomind/addons/menu_list`.