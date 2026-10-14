#!/usr/bin/env python3
"""Run the local CHGame tools with this sketch and the available CLI.

From the workspace root:
    python3 vroom/tools/vroom.py build
    python3 vroom/tools/vroom.py check --no-device
    python3 vroom/tools/vroom.py gif
    python3 vroom/tools/vroom.py export --no-build
    python3 vroom/tools/vroom.py upload
"""
from pathlib import Path
import os
import shutil
import subprocess
import sys
game = Path(__file__).resolve().parents[1]
workspace = game.parent
env = dict(os.environ)
if not shutil.which("arduino-cli"):
    candidates = [
        workspace / ".build/arduino-cli/resources/app/lib/backend/resources/arduino-cli",
        workspace / "FirstGame/bin/arduino-cli",
    ]
    for cli in candidates:
        if cli.is_file():
            env["PATH"] = str(cli.parent) + os.pathsep + env.get("PATH", "")
            break
tool = workspace / "chgamerepo/tools/chgame.py"
if not tool.is_file():
    raise SystemExit("Place vroom beside the chgamerepo checkout to use these tools.")
raise SystemExit(subprocess.run(
    [sys.executable, str(tool), "--sketch", str(game), *sys.argv[1:]],
    env=env, cwd=workspace).returncode)
