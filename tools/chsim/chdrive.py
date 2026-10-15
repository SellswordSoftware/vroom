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
        if name == "until":
            key, expected = args[:2]
            limit = int(args[2]) if len(args)>2 else 4000
            for tick in range(limit+1):
                line=self.query("V","VROOM ")
                state=dict(word.split("=",1) for word in line.split()[1:])
                if state.get(key)==expected:
                    return True
                self.frames(1)
            raise AssertionError(f"Timed out waiting for {key}={expected}; {line}")
        if name in ("expectgt","expectlt"):
            key,expected=args
            line=self.query("V","VROOM ")
            state=dict(word.split("=",1) for word in line.split()[1:])
            got=int(state[key])
            if not (got>int(expected) if name=="expectgt" else got<int(expected)):
                raise AssertionError(f"{name} {key} {expected}, got {got}")
            return True
        return False
if __name__ == "__main__":
    raise SystemExit(main(VroomDriver, ident="VRM1"))
