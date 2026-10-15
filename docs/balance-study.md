# Vroom balance study — approved tuning, 2026-10-14

This study selected the approved tuning now implemented in Vroom 0.2.1.
The historical 0.2.0 core is retained for the reproducible comparison.

## Recommendation and reproduction

Adopt the staggered unlocks, 80% partial overdrive, fixed boss shift delays,
and reward tables below. Keep all 37 purchases and their current prices.
Keep the first perfect win at $440 and the free ability/gear milestones.

The experiment compiles the actual Game.cpp race/physics code. It injects
only availability, partial-pulse strength, boss clutch delay and payouts into
a temporary copy. With hooks disabled, output matches the retained 0.2.0 core exactly.
The shipping 0.2.1 core must also match all candidate output in both retry models.

From the workspace root:

    python3 vroom/tools/balance/run.py --seeds 100

Outputs: vroom/out/balance/baseline.txt, candidate.txt,
candidate-direct-retry.txt and summary.json (including source hashes).
The runner refuses to patch changed source fragments silently.

Sanitizer check:

    python3 vroom/tools/balance/run.py --seeds 10 --sanitize --out /tmp/vroom-balance-sanitized

## Method

Four driving profiles × three purchasing policies × 100 campaign seeds:
1,200 baseline executions and 2,400 candidate executions across two retry
models. Fixed profiles repeat as controls; seeds vary the other two profiles.
These are model executions, not independent human playtests.

- Ideal: 7300-RPM shifts, target launch at green, efficient NOS, gold pulses.
- Good: 6800-RPM shifts, six-tick reaction, staging target 3850 RPM,
  partial pulses, otherwise efficient NOS.
- Variable: shift targets 6800–7500 RPM, reaction 8–14 ticks, staging targets
  3600–4240 RPM, full/partial pulses, modest NOS timing variation and
  occasional fuel burned through shifts.
- One major error: variable profile plus a 7900-RPM second-gear shift or an
  out-of-window first overdrive press, alternating by seed. Before overdrive
  unlocks, the latter cannot be tested.

Every profile uses gold shifts against the first precision-shift boss,
which is intentionally unchanged. Profiles do not cover repeated false
starts, arbitrary input errors or every human NOS strategy.

Purchase policies: best immediate time gain per dollar, cheapest available
(including comfort upgrades), and engine/tires/gearbox priority. The value
policy simulates purchase benefits; none previews the race result to decide
whether to attempt a rival. After a loss, the main retry model farms the last
beaten rival once. The second keeps attempting the current rival and spends
its finish payouts.

## Results

| Driving profile | Current completed / 300 | Candidate completed / 300 | Candidate races, with farming after losses |
| --- | ---: | ---: | ---: |
| Ideal | 300 | 300 | 36–38 |
| Good, partial boosts | 0 | 300 | 38–40 |
| Variable timing | 77 | 300 | 36–38 |
| Variable plus one major error | 0 | 300 | 38–40 |

Unfinished baseline runs hit the 180-race limit, mostly at boss 24. This means
those fixed profiles are blocked, not that humans would keep repeating them
without learning.

Both candidate retry models completed 1,200/1,200 runs. Direct retries took
36–42 races. Observed farming gaps between purchases were at most one win;
the acceptance assertion allows two. Shop visits bought at most three items,
versus nine in the baseline, and usually one or two.

Campaigns bought 33–37 upgrades. Ideal drivers can skip some launch-tolerance
purchases and retain more cash; the value policy does not force purchases
that cannot improve its profile.

A first candidate made rivals 14 and 20 unwinnable for the variable driver
by delaying engine levels too far. The final schedule releases them ahead
of those power rivals. All 30 regular rivals were then beatable with available
stage equipment in all four profiles across 200 seeds each.

An earlier reward candidate left the good/cheapest driver short of engine 7,
requiring seven direct losses at the final boss to fund it. Moving first-win
income into stage 6 reduced that case to one final-boss loss or one farming
win. A blanket payout reduction would make the ending grindier.

## Exact candidate: upgrade unlocks

Entries are completed campaign victories required for each successive level.
Zero means available at the start. Existing owned levels remain owned even
when a save exceeds a new purchase gate.

| Path | Wins required for levels 1 onward |
| --- | --- |
| Engine | 0, 2, 7, 13, 19, 26, 31 |
| Gearbox | 0, 10, 23, 32 |
| Tires | 0, 4, 9, 15, 25, 31 |
| Launch | 6, 16, 29, 32 |
| NOS capacity | 12, 21, 24, 27, 30 |
| NOS pressure | 14, 22, 28, 33 |
| Overdrive power | 18, 27, 33, 35 |
| Overdrive charges | 20, 28, 34 |

