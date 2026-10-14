# Game Concept: Shift King

Incremental drag racing • One-button timing • Upgrades • Progression

> Implementation reference: see section 11, Confirmed CHGame MVP decisions.
> Earlier sections describe the broader concept. The economy update in section 12
> supersedes section 11's original campaign-duration target.


I think drag racing is an excellent fit for an incremental game, particularly because it combines two things that work well together: a repeatable, satisfying skill challenge and a progression system that continuously changes that challenge.

The important design decision is that the game shouldn't simply become easier as the player upgrades their car. Ideally, every few upgrades introduce something new to master, creating a cycle of improving the car, learning its behavior, earning more money, and pushing into faster racing classes.

I'd aim for a game that takes 8–15 hours to complete its primary progression, but remains interesting longer through optimization, prestige, and optional challenges.

## 1. The core gameplay loop

RACE 24

1/4 MILE

+$2,400

SPEED

# 147&#x20;

MPH

GEAR

# 4&#x20;

/ 6

SHIFT

Tap at perfect RPM

NOS READY

Tap for acceleration

Conceptual gameplay interface. The shifting zone moves as RPM rises, while NOS provides a separate timing decision.

Each race lasts somewhere around 5–20 seconds, depending on the vehicle and distance. At the beginning, the player only has to shift correctly. As the game progresses, launches, NOS, traction, and additional gearing create more things to think about.

Race — hit shifts and time boosts

Earn — money based on results and performance

Upgrade — improve speed, acceleration, and control

Challenge — tougher opponents and new classes

The game should be designed so a perfect race against a difficult opponent feels genuinely earned, not predetermined entirely by the player's upgrade level.

## 2. How many levels of progression?

I'd recommend 100 primary races arranged into 10 tiers, with 10 races per tier.

That gives the player a clear, achievable campaign with opportunities for progression even after the final opponent.

## 10 racing tiers

Suggested campaign structure and unlock pacing

| Tier | Theme         | New gameplay mechanic          |
| ---- | ------------- | ------------------------------ |
| 1    | Street Rookie | Basic shifting, 3 gears        |
| 2    | Local Circuit | 4th gear, engine tuning        |
| 3    | Underground   | NOS timing                     |
| 4    | Pro Street    | 5th gear, traction             |
| 5    | Sports Class  | Launch control                 |
| 6    | Supercars     | Turbo spool, heat              |
| 7    | Elite         | 6th gear, advanced NOS         |
| 8    | Hypercars     | Gear ratios, aero              |
| 9    | Outlaws       | 7th gear, extreme power        |
| 10   | Legends       | Championships, ultimate builds |

Each tier would have:

- Races 1–7: Increasingly difficult regular opponents.
- Race 8: A special challenge, such as an unusually fast-launching opponent.
- Race 9: A qualifying race with tougher requirements.
- Race 10: Boss race that unlocks the next tier and a significant upgrade.

A crucial distinction: 100 races doesn't mean 100 individual upgrades. Each race can be replayed for money, records, and mastery bonuses.

You could also introduce longer racing distances at higher tiers (⅛ mile, ¼ mile, ½ mile, full mile), so short acceleration builds and high-speed builds both have value.

## 3. The upgrade system

I'd divide upgrades into four types because each serves a different purpose.

