# Vroom 0.2.1 development notes

The current implementation follows sections 15–17 of [plan.md](plan.md).
This release has 36 rivals, six stages, one player car, four active driving
systems, and 37 permanent purchases across eight upgrade paths.
[MVP development history](docs/mvp-development-notes.md) is archived separately.

## Structure

- `Game.h/.cpp`: portable fixed-tick physics, abilities, rivals, rewards and migration.
- `Pilot.h`: deterministic host/debug driver; not part of release player controls.
- `App.h/.cpp`: menus, real inputs, stage rewards, sound and saves.
- `View.h/.cpp`: race, launch and ability instruments; scenery.
- `Appearance.h/.cpp`, `CarArt.h/.cpp`: 100 car recipes and shared geometry.
- `Sounds.h/.cpp`: sound effects and their priorities.
- `tools/tests/test_vroom.cpp`: mechanics, migration, bosses and campaign models.
- `tools/tests/test_appearance.cpp`, `test_visuals.py`: identities and skyline seams.
- `tools/scripts/`: deterministic UI, abilities, milestones, migration and preview.
- `tools/tests/check_firmware.py`: optional compiled-RISC-V emulator integration.

The game stays beside the upstream checkout. No platform or shared library
sources are modified.

## Driving and campaign

Logic runs at 60 Hz with bounded catch-up and DMA-safe drawing.
Speed is mm/s and distance is mm. Power, drag, gear ratios, grip and power
interruptions drive acceleration; there is no synthetic shift-speed bonus.

The opening 12 rivals use eighth-mile lessons. Later races contain 17
quarter-mile and five half-mile challenges, making quarter-mile the most
common format overall. Each stage has six rivals and ends in a boss.
Fixed opponent builds and strategies specialize in launch, power, sprinting,
late boosting, balanced driving, or stage mastery. Their specialty is shown
in the rival list. No rubber-banding is used.

Wins 6, 12 and 18 award manual launch, NOS and overdrive for free. Additional
gears arrive at wins 6, 12, 18 and 24 (three through seven). Win 30 opens final
upgrade levels; win 36 is the championship.

- Manual launch: the A press selecting a race must first be released. Hold
  A at countdown 1 to build from 1800 toward 4200 RPM (40 RPM per tick),
  and release at green. The clock starts at green; reaction delay counts.
  Early release adds 30 ticks. RPM error reduces launch efficiency for the
  first 60 running ticks; launch upgrades widen tolerance.
- NOS: hold B, with no spending before launch or while paused. Fuel burns
  through shifts even while power is interrupted. Capacity levels supply
  120/156/204/252/312/420 ticks (2–7 seconds). Pressure adds power, still
  subject to tire grip.
- Overdrive: good/perfect shifts arm a 24-tick window after the clutch
  interruption. Remaining-window ticks 10–18 give full strength; the rest
  of the window gives 80 percent. Pressing outside it spends a charge for
  no pulse. Active-pulse repeats are ignored. Strength adds 50–110 percent
  requested force for 42–66 ticks, subject to grip. Charges range from one
  to four, refilled each race.
- Pause freezes race time, staging, fuel, pulse/window timers and motion.
  Holding START still returns to the console menu through CHGame.

Upgrade limits: engine 7, gearbox 4, tires 6, launch 4, NOS capacity 5,
NOS pressure 4, overdrive power 4, overdrive charges 3. Victory gates spread levels throughout each stage, with every stage cap
available before its boss. The garage shows the next level’s required win. A perfect first win pays $440.
Loss finishes pay modestly; DNFs pay nothing. Ties are losses.

## Save schema and migration

Save magic/debug ID is `VRM1`; configuration prefix is `VRM_`. The release
version is 0.2.1 and save schema version is 2. `Progress` is 112 bytes,
below the 244-byte flash-record limit. It stores cash, 36 best times,
eight upgrade levels, sequential win/unlock state, and archived MVP data.
Every record is validated. Change the schema version when changing layout.

The loader reads a valid v2 record first, otherwise attempts v1 migration.
The original v1 ABI is explicitly represented by the 36-byte
`LegacyProgress` struct. Migration preserves:

- Cash and engine, gearbox and tire levels, including previously owned levels
  above a new stage's purchase gate.
- The count of completed rivals (up to ten), continuing into the expanded list.
- Ten old quarter-mile bests, archived in the driver guide rather than compared
  against new distances.

Removed purchases receive current-catalog credits: fourth gear $900, fifth
gear $1400, and old NOS $1000. Abilities and extra gears now follow milestones.
An import screen explains the change and remains until acknowledged.
Archived records remain accessible even when the old save had no wins.

Writes happen only after `gfx_wait()`. Failed saves are visibly reported and
retried on the next result/purchase, rather than writing every frame.
CHGame's two save pages are shared with other games, as on the platform.
Reset Progress intentionally clears the campaign and archived records.

## Commands

From the workspace root:

```bash
python3 vroom/tools/vroom.py check
python3 vroom/tools/vroom.py gif
python3 vroom/tools/vroom.py export --no-build
python3 vroom/tools/vroom.py cart verify vroom/build/vroom.chgame
python3 tools/run_emulator.py vroom/build/release/vroom.ino.bin
```

