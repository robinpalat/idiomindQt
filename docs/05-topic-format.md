# 05 - `.idmnd` topic format (legacy) and import

## Structure — exactly 3 lines (validated by `check_format_1` in tls.sh)
```text
{"items":{
<items object as JSON>                                          <- line 2
{"name":...,"slng":...,...}                                     <- line 3 (info)
```
Line 3 element order (index n => field, per `tsets`):
0 name · 1 slng · 2 tlng · 3 autr · 4 cntt · 5 ctgy · 6 ilnk ·
7 orig · 8 dtec · 9 dteu · 10 dtei · 11 nwrd · 12 nsnt · 13 nimg ·
14 naud · 15 nsze · 16 levl · 17 info · 18 stts · 19 discarded —

Validation (import in `main.sh` / `tls.sh check_format_1`):
1. `python3 -m json.tool` must succeed (well-formed JSON).
2. `wc -l` must equal 3.
3. line 1 after `tr -d '"{'`, first token must equal `items`.
4. Per-field checks (n indexes): autr/cntt <= 30 chars & no `\* / $ ) ( =`;
   ctgy in `Categories[@]` (underscores→spaces); ilnk <= 36 non-blank;
   orig <= 60 chars no `\* / @ $ =`; dates `^[0-9]{4}-[0-9]{2}-[0-9]{2}$`
   or empty; nwrd/nsnt/nimg numeric <= 200; naud <= 1000; nsze len <= 6;
   levl numeric <= 2 digits; info <= 10240; stts <= 40 chars, no `\* / @ $ = -`.
   name <= 60 chars, no `\* / @ $ =`.
5. slng must be a key of `slangs[]`, tlng of `tlangs[]`; slng may carry
   appended extra translations `, <alt1>, <alt2>...` (tokens saved to
   `otranslations` and later mapped back onto items via `slxx` fields).

## Item JSON (line 2)
```json
"<trgt>": {
  "srce":"...",
  "slch":"","slde":"","slen":"","sles":"","slfr":"","slit":"",
  "slja":"","slpt":"","slru":"","slvi":"",
  "exmp":"","defn":"...","note":"","wrds":"...","grmr":"",
  "tags":"","mark":"","refr":"","imag":"","link":"","cdid":"","type":"1"
}
```
Target string is the JSON object key. `slxx` = alternative native-language
translations (zh, de, en, es, fr, it, ja, pt, ru, vi...). Import maps them into
the internal `srce`/`slxx` fields.

## Real fixture example
From `Web/idiomind.com/public/english/tech/Google Announces New Services and
Products.idmnd` (3 lines):
```
{"items":{
"Last week, Google held a conference where the company announced what it has planned for the coming months":{"srce":"La semana pasada, Google llevó a cabo una conferencia en la que la compañía anunció lo que se ha previsto para los próximos meses","slch":"","slde":"",...},"...more items...":{...}}
{"name":"Google Announces New Services and Products","slng":"Español","tlng":"English","autr":"nguyen","cntt":"","ctgy":"tech","ilnk":"google_announceb3a485ecb7d8efef929b","orig":"Google Announces New Services and Products","dtec":"2016-05-26","dteu":"2016-06-24","dtei":"","nwrd":"5","nsnt":"77","nimg":"0","naud":"359","nsze":"8,6M","levl":"1","info":" ","stts":"0"}
```
Notes: `nsze` "8,6M" uses comma decimal (Spanish locale) and is only
length-checked (<=6). `dtei` often empty. `stts` exported as string.

## Post-import
- Items > 200 are truncated (`head -n200`).
- `tpc_db 6 learning` rebuilds the learning list; `check_index` runs to repair
  tables; stts normalized (`! number → 13`).
- Import path: `chng.sh`/`cnfg.sh` pick file → `main.sh` import → validate →
  create `$DM_tlt/$name` + `.conf` → `check_index`.

## Export (for round-trip)
`mods/export/` produces: Audio (zip), CSV, idmnd (-.idmnd), PDF, TSV.
`.idmnd` exporter wraps stat+Data into the 3-line JSON (fields order above).