[Sibling rivalry: Nitto's seven-second GT-R | NZ Performance Car](https://images.openai.com/static-rsc-4/Jb1QbFynH1x3-4kcHQxkUxQuCMJHOMXDgboUTPq_Eq0vofwH1DDB7AO4Ki1Sq5ERNRCR0P9nfAhM_88qQy2NVaAcKOcWFliiV4AGeJVWsB47xmJDBTAL3Dff2SWfXpZcQwpAakDQlSCUhXoyI6iwJEQNhE57-eVlYRFnTEEMuKo?purpose=inline)

Power upgrades

Engine, turbo, exhaust, ECU, fuel injection. These make the car accelerate faster, but also push RPM up more quickly and make shift timing harder.

[Quaife 6-Speed 69G Heavy Duty RWD Sequential Gearbox with Load Cell Ge](https://images.openai.com/static-rsc-4/vx7cfISTSuVB221W_KoXqJ6Zad4mfp028gTXHUsHp6iWpWGLWt_XW_nlrfB6CiEuPFJtxhUso-Fd7CUveCESWyG09N-pzhEPk45snt0Q2_TD8dH_e5XPSZAS2RYtn-jF12hxYgQDUf5XfV8w4lzrSyxlXGvEYbSTARJt3pqkZDM?purpose=inline)

Transmission upgrades

Extra gears, faster shifts, clutch, gearbox, customizable ratios. These add both power efficiency and new timing challenges.

["NOS" Nitrous Oxide Systems™ - Nitrous Kits, Gauges, Parts | CARiD](https://images.openai.com/static-rsc-4/oHd9toGE_T8D9cAgdzujHL5hJ5fPuQSCV6-Q7G6fsVvKpJ8CcdPch7WY5zRZifMB-ZAbLHdJsIzdhhJg_VOa6buv20tvAqi6h9BDwT4ahJynoWx5Oovoe_ZShFZlw0yzcfDadS8UySqj8M4Up_KWVfyEqiH5f-QfECiuI5mmvmg?purpose=inline)

Active abilities

NOS, launch control, boost management, temporary overdrive. These provide additional decisions during each race rather than just making the car faster.

[Pin by Wayne Thornton on Drag Racing - then and now | Top fuel dragster, Dragsters, Drag racing cars](https://images.openai.com/static-rsc-4/8zeUePQmSxe_QL55qbeFPJBeXx5g20MelBchANU5BQBBtyEB4gfEWwtQxvGkI_zcXpRdSOpdi7EfB4vHjRev2UiyJ3cbne1083Kw33W9eKCHnw5_w1bR28Uv1Lnin4cxjBgSIZCti48iNfqqJqYPQQ_pneOAbFqbhcjuDLIGkKg?purpose=inline)

Supporting upgrades

Tires, suspension, weight reduction, aerodynamics, cooling. These can make the car easier to control or mitigate disadvantages from high power.

### Recommended upgrade inventory

| Upgrade          | Levels | Gameplay effect                    |
| ---------------- | ------ | ---------------------------------- |
| Engine           | 25     | More power, faster RPM rise        |
| Turbo            | 15     | More high-RPM acceleration         |
| Transmission     | 10     | Quicker shifts, less lost momentum |
| Gear count       | 4      | Adds gears, 3 → 7                  |
| Tires            | 15     | Reduces wheelspin                  |
| Weight reduction | 10     | Improves acceleration              |
| NOS capacity     | 10     | Longer boost duration              |
| NOS pressure     | 10     | Stronger acceleration boost        |
| NOS efficiency   | 10     | More benefit per charge            |
| Launch control   | 8      | Widens optimal launch window       |
| ECU tuning       | 10     | Improves power delivery            |
| Cooling          | 8      | Reduces heat/boost penalties       |
| Aerodynamics     | 10     | Improves high-speed performance    |
| Clutch           | 10     | Reduces shift delay                |

That provides around 155 individual upgrade purchases, spread across 14 upgrade paths. The player won't necessarily max all of them during the main campaign.

For an MVP, I would start with only engine, transmission, gears, tires, and NOS.

The important part is that every upgrade should have a perceivable effect. An engine purchase that makes the next race slightly harder to time is more interesting than an invisible +2% performance boost.

## 4. Make shifting the main skill

The shifting mechanic needs enough depth to stay enjoyable after hundreds of races.

At a basic level, RPM rises automatically and the player taps SHIFT when it reaches the optimal point.

Shift timing prototype

Interactive

Move the RPM indicator to see how shift accuracy affects acceleration.

EARLY SHIFT

Acceleration retained

# 42%

That timing window should evolve throughout the campaign:

- Early races: wide perfect-shift zone and slow RPM rise.
- Middle races: narrower zone, rapidly increasing RPM, and NOS timing.
- Late races: very fast RPM rise, gearing choices, boost management, and traction considerations.

However, never make the perfect-shift window unreasonably small. For a mobile game, I'd target approximately 120–200 milliseconds of achievable perfect-shift timing, with a broader good-shift window. Adjust after playtesting.

The indicator also needs predictable behavior. Players should feel they missed a shift because of their timing, not input latency or misleading animation.

### An important design opportunity: different engines shift differently

An upgrade doesn't necessarily have to make RPM rise faster across the board.

For example, a high-revving engine might produce peak power close to redline, while a large turbo engine produces peak power earlier. Their ideal shift points would be different.

That creates meaningful variety without needing complicated physics.

## 5. NOS should be more than a simple speed button

I'd introduce NOS around tier 3, once players have mastered basic shifting.

NOS should create a timing challenge with several considerations:

- Activating while tires are slipping wastes power.
- Activating near an upcoming shift wastes some boost duration.
- Activating within the engine's optimal power band maximizes acceleration.
- Saving boost for a later section might be advantageous, depending on distance and opponent.

For example, early NOS could be a single 2-second charge. Later upgrades allow the player to split it into two bursts or extend its duration.

The trick is to keep the inputs simple, even when the underlying mechanics become more interesting. Two large buttons for SHIFT and NOS would probably suffice through most of the campaign.

## 6. Economy and progression pacing

An incremental game lives or dies by how rewarding the upgrades feel.

I'd recommend an economy tuned around 4–6 races per meaningful upgrade, with smaller upgrades appearing more frequently.

## Illustrative campaign pacing

Design targets, not measured player data

| Stage     | Target time | Feel                            |
| --------- | ----------- | ------------------------------- |
| Tier 1    | 10–15 min   | Fast upgrades, frequent unlocks |
| Tiers 2–3 | 30–60 min   | Build investment, NOS unlocked  |
| Tiers 4–6 | 2–4 hours   | Tradeoffs and build choices     |
| Tiers 7–9 | 3–6 hours   | Strategic optimization          |
| Tier 10   | 1–3 hours   | Championship and final upgrades |

The price and payout curves should rise together, but not identically. Players should occasionally encounter a wall that asks them to improve driving, optimize their build, or spend a few races earning upgrades.

For example:

Example upgrade costs versus race earnings

Illustrative balance using arbitrary currency units, rather than finalized economy values.

Average race payoutRepresentative major upgrade cost

$0$900K$1.8M$2.7M$3.6M12345678910

One major recommendation: avoid making players repeatedly run trivial races for tiny rewards.

Three systems can help here:

Performance multipliers: Perfect shifts, clean launches, and no wheelspin award bonus money. This makes skill valuable even when the player can already beat an opponent.

Repeatable race records: Players can improve their previous time for progressively smaller record bonuses, giving them something to optimize beyond merely winning.

Quick progression between races: The garage should be accessible directly from the results screen, with one-tap upgrades and immediate rematches.

## 7. What happens after the first 100 races?

This is where the design becomes more distinctly incremental.

I'd implement a prestige mechanic called Rebuild or New Season.

After winning the championship, the player can restart campaign progression in exchange for permanent benefits.

Potential rewards include increased earnings, access to exclusive components, more sophisticated tuning options, or unusual engine types.

I would avoid a flat prestige multiplier that makes all future races meaningless. Instead, offer a mixture of economic bonuses and new playstyles.

For example, a player could choose a permanent specialty:

Power Build

Higher peak performance, narrower shift timing windows.

Precision Build

Better shift tolerance, slightly lower peak power.

NOS Build

Short, extremely powerful acceleration bursts.

Prize Build

Greater earnings, but less competitive power.

I wouldn't force the player to replay exactly the same 100 races every season. Higher prestige levels should add modifiers, unlock new opponents, and accelerate the early stages substantially.

For example, the first ten races might take 15 minutes during the first season, but only two minutes during the fourth.

## 8. Avoid the most common incremental-game problem

There is an inherent conflict between skill-driven racing and incremental progression.

In a traditional incremental game, players eventually want automation. In a skill game, automation removes the central challenge.

I would resolve this with a distinction:

Active racing is always the fastest way to progress, but passive income is available later.

For instance, winning a championship unlocks a team garage that can automatically earn modest income through exhibition races. That income pays for upgrades but cannot beat bosses, set records, or earn competitive performance bonuses.

This gives players a reason to return without undermining the shifting mechanic.

I'd also recommend some forgiving assistance options—such as a wider good-shift window—so that casual players aren't completely excluded.

## 9. Scope for an initial playable version

I'd deliberately build a much smaller game first.

## MVP scope

First milestone

Implementation checklist

0/8

One fixed car with 3–5 gears

RPM curve and shift timing

One race distance (¼ mile)

10 progressively faster opponents

Race winnings and a simple garage

Engine, gearbox, and tire upgrades

Basic NOS ability

Race results and rematch flow

A 10-race prototype with just five upgrades is enough to determine whether the fundamental mechanic stays satisfying as the car gets faster.

Once that's fun, expand into the remaining tiers. Avoid building dozens of upgrade systems before knowing whether the timing and economy feel right.

## 10. Three decisions I'd settle before implementing

Driving model

Arcade — speed and RPM are tuned for fun

Simulation-lite — torque curves, traction and gearing matter

Controls

One-button — tap to shift, hold for NOS

Two-button — separate shift and boost buttons

Target campaign length

Short — 2–4 hours

Medium — 8–15 hours

Long — 25+ hours

Refine game design&#x20;

My starting design would be simulation-lite physics beneath arcade-style controls. The player doesn't need to understand torque curves or clutch mechanics, but those systems can provide genuinely interesting decisions about shift points, engine upgrades, and boost timing.

Most importantly, I'd optimize for the feeling that every race makes some progress and every upgrade changes how the next race plays. That's what can make this concept more compelling than either a conventional drag racing game or a conventional idle clicker.

## 11. Confirmed CHGame MVP decisions

Confirmed on 2026-10-14. This section is the implementation scope for the
first playable version and takes precedence over earlier suggestions in this
document. The larger campaign remains a future concept.

### Racing model and controls

- One fixed player car, quarter-mile races, and ten progressively faster opponents.
- Simplified vehicle physics implemented with integer or fixed-point maths for
  CHGame. Speed and the current gear ratio determine RPM; engine power, tire
  grip, and NOS determine acceleration.
- Automatic countdown and launch. Launch timing is deferred.
- A shifts up; B activates NOS. START pauses; holding START for three seconds
  returns to the console menu.
- Early shifts drop RPM and reduce acceleration. Late shifts lose time against
  a rev limiter. Shifting briefly interrupts power; gearbox upgrades shorten
  that interruption.
- No engine damage or automatic upshifts. Perfect and Good labels describe
  timing; they do not award artificial speed boosts.
- Tire grip limits usable engine power. Excess power causes visible wheelspin
  and weaker acceleration, particularly at launch or with NOS in low gears.
  Tire upgrades raise the grip limit.
- Each opponent has a fixed car setup and shift schedule, uses the same vehicle
  model, and performs consistently on retries. No rubber-banding.

### Garage and unlocks

Permanent purchases, with each cost and effect shown in the garage:

| Purchase | MVP quantity | Effect or unlock |
| --- | --- | --- |
| Engine | 5 upgrade levels | More power and faster RPM rise under acceleration |
| Gearbox | 3 upgrade levels | Shorter power interruption during shifts |
| Tires | 5 upgrade levels | Higher grip limit and less wasted power |
| Fourth gear | 1 purchase | Available after beating opponent 3 |
| Fifth gear | 1 purchase | Available after beating opponent 6 |
| NOS | 1 purchase | Available after beating opponent 3 |

This is 16 purchases total. The car starts with three gears.

NOS is one uninterrupted two-second boost per race, activated by pressing B.
It refills free for the next race. Boost continues during shifts, so activating
just before a shift wastes some useful duration. NOS upgrades and split bursts
are deferred.

### Economy, progression, and race flow

- Target 30–45 minutes for a fresh player to beat all ten opponents, including
  retries and garage purchases.
- Finishing a loss pays a modest reward. Wins pay more; good shift performance
  adds bonuses. No entry fees.
- Winning unlocks the next opponent. A race list allows replaying any unlocked
  opponent for money and personal best times.
- First wins grant a one-time bonus; repeat wins retain the normal payout.
- Results offer Next Race, Rematch, and Garage as applicable.
- Beating opponent 10 shows a championship screen. Players can then continue
  racing, buy remaining upgrades, and improve their records.

### Presentation

- Night-time street drag strip, with a brightly coloured player car, a
  contrasting opponent, scrolling road markings, and sparse city scenery.
- Side-on race view in the upper portion of the 128×128 display. The player
  car stays visible in the foreground lane, with the camera following it.
  Road and scenery scroll past; the opponent occupies the farther lane and
  moves ahead or behind according to the actual race gap.
- Instrument-style HUD in the lower portion, with a prominent RPM bar,
  clearly marked shift zones, gear, speed, and NOS status. Use readable
  green/gold shift zones and a red limiter.
- Shift timing receives the strongest visual emphasis. Wheelspin has a
  visible cue.

### Saving

- Automatically save money, upgrades, unlocked opponents, first-win bonuses,
  and personal best times after race results and purchases.
- An interrupted race earns nothing and restarts from the garage.
- Include a confirmed Reset progress option.
- No mid-race saves. Use a unique game save magic and fit progression into
  CHGame's small flash-save record.

### Deferred features and tuning

Outside this MVP: launch timing, part selling or swapping, custom gear ratios,
NOS upgrades or split bursts, engine damage, additional cars, additional tiers
or race distances, prestige, and passive income.

Exact physics coefficients, gear ratios, opponent setups, shift-window widths,
upgrade prices, payouts, and bonus amounts are implementation and playtesting
decisions. Tune them to make upgrades perceptible, timing readable on CHGame,
and the ten-opponent progression fit the confirmed 30–45 minute target.

## 12. Approved economy tuning update

Approved on 2026-10-14 after playtesting. Upgrade cadence takes priority over
section 11's original 30–45 minute duration target.

- A perfect first race must win and fund a first upgrade.
- Subsequent progression should avoid requiring more than two replayed races
  between useful upgrade purchases.
- Keep existing race payouts, skill bonuses, first-win bonuses, physics,
  opponents, upgrade effects, unlock gates, and saved progression.
- Apply the following permanent purchase prices:

| Upgrade | Prices by level |
| --- | --- |
| Engine | $350 / $650 / $1,000 / $1,400 / $1,700 |
| Gearbox | $400 / $800 / $1,300 |
| Tires | $300 / $550 / $850 / $1,200 / $1,500 |
| Fourth gear | $900 |
| Fifth gear | $1,400 |
| NOS | $1,000 |

A perfect first win pays $440, allowing a choice of engine, gearbox, or tires.
The ideal-driver, efficient-purchase model reaches the championship in about
16 races with six replays, a maximum two-replay stretch between upgrades, and
7.6 modeled minutes including countdowns and an eight-second menu allowance
per race. These are modeled results, not guarantees for every purchase order
or measurements of human completion time.