`check` includes a release compile; `check --no-device` skips that compile.
`build --debug` checks the full protocol build. Outputs are under
`vroom/build/` and `vroom/out/`. The wrapper locates the bundled Arduino CLI.
Use the repository's preferred Zig compiler for the host simulator; the
local GCC chrono headers conflict with the upstream Arduino min/max macros.

Optional compiled-firmware verification, using the existing workspace emulator,
a C++ compiler and nm:

```bash
python3 vroom/tools/tests/check_firmware.py
```

It creates isolated ROM/save copies under `out/firmware/`, schedules real
button events, checks guest state, restarts with saved flash, and imports a
real CRC-valid v1 fixture. No connected device or user save file is accessed.

## Debug commands

Available only in simulator/debug builds:

- `V`: state, resources, timing, stage, rewards and archived-save status.
- `O 0/1/2`: manual / ideal pilot / good-shift pilot with partial pulse timing.
- `E [0..35]`: seed a rival with its currently available upgrade levels (default final rival).
- `Z`: reset progress; `U`: reload a valid saved record.
- `J [0..10]`: write v1 test records and exercise real migration (default ten wins).
- `I <0..99>`: appearance preview; `C <mm>`: scenery continuity probe.

E, Z and J deliberately modify test progress. Automated checks use isolated
simulator processes or emulator files. Release player controls contain no pilot
or seed commands.

## Archived 0.2.0 verification recorded 2026-10-14

- Game suite: 957 checks passed with Clang AddressSanitizer/UBSan.
- Appearance/world-tile suite: 1003 checks passed.
- Pixel suite: 132 checks, including 100 distinct car renders and 32 skyline seams.
- Six simulator scenarios passed twice with identical image frames.
- Campaign models: all 36 rivals cleared with three purchase strategies and a
  good-shift/full-pulse driver. Runs used 36–37 races, 32–33 purchases, and
  zero or one replay, about 15 modeled minutes with countdown/menu allowances.
- Boss checks demonstrate launch, NOS and overdrive matter at their introduction.
  Half-mile model runs retain meaningful actions within six seconds of finishing.
- First-race checks cover representative perfect-shift points and immediate
  affordability of each starter path.
- Compiled release won races across all six stages with scheduled physical
  button inputs, including a fresh first-race $440 win. Saves survived restarts;
  CRC-valid v1 migration and v2 reloading passed.
- Emulator measurements were approximately 57.5–58.5 LCD frame starts/second
  during those runs. This is emulator evidence, not a physical timing measurement.
- Release image: 27,876 bytes; static RAM: 12,180 bytes; both save pages free.
  Debug image: 33,948 bytes; static RAM: 12,380 bytes; both save pages free.

Remaining work is human playtesting of launch feel, pulse readability,
opponent variety, and upgrade cadence across varied choices. Early lessons
are intentionally simple, and the 36-rival count remains a playtest hypothesis.
Physical console audio, display and power-cycle persistence also need checking.
No console upload was performed.


## 0.2.1 balance update

[Balance study](docs/balance-study.md) records the exact approved victory
gates, reward tables and boss hesitation. Prices, ability milestones, save
version/layout, existing cash and purchased levels are preserved. Partial
overdrive is 80%; bosses 18/24/30 add 2/8/10 ticks of clutch interruption.
Win bases are 160/190/280/405/665/945 plus 10 per within-stage position.
First bonuses are 220/190/250/310/670/1375. Losses pay 70 plus 25 per
zero-based stage and at most 100 skill money. First perfect win remains 440.

The balance runner now retains the historical 0.2.0 core separately and
compares shipping Game.cpp against the approved candidate in both retry
models. It must match the candidate's complete output exactly.


## 0.2.1 verification recorded 2026-10-14

- 3,236 game checks passed, including Clang AddressSanitizer/UBSan.
- 1,003 appearance/world checks and 132 pixel checks passed.
- Seven UI scenarios passed twice with identical frames; upgrades.txt checks
  the locked shop, a level opening after win 2, and final-stage gating.
- The production core exactly matched the approved candidate across 2,400
  campaign runs (four drivers, three purchase policies, two retry models).
- Release emulator input checks won across all six stages. Save reloads
  and v1 migration passed. Existing v2 cash/owned levels remain valid.
- Release image: 28,136 bytes; static RAM: 12,188 bytes; both save pages free.
- Version 0.2.1 cartridge exported and verified.


## Browser site — 2026-10-15

The self-contained static bundle is in site/. It automatically boots the
release Vroom ROM in the unmodified upstream CHGame browser runtime.
See docs/web-site.md for preview, updates, deployment and provenance.

- Bundled 0.2.1 ROM matches build/release/vroom.ino.bin byte for byte.
- Python bundle validation checks ROM/version and matching JS/Wasm hashes.
- Browser tests passed at a project subpath, including real race completion,
  IndexedDB restore after reload, held START, overlapping controls, blur
  release, fullscreen, mobile touch and missing-ROM failure handling.
- Screenshot pixel checks verify actual Vroom colors, beyond CPU/save activity.
  Nearest-pixel integer scaling avoids a blank render observed with the
  upstream pixel-art scaling mode at larger viewport sizes.
- The firmware's LCD initialization takes roughly 0.7 seconds; the shell hides
  uninitialized display RAM and holds inputs until its brief startup completes.
- Root and standalone-game Pages workflow copies match and validate the
  committed bundle without a firmware toolchain. Both layouts checked locally.
- No remote GitHub repository or Pages deployment was created.
