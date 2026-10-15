# Historical MVP development notes

This snapshot predates Vroom 0.2.0. See [the current notes](../NOTES.md).

# Vroom development notes

## Scope

The confirmed reference is sections 11–13 of [plan.md](../plan.md). This is a
CHGame Rev0 sketch kept beside the upstream checkout, not added to its
casino collection. No upstream library or platform files are modified.

## Structure

- `vroom.ino`: setup, fixed 60 Hz logic with bounded catch-up, DMA-safe drawing.
- `Game.h/.cpp`: integer vehicle physics, opponents, upgrades, payouts.
- `App.h/.cpp`: menus, input, sound, debug hook, saves.
- `Sounds.h/.cpp`: sound step tables and effect priorities.
- `Appearance.h/.cpp`: pure appearance mapping and world-space skyline recipes.
- `CarArt.h/.cpp`: shared native-resolution car geometry and decal renderer.
- `View.h/.cpp`: parallax city, dashboard, and reusable UI drawing.
- `tools/tests/test_vroom.cpp`: host physics, economy, and progression tests.
- `tools/scripts/`: deterministic UI, NOS, saves, and championship checks.

The save handshake and magic are `VRM1`; configuration prefix is `VRM_`.
Save schema version 1 holds cash, wins, unlocks, levels, champion status,
and ten personal best times, within the library's 244-byte payload.
Records are validated at boot; invalid or incompatible records start fresh.
Flash writes occur only between `gfx_wait()` and the next flush.
Save failure is shown on menu screens; the next result or purchase retries.
CHGame's save pages are shared between games, so another game's saves can
replace Vroom's progress, as on the rest of the platform.

## Commands

Run from the workspace root:

```bash
python3 vroom/tools/vroom.py build
python3 vroom/tools/vroom.py test
python3 vroom/tools/vroom.py check --no-device
python3 vroom/tools/vroom.py gif
python3 vroom/tools/vroom.py boxart
python3 vroom/tools/vroom.py audio out/audio
python3 vroom/tools/vroom.py export --no-build
python3 vroom/tools/vroom.py cart verify vroom/build/vroom.chgame
```

`build` produces `vroom/build/release/vroom.ino.bin`, ELF, HEX, and CHG.
`export` produces `vroom/build/vroom.chgame` for sharing or SD-menu deployment.
The wrapper locates the workspace's bundled arduino-cli if it is not on PATH.
Simulator builds use the repository's preferred Zig compiler. On this
machine GCC 16's chrono headers conflict with Arduino min/max macros in
the upstream simulator; use Zig rather than modifying shared libraries.

## Physics and balance

Speed is mm/s, distance is mm, and races advance at 60 fixed ticks/second.
Gear ratios connect speed to RPM. A broad power curve peaks at 6000 RPM;
the gold shift window is 7000–7600 RPM around a 7300 target. Torque is
limited by tire grip. The first gear's clutch launch maintains 2500 RPM.
Shifts interrupt power for 400–150 ms depending on gearbox level.
A limiter cuts power at 8000 RPM; road drag continues during shifts.
NOS increases requested drive force 45 percent for 120 ticks and remains
subject to traction limits.

Rivals use the same physics and fixed builds/shift thresholds. They finish
in about 23.1 down to 15.1 seconds. An ideal fully upgraded player runs
about 14.9 seconds. Exact ties count as losses. The race ends when the
player finishes; an unfinished rival is shown as BEHIND rather than
inventing an elapsed time.

The campaign test uses ideal shifts and a greedy affordable-upgrade policy,
farming the latest beaten rival when blocked. With the approved price curve,
it completes in 16 races (six replays), about 7.6 modeled minutes including
three-second countdowns and an assumed eight seconds of menu time per race.
Its longest replay stretch between useful purchases is two. The first perfect
win pays $440 and can buy an engine ($350), gearbox ($400), or tires ($300).
These results describe one efficient purchase policy, not every possible
purchase order. This is a balance model, not human playtest evidence.

## Simulator commands

Commands below exist only in debug/simulator builds:

- `V`: report screen, cash, progression, and current race state.
- `O 1` / `O 0`: automated ideal player shifts and NOS on/off.
- `Z`: reset progress and queue an autosave.
- `E`: prepare an upgraded build and the final rival for system checks.
- `U`: reload the real save record (refuses pending writes or invalid data).
- `I <0..99>`: preview an opponent appearance without altering progress.
- `C <distance>`: draw the scrolling scene at a distance in millimetres.

The release build contains no automation or progression cheats.
The simulator's save pages persist within a process; `U` tests reloads
through the library. Persistence across physical power cycles requires
console verification.

## Hardware verification remaining

No upload has been performed. Real panel readability, frame timing, audio,
physical input feel, and save persistence across power cycles need a
console check. The approved economy update prioritizes upgrade cadence
over the original 30–45 minute campaign target.

