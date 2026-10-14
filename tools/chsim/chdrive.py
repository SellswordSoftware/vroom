#!/usr/bin/env python3
"""Vroom scripts using the repository's CHGame driver."""
from pathlib import Path
import sys
shared = Path(__file__).resolve().parents[3] / "chgamerepo" / "tools" / "chsim"
sys.path.insert(0, str(shared))
from chdrivelib import Driver, main
class VroomDriver(Driver):
    def op(self, name, args, outdir):
        if name == "expect":
            line = self.query("V", "VROOM ")
            state = dict(word.split("=", 1) for word in line.split()[1:])
            key, expected = args
            if state.get(key) != expected:
                raise AssertionError(f"{key}: expected {expected}, got {state.get(key)} ({line})")
            return True
        return False
if __name__ == "__main__":
    raise SystemExit(main(VroomDriver, ident="VRM1"))