Three starter choices offer one initial level each. Thereafter at most two
new levels unlock at a victory, with gaps no longer than two victories.
Every stage's full equipment cap is available before its boss.
The garage shows the next level's exact required win.
Ability and gear awards stay unchanged.

## Exact candidate: driving and bosses

Partial overdrive becomes 80% of full strength, up from 60%. Timing windows,
charges, misses, NOS and player shifting stay unchanged.

Add fixed clutch interruption to bosses after each successful shift:

| Boss | Added interruption per shift | Current ideal/max-car lead | Candidate ideal/max-car lead |
| --- | ---: | ---: | ---: |
| 6 | 0 ticks | 0.083 s | 0.083 s |
| 12 | 0 ticks | 0.267 s | 0.267 s |
| 18 | 2 ticks (33 ms) | 0.283 s | 0.350 s |
| 24 | 8 ticks (133 ms) | 0.150 s | 0.483 s |
| 30 | 10 ticks (167 ms) | 0.150 s | 0.667 s |
| 36 | 0 ticks | 0.900 s | 0.900 s |

There is no rubber-banding or attempt-dependent adjustment. Boss 36 already
has a larger equipment disadvantage. Stronger partial pulses give its good
driver 0.483 seconds of headroom instead of 0.383 seconds without slowing
the rival further.

Across 1,000 fixed-cap seeds, variable timing won every trial against bosses
18, 24, 30 and 36. The major-error profile won 1000/1000, 999/1000,
1000/1000 and 999/1000 respectively. Rare ties/losses remain possible when
errors combine; a mistake is not guaranteed survivable in every situation.

Ability removal checks: weak launch still loses boss 12; weak launch or
wasted NOS can lose boss 18; removing NOS or overdrive loses the final boss,
even with other inputs ideal. Perfect driving with full upgrades can now
beat bosses 24/30 without overdrive. This deliberately lets excellent play
elsewhere and maximum equipment compensate for skipping boosts, rather than
demanding every system together. Overdrive remains worth about 0.3–1.1 seconds
in those fixed-build comparisons.

## Exact candidate: money

Position within each stage is 0–5.

| Stage | Win base (also replay wins) | First-victory bonus |
| --- | ---: | ---: |
| 1 | $160 | $220 |
| 2 | $190 | $190 |
| 3 | $280 | $250 |
| 4 | $405 | $310 |
| 5 | $665 | $670 |
| 6 | $945 | $1,375 |

Add $10 × position to every win's base. Keep existing skill money:
$30/perfect shift, $12/good shift, $25/successful pulse, and $60/$25 for
a perfect/good manual launch. The first-victory bonus is awarded once.
First perfect stock win: $160 + $220 + $60 = $440.

Loss finish base: $70 + $25 × (stage − 1). Add skill money capped at $100.
Stage-6 losses therefore pay at most $295. DNFs pay zero. No entry fees.

| Stage | Current ideal/max-car first-win income | Candidate income | Cost of new levels |
| --- | ---: | ---: | ---: |
| 1 | $3,165 | $2,790 | $2,250 |
| 2 | $4,965 | $3,330 | $2,850 |
| 3 | $6,405 | $4,410 | $3,900 |
| 4 | $8,145 | $6,000 | $5,200 |
| 5 | $9,615 | $9,930 | $9,650 |
| 6 | $11,090 | $16,055 | $14,050 |

This benchmark uses each stage's full cap for all six races to isolate
reward/catalog totals. Actual campaign income varies with purchases and
timing. It is not guaranteed payout or a starting bank.

The first four stages' benchmark surplus falls from $8,480 to $2,330:
a 73% reduction. Whole-campaign income falls only about 2%, from $43,385 to
$42,515, because later rewards now support the expensive final levels.
The full catalog still costs $37,900.

Finishing with cash is expected: the championship itself awards about
$2,710 for an ideal run, and no further purchases are required. The important
measure is money waiting behind locked purchases during the campaign.

## Acceptance checks and limits

The runner asserts: first win $440; all three starter choices affordable;
37 purchases costing $37,900; monotone gates; at most two new levels per win;
every stage cap available before its boss; loss payout at most $295; every
candidate campaign completed; farming gaps no greater than two; purchase
batches no greater than three. Disabled-hook output must match original-core
output exactly.

The tuning is implemented for human playtesting; these simulations are not
a measured human win rate or proof of enjoyment. Existing cash and purchased
levels are preserved, with no save schema change.
