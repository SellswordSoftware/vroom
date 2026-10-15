#!/usr/bin/env python3
"""Refresh or verify the self-contained Vroom Pages bundle."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil

GAME = Path(__file__).resolve().parents[1]
SITE = GAME / "site"

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="check committed assets without an Arduino toolchain")
    args = parser.parse_args()
    manifest = json.loads((SITE / "vendor/manifest.json").read_text())
    for name, expected in manifest["files"].items():
        if digest(SITE / "vendor" / name) != expected:
            raise SystemExit(f"Emulator asset mismatch: {name}")
    engine_hash = hashlib.sha256("".join(manifest["files"].values()).encode()).hexdigest()[:16]
    version = json.loads((GAME / "chgame.json").read_text())["version"]
    if not args.check:
        rom = GAME / "build/release/vroom.ino.bin"
        if not rom.is_file():
            raise SystemExit("Build Vroom first: python3 tools/vroom.py build")
        shutil.copyfile(rom, SITE / "vroom.bin")
        (SITE / "VROOM-LICENSE.txt").write_text((GAME / "LICENSE").read_text() + "\n" + (GAME / "NOTICE").read_text())
        config = dict(version=version, romHash=digest(SITE / "vroom.bin"), engineHash=engine_hash)
        (SITE / "game-config.js").write_text("window.VROOM_CONFIG = " + json.dumps(config, indent=2) + ";\n")
    config_text = (SITE / "game-config.js").read_text()
    config = json.loads(config_text.removeprefix("window.VROOM_CONFIG = ").rstrip().removesuffix(";"))
    assert config["version"] == version, "Site version is stale; run tools/site.py"
    assert config["romHash"] == digest(SITE / "vroom.bin"), "Site ROM hash mismatch"
    assert config["engineHash"] == engine_hash, "Runtime cache version mismatch"
    assert 0 < (SITE / "vroom.bin").stat().st_size <= 50432, "ROM exceeds CHGame two-save-page budget"
    for file in ["index.html","style.css","app.js",".nojekyll","THIRD_PARTY_NOTICES.txt","VROOM-LICENSE.txt"]:
        assert (SITE / file).is_file(), f"Missing site file: {file}"
    print(f"Site verified: Vroom {version}, {(SITE / 'vroom.bin').stat().st_size} byte ROM, matching JS/Wasm hashes")

if __name__ == "__main__":
    main()