## Initial MVP verification recorded 2026-10-14 (before economy retuning)

- Host suite: 360 checks, zero failures, including a naturally funded campaign.
  Clang AddressSanitizer and UndefinedBehaviorSanitizer also passed.
- Three simulator scenarios (gameplay, smoke, systems) passed; all screenshots
  and GIF frames matched across two runs, with no DMA tearing reports.
- Release: 20,352-byte image; 11,948 bytes static RAM; both save pages free.
  Debug: 25,076-byte image; 12,148 bytes static RAM; both save pages free.
- Five sound effects rendered successfully through the platform audio harness.
- Menu cover passed the platform picture validation. Gameplay GIF is 95,383
  bytes, below the one-megabyte limit.
- Exported cartridge passed `cart verify`.
- The compiled release booted and entered racing in the existing RISC-V
  emulator. Its ten-second run reported 55.4 LCD frame starts per second and
  generated audio. The captured frame is `out/firmware-race.png`.
  This is emulator evidence, not a physical hardware timing measurement.

## Approved economy update verification

The price curve in plan section 12 is now implemented. Payouts, physics,
unlock gates, and save schema remain unchanged; existing saves are compatible.

- Host suite: 183 checks, zero failures. It explicitly checks that a perfect
  first win buys each starter upgrade independently and that the modeled
  efficient campaign never exceeds two replays between useful purchases.
- Campaign model: 16 races, six replays, 14 purchases, 454 modeled seconds.
- All three simulator scripts passed twice with identical images, including
  a first-win engine purchase leaving $90 and a successful save reload.
- Updated release: 20,336-byte image, 11,948 bytes static RAM, two save pages.
- Gameplay preview regenerated with the immediate first-win purchase.

## Larger cars and 100 opponent appearances

`Appearance.cpp` permutes IDs 0–99 with `(id * 37 + 11) % 100`, then
decodes four bodies × five paints × five decals. The mapping is bijective,
deterministic, and costs no per-opponent sprite or save storage. The player
has a reserved gold coupe livery. Opponent paints use existing palette entries.
`CarArt.cpp` shares 1/16-pixel polygon recipes, windows, rims, bumpers, and
five side decals at native 61×26 resolution. Wheel animation follows distance.

The street occupies rows 0–83; the dashboard occupies rows 84–127. Far and
near lanes fit both larger cars without covering the RPM instrument.
The rival list now shows the selected car above four scrollable entries.

Skyline buildings are attached to world tiles spaced 24 pixels apart, with
one parallax pixel per 1500 mm of travel. A 16-tile pattern repeats seamlessly;
building heights and windows retain their identities when tiles wrap.
The moon stays fixed. Road markings retain their separate faster scroll.

Generate the native-scale gallery and validate graphics with:
`python3 vroom/tools/tests/test_visuals.py`.
Output is `out/visuals/opponents-100.png`; its documented copy is
`docs/opponents-100.png`. The existing ten opponents remain the only playable
campaign races; these changes do not alter progression or the save schema.

Verification:
- Existing game/economy suite: 183 checks passed, with the same 16-race model.
- Appearance and world-tile suite: 1003 checks passed.
- Pixel tests: 100 distinct car renders and 32 seamless skyline boundaries.
  Comparisons cover rooflines and windows, masking only the stationary moon.
- Four simulator scenarios passed twice with identical captured images.
- Release: 21,672-byte image and 12,020 bytes static RAM, two save pages free.
  The visual update adds 1336 image bytes and 72 static RAM bytes.
- Compiled release entered racing in the RISC-V emulator: 55.4 LCD frame
  starts per second over ten seconds, matching the earlier emulator check.
  Capture: `out/firmware-large-cars.png`. Physical console checks remain.

## Stronger sense of speed

Visual scrolling is now 3× faster for the skyline (1500 mm per pixel)
and 3.5× faster for the lane markings (100 mm per pixel). Markings are
30 pixels long with a 64-pixel period so their faster movement stays
readable. A near verge scrolls at 70 mm per pixel with a 96-pixel period,
creating an additional foreground depth cue. The moon remains stationary.

All scrolling is derived from travelled distance, so launch builds naturally
into speed and pausing freezes scenery. Vehicle physics, RPM, shift windows,
opponent gaps, wheel animation, payouts, and saves are unchanged.
The preview now samples every two frames to show fast motion clearly.

Verification: 183 game checks, 1003 appearance/world-tile checks, and 132
pixel-render checks passed. Four simulator scripts were deterministic on
repeated runs; all 32 skyline seam checks passed at the new scroll scale.
The compiled release measured 55.4 LCD frame starts per second in the same
ten-second emulator check. Release image is 21,752 bytes; static RAM is
12,020 bytes, with both save pages free. Physical console feel remains
a playtest check.
