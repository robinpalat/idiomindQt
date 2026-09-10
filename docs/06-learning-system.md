# 06 - Learning engine

## State file `$DC_tlt/stts`
Single integer on one line. Meaning (used across mngr.sh / main.sh / items_list.sh):

| stts | Meaning |
|---|---|
| 0 | paused / disabled |
| 1 | learning (active) |
| 2 | mastered |
| 3 | waiting (review due) |
| 4 | waiting (review due, alternate) |
| 5 | mid-review / "Finalize review" (readd topics) |
| 6 | mid-review (alternate) |
| 7 | ready for review (after 100% first interval) |
| 8 | ready for review (alternate) |
| 9 | ready for review (after 150%) |
| 10 | ready for review (alternate) |
| 13 | corrupted / addon topic without valid stts (auto-repair → 1) |

## Review schedule
`notice[] = ( '0' '4' '7' '7' '10' '15' '15' '20' '30' '60' )` — days between
reviews indexed by `count_date_reviews`.

- `count_date_reviews` = number of non-empty `reviews.date1..date10`.
- `>= 9` filled dates ⇒ mastered.
- `days_to_review_porcent = 100 * TM / notice[count]` where `TM` =
  days between "today" and the last non-empty review date.
- Listings:
  - even stts (4,8,10): `>=150% && stts==8 → 10`; `>=100% && stts<8 → 8`;
    `stts==8 → T3`; `stts==10 → T4`.
  - odd stts (3,7,9): `>=150% && stts==7 → 9`; `>=100% && stts<7 → 7`;
    `stts==7 → T3`; `stts==9 → T4`.
  - `stts==2` → T10 (mastered with unseen changes).
  - stts 5/6 older than 20 days → T7 ("readd").
- The T1..T4, T7, T10 lists live in `shrdb` (`$DM_tls/data/config`).

## Marking (answered) — `mark_as_learned` (mngr.sh)
Only runs when `stts ∈ {1,5,6}`.
- `count == 0` → set `reviews.date1 = today`; `stts=3`
- `count == 3` → `stts+1`
- `count > 8` → `date9 = today`; `stts=2` (mastered)
- else → `count++`, write `dateN = today`; `stts = even?4:3`
- `count == 8` → `mast=TRUE`
- After mastering: item leaves `learning`, goes to `learnt`.

## Reset — `mark_to_learn` (mngr.sh)
- `stts = even?6:5` ("finalize review")
- clears `learning`/`learnt`
- re-inserts all items into `learning`
- `repass = count_date_reviews` (config) — re-practice all.

## Index rebuild on state change
`colorize` regenerates `$DC_tlt/index` from `learning`, `marks`, `log1..log3`
(see 03). For stts 5/6, mngr builds the index inline (mixed colors) while the
"Finalize review" banner shows with the "review" action.

## Topic lifecycle (main.sh)
- `new_session`: loads c.conf, checks internet lazily, calls
  `items_list.sh notebook_*`, launches tray, updates `mkmn`.
- `readd`/`calculate_review`: recalculates the topic lists each time a topic is
  opened or the "review" button is pressed.
- `remove/delete topic`: removes `$DM_tlt/$tpc` + `$DC_tlt`.
- `played when listening/audio`: items get queued from `learning`.

## Practice integration (practice/strt.sh)
- 5 modes pull from `Pract1..Pract5` (items reordered).
- `log1` = index/list practice history, `log2`, `log3` = deeper practice +
  failed lists (written per practice session into `$DC_tlt/practice/`).
- `colorize` reads log1/log2/log3 only to color the index (red/orange for
  recently-failed items).