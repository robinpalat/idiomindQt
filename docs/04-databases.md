# 04 - Database schemas

## Per-topic DB `$DC_tlt/tpc` (created by `ifs/mkdb.sh tpc`)

```sql
CREATE TABLE id (
  name, slng, tlng, autr, cntt, ctgy, ilnk, orig,
  dtec, dteu, dtei, nwrd, nsnt, nimg, naud, nsze, levl, stts );
```
Column order mirrors JSON info fields order (index 0..17 => `tsets[]`,
see `sets.cfg`).

```sql
CREATE TABLE config (
  words, sntcs, marks, learn, diffi, rplay, audio, ntosd,
  loop, rword, acheck, repass );
```
Defaults row:
`'TRUE','TRUE','FALSE','FALSE','FALSE','FALSE','FALSE','FALSE','FALSE','FALSE','TRUE','0'`
(all single-row).

```sql
CREATE TABLE reviews ( date1, date2, ..., date10 );
```
Single row; `dateN` = `%m/%d/%Y` strings; empty until first learning pass.

```sql
CREATE TABLE learning ( list );
CREATE TABLE learnt   ( list );
CREATE TABLE words    ( list );
CREATE TABLE sentences( list );
CREATE TABLE marks    ( list );
```
Each `list` holds one item `trgt` per row.

```sql
CREATE TABLE Data (trgt, srce, exmp, defn, note, wrds, grmr, tags, mark, refr, imag, link, cdid, type);
```
Used by dialog editing (read from `data` file into a form).

```sql
CREATE TABLE Pract1 ( items_0 );   -- items in order for practice
-- Pract2..Pract5 similar (per-mode), plus Practice_stats view logs
```

`check_index` verifies:
- exactly 1 row in `reviews`, `id`, `config`;
- `COUNT(words)+COUNT(sentences) == item lines`;
- `COUNT(learning)+COUNT(learnt) == item lines`;
- index lines/3 == learning count;
- else rebuilds via python `_restore` (drop tables, re-insert from `data`,
  honoring `type`/`mark`, and `s` flag → learnt vs learning).

## Shared per-language DB `$DM_tls/data/config` (`shrdb`)
Tables `topics` and the review lists `T1..T4`, `T7`, `T10` (see 06). Each row
holds topic name + computed values. `mngr.sh` refreshes these lists on
calculate_review.

## Translation memory `$DM_tls/data/$tlng.db` (`tlngdb`)
```sql
CREATE TABLE Words ( Word TEXT PRIMARY KEY, <slng> TEXT );
```
Column name == the native language name (e.g. `Spanish`). Used to auto-skip
network translation when the word was translated before (`translate()` in
`mods/add/add.sh`).

## Global config `$DC_s/config` (`cfgdb`)
- `opts` (single row): gramr, trans, dlaud, ttrgt, itray, swind, stsks,
  intrf, synth, txaud, tlang, level, slang
- `lang` (single row): tlng, slng  (language display names)
- `updt`: date, ignr
- `config` (single row): version, yadversion check markers

Helper `cdb ${cfgdb} <read|write> <table> <field> [value]` (> in cmns.sh).