# Vroom: Shift King

Shift at the right RPM, time your nitrous, and upgrade a midnight street racer.
Beat ten quarter-mile rivals while turning race earnings into a faster car.

![Vroom gameplay](docs/gameplay.gif)

## Controls

| Button | Action |
| --- | --- |
| A | Shift up in a race; select or buy in menus |
| B | Activate NOS once per race; back in menus |
| Up / Down | Navigate menus |
| START | Pause / resume a race |
| SELECT | Switch between garage and rival list |

## Rules

The countdown and launch are automatic. Tap A as the RPM marker enters the
gold zone. Early shifts drop revs; late shifts waste time on the limiter.
Gear changes briefly interrupt engine power. Tire grip limits usable power:
wheelspin means an engine or NOS boost is overpowering the tires.

Start with three gears. Beating rival 3 makes fourth gear and NOS available
to buy; beating rival 6 makes fifth gear available after purchasing fourth.
NOS is a continuous two-second burst, refilled free each race. It keeps
burning through shifts, so use B after a shift when grip and revs suit it.

Finishing earns cash even on a loss. Wins pay more, good/perfect shifts add
bonuses, and each rival has a one-time first-win prize. Winning unlocks the
next rival; earlier rivals remain available for repeat earnings and records.
An exact tie is a loss. A 60-second race limit handles a failure to finish,
without a payout.

## How to play

Choose Race to meet your next rival, or SELECT in the rival list to open
the garage. Engine upgrades add power, gearbox upgrades shorten shifts,
and tires improve grip. Purchases are permanent. Cash, upgrades, wins,
unlocks, and best times save automatically after purchases and results.
An interrupted race earns nothing; next boot starts at the home screen.

Beat Shift King to become champion, then keep racing to improve your car
and records. Reset Progress requires confirmation. Sound can be toggled
on the home screen. Holding START for three seconds returns to the console
menu, including while paused.

Open `vroom.ino` in Arduino IDE with CHGame Rev0, USB **Upload only**, and
Optimize **Smallest + LTO**, or run `python3 vroom/tools/vroom.py upload`
from the workspace root. The packaged cart is `build/vroom.chgame`.

For an interactive desktop session with the emulator already in this workspace:

```bash
python3 tools/run_emulator.py vroom/build/release/vroom.ino.bin
```

Space/X is A, Z is B, arrows navigate, and Enter is START.

## Developer notes

- `Game.cpp` separates fixed-rate vehicle physics and progression from drawing.
- `View.cpp` draws the cars and scrolling street directly with CHGfx primitives.
- `App.cpp` uses CHGame input, sound, and CRC-protected flash saves.
- Host tests and deterministic simulator scripts cover the race and garage loop.
- See [NOTES.md](NOTES.md) for build commands, balance assumptions, and verification.

## Credits

Original game code and code-drawn art created for this project.
CHGame and CHGfx libraries by bateske, Apache-2.0.
CHGame's 3x5 font is by Press Play On Tape; see the library's NOTICE.
