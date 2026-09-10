# 07 - External services & addons framework

The original runs every network/dictionary/TTS/image service through **addon
shell scripts** stored in `$DS_a/Resources/scripts/`. They declare exported
variables and are executed with fixed positional args. The reimplementation
must expose these behind clean interfaces (`ITranslationProvider`,
`IPronunciationProvider`, `IImageProvider`) but preserve the *behavior* and
priority ordering.

## Addon script contract
- Filename: `<Name>.<Category>.<LangOrSub>` where category strings include:
  - `Traslator online.Translate` (translation)
  - `TTS online.Convert text to audio`
  - `TTS online.Download audio.<lgt>`
  - `TTS offline.Convert text to audio`
  - `TTS online.Download audio.various`
  - `Script.Download image`
  - `Link.Search definition` / `Link.Search translation`
  - `Audio` / `Video` related (via tls.sh)
- Declared variables: `URL`, `EX`, `EXECUT`, `INFO`, `STATUS`, `VOICES`,
  `CONF`, `FILECONF`, `TESTWORD`, `TLANGS`, `LANGUAGES`, `useragent`.
- Enabled list: symlinks/copies in `$DC_d` = `~/.config/idiomind/addons/resources/enables`.
- Priority order in code: `TTS online.Download audio.<lgt>` → `TTS online.Convert text to audio` → `TTS offline...` → `TTS online.Download audio.various`.

## Translation
- `translate()` (mods/add/add.sh): single word + `ttrgt != TRUE`
  → consult `tlngdb: select <slng^> from Words where Word='word'`;
  else run first enabled `*.Traslator online.Translate.*`.
- Google Translate script (`Google translate.Traslator online.Translate.various`):
  ```
  link="https://translate.googleapis.com/translate_a/single?client=dict-chrome-ex&sl=$2&tl=$3&dt=t&q=$1"
  ```
  Response parsed for ja/zh-cn/ru vs latin targets with `grep -oP '(?<=trgt=\").*?(?=\",\")'`.
- `tls.sh translate_to`: wraps the whole topic; per-language `.tra` copies;
  builds `words.trad_tmp` (unique words) + `index.trad_tmp`, calls `translate`
  twice (index first, then words) separated by sleep 1; maps results back with
  `paste`; fallback delimiters `~~` → `||` → `:` → `_` when translation line
  counts mismatch. Saves old `data` as `<lang>.bk`; on success writes new
  `data` and sets `translations/active`; touches `slng_err` if active != slang.
- Batch manual editing: `transl_batch` shows one text field per item
  (`--isize` form), writes full `data` back; capitalizes first letter
  (`${srce_mod^}`).

## TTS / pronunciation
- Word TTS: `tts_word` (add.sh): enabled order as above; `dwld1`:
  `source script` then `wget -U useragent -O "$audio_file" $URL`, converts non-mp3
  via `sox -t wav -c 1`. Cache: `$DM_tls/audio/<word>.mp3` (lowercase).
- Sentence TTS: `tts_sentence`.
- Offline: `Festival.TTS` (espeak) uses `espeak` params from `sets.cfg`
  (sAmplitude=100 sSpeed=120 sEncoding=1 sPitch=60 sWordgap=0).
- Playback (`play.sh`): `mplayer` per word/sentence; PID/lock files in `$DT`;
  `bucle` repeats (config `loop`), pause `pause_rep`; `stop.sh` kills by pid
  files; "bucle" playlist file = temp list of audio paths.
- Prefixes rule for CJK (`ja|zh-cn|ru`): audio/list/line handling uses the
  second argument (`${2}`) — see `fetch_audio`, `list_words_2/3`.

## Images
- `img_word`: enabled `*.Script.Download image.*`; target cache
  `$DM_tls/images/<word>-1.jpg` (shared, one per word) or
  `$DM_tlt/images/<word>.jpg` (topic-specific for multi-word). Uses `identify`
  dimensions; if `h*100/w > 80` compress+pad to `400x270` white background;
  final `-thumbnail 405x275^ -extent 400x270 -quality 90%`.
- `set_image_1` (`import`): screen clipping → `img.jpg` + `ico.jpg` 110x90.
- `set_image_2`: convert clipping to `405x275^ → 400x270`.
- Yes/no image per item: stored as `imag` field + image file presence.

## Definition & translation browsing
- `_definition`: first enabled `*.Link.Search definition.*`, script's only
  content is a URL template `_url`; opened in `/usr/lib/idiomind/idiomind-htmlview`.
- `_translation`: fixed URL `https://translate.google.com/?sl=$lgt&tl=$lgs&text=${2}&op=translate`.

## OCR
- "Optical character recognition" button in dlg_form_1/2 launches
  `mods/add_process/ocr.sh` (documented in text-processing).

## Other services in tls.sh
- `check_updates` / `a_check_updates`: `curl https://idiomind.sourceforge.io/doc/checkversion`,
  version compare `sort -V`, dialog to download `.../files/latest/download`,
  ignore flag (`ignr`) in cfgdb `updt`, last-check throttling (30 days / daily).
- `fback`: `xdg-open .../contact.html`; `_help`: help.html.
- `backup`/`restore`: flat `.bk` (see 03).
- `attatchments` / `add_file` / `videourl` / `addFiles`: resource files →
  `att.html` (HTML audio/video/img/iframe).