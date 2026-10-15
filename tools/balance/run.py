#!/usr/bin/env python3
"""Reproduce the Vroom 0.2 balance study without modifying firmware sources."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
DRIVER = Path(__file__).with_name("driver.cpp")
BASELINE = Path(__file__).with_name("baseline-0.2.cpp")


def replace_once(source: str, before: str, after: str) -> str:
    if source.count(before) != 1:
        raise RuntimeError(f"Game.cpp changed; review the experiment patch: {before}")
    return source.replace(before, after, 1)


def instrument(source: str) -> str:
    replacements = [
        ("namespace vr {",
         "namespace balance {extern int partial,delay[6]; "
         "uint8_t available(const vr::Progress&,vr::Upgrade); void reward(vr::Race&);}\nnamespace vr {"),
        ("return u<UPGRADE_COUNT?availability[stageOf(p.unlocked)][u]:0;",
         "return balance::available(p,u);"),
        ("pulsePerfect(c)?100:60", "pulsePerfect(c)?100:balance::partial"),
        ("if(r.rival.rpm>=opponentShift(r.opponent))shift(r.rival,r.rivalBuild);",
         "if(r.rival.rpm>=opponentShift(r.opponent)){"
         "if(shift(r.rival,r.rivalBuild)!=NONE && r.opponent%6==5){"
         "r.rival.shiftLeft+=balance::delay[stageOf(r.opponent)];"
         "r.rival.windowDelay=r.rival.shiftLeft;}}"),
        ("r.payout=r.basePay+r.skillPay+r.firstPay;",
         "balance::reward(r);r.payout=r.basePay+r.skillPay+r.firstPay;"),
    ]
    for before, after in replacements:
        source = replace_once(source, before, after)
    return source


def run(command: list[str]) -> str:
    return subprocess.run(command, check=True, text=True, capture_output=True).stdout


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--seeds", type=int, default=100)
    parser.add_argument("--out", type=Path, default=ROOT / "out" / "balance")
    parser.add_argument("--sanitize", action="store_true")
    args = parser.parse_args()
    if args.seeds < 1:
        parser.error("--seeds must be positive")
    args.out.mkdir(parents=True, exist_ok=True)
    source = BASELINE.read_text()
    compiler = os.environ.get("CXX", "clang++" if args.sanitize else "g++")
    flags = ["-std=c++17", "-O2", "-Wall", "-Wextra"]
    if args.sanitize:
        flags += ["-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"]

    with tempfile.TemporaryDirectory(prefix="vroom-balance-") as tmp:
        tmp = Path(tmp)
        experiment = tmp / "Game.cpp"
        experiment.write_text(instrument(source))
        baseline_exe, candidate_exe = tmp / "baseline", tmp / "candidate"
        production_exe = tmp / "production"
        for core, output in [(BASELINE, baseline_exe), (experiment, candidate_exe), (ROOT / "Game.cpp", production_exe)]:
            run([compiler, *flags, "-I", str(ROOT), str(DRIVER), str(core), "-o", str(output)])

        # The injected hooks must reproduce every original-core measurement when disabled.
        baseline = run([str(baseline_exe), "0", "0", "0", "0", "0", "0", str(args.seeds), "0"])
        shadow = run([str(candidate_exe), "0", "0", "0", "0", "0", "0", str(args.seeds), "0"])
        if baseline != shadow:
            raise AssertionError("Instrumented baseline differs from actual Game.cpp")
        print("Baseline parity: exact output match with historical 0.2.0 core", flush=True)
        outputs = {"baseline": baseline}
        for name, retry_mode in [("candidate", "0"), ("candidate-direct-retry", "1")]:
            # Stage-3/4/5 boss clutch delays: 2/8/10 ticks; reward preset 1.
            config = ["1", "2", "8", "10", "0", "1", str(args.seeds), retry_mode]
            outputs[name] = run([str(candidate_exe), *config])
            actual = run([str(production_exe), *config])
            if actual != outputs[name]:
                raise AssertionError(f"{name}: shipping Game.cpp differs from simulated candidate")
        print("Production parity: exact candidate output match in both retry models", flush=True)
        summary: dict = {
            "seeds_per_profile_policy": args.seeds,
            "baseline_parity": True,
            "production_parity": True,
            "compiler": compiler,
            "sanitizers": args.sanitize,
            "source_sha256": {
                str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                for p in [ROOT / "Game.cpp", ROOT / "Game.h", ROOT / "Pilot.h", DRIVER, BASELINE]
            },
            "runs": {},
        }
        pattern = re.compile(
            r"CAM profile(\d+) policy(\d+) complete(\d+)/(\d+) minWins(\d+) "
            r"races(\d+)\.\.(\d+) replayAvg([\d.]+) lossAvg([\d.]+) buysAvg([\d.]+) "
            r"maxGap(\d+) maxBulk(\d+) cashAvg(\d+) peak(\d+)"
        )
        for name, output in outputs.items():
            (args.out / f"{name}.txt").write_text(output)
            rows = []
            for line in output.splitlines():
                match = pattern.fullmatch(line)
                if match:
                    fields = ["profile", "policy", "completed", "seeds", "min_wins",
                              "min_races", "max_races", "average_replays", "average_losses",
                              "average_purchases", "max_replay_gap", "max_purchase_batch",
                              "average_final_cash", "peak_cash"]
                    rows.append({key: float(value) if "." in value else int(value)
                                 for key, value in zip(fields, match.groups())})
            if len(rows) != 12:
                raise AssertionError(f"{name}: expected 12 campaign aggregates")
            summary["runs"][name] = rows
            completed, total = sum(r["completed"] for r in rows), sum(r["seeds"] for r in rows)
            print(f"{name}: {completed}/{total} campaigns completed, "
                  f"{min(r['min_races'] for r in rows)}–{max(r['max_races'] for r in rows)} races")
        (args.out / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(f"Verified results: {args.out}")


if __name__ == "__main__":
    main()
