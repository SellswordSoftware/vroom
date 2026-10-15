#!/usr/bin/env python3
"""Pixel checks for unique car renders and seamless skyline scrolling.
Uses the real CHGfx rendering via Vroom's debug-only visual probes.
"""
from pathlib import Path
import sys
import hashlib
import re
from PIL import Image, ImageChops, ImageDraw

GAME=Path(__file__).resolve().parents[2]
SHARED=GAME.parent/"chgamerepo/tools/chsim"
sys.path.insert(0,str(SHARED))
from chsim import build
from chdrivelib import Driver, SimTransport
from fbimage import to_image

def main():
    out=GAME/"out/visuals"
    out.mkdir(parents=True,exist_ok=True)
    transport=SimTransport(build(GAME))
    driver=Driver(transport,"VRM1")
    checks=0
    def screen(command):
        driver.cmd(command)
        driver.frames(1)
        return to_image(driver.shot(),1).convert("RGB")
    try:
        driver.handshake()
        driver.cmd("L1")
        seen=set()
        gallery=Image.new("RGB",(1280,800))
        for i in range(100):
            frame=screen(f"I {i}")
            # Include only the actual car, not its ID or text descriptions.
            body=frame.crop((30,34,97,64))
            fingerprint=hashlib.sha256(body.tobytes()).hexdigest()
            assert fingerprint not in seen, f"Duplicate rendered appearance for opponent {i+1}"
            seen.add(fingerprint);checks+=1
            # Compact contact sheet with ID/body label and native-size car.
            tile=Image.new("RGB",(128,80),(0,0,0))
            tile.paste(frame.crop((0,0,128,24)),(0,0))
            tile.paste(frame.crop((0,32,128,74)),(0,28))
            gallery.paste(tile,((i%10)*128,(i//10)*80))
        gallery.save(out/"opponents-100.png")
        boundary_frames=[]
        header=(GAME/"Appearance.h").read_text()
        sky_scale=int(re.search(r"SKY_SCALE\s*=\s*(\d+)",header).group(1))
        for tile in range(1,33):
            before=screen(f"C {(tile*24-1)*sky_scale}")
            after=screen(f"C {tile*24*sky_scale}")
            # All building pixels shift left one pixel. The moon stays fixed,
            # so mask the union of its old/new locations before comparison.
            old=before.crop((1,0,128,27))
            new=after.crop((0,0,127,27))
            for image in (old,new):
                ImageDraw.Draw(image).rectangle((97,3,115,19),fill=(0,0,0))
            assert ImageChops.difference(old,new).getbbox() is None, f"Skyline jump at tile {tile}"
            checks+=1
            if tile<=4:
                boundary_frames.extend((before.resize((384,384),Image.Resampling.NEAREST),
                                        after.resize((384,384),Image.Resampling.NEAREST)))
        boundary_frames[0].save(out/"skyline-boundaries.gif",save_all=True,
                               append_images=boundary_frames[1:],duration=350,loop=0)
    finally:
        rc=transport.close()
    assert rc==0, f"Simulator exited {rc}"
    assert not transport.bugs, "\n".join(transport.bugs)
    print(f"Visuals: {checks} checks passed (100 distinct rendered cars, 32 skyline boundaries)")
    print(out/"opponents-100.png")
if __name__=="__main__":
    main()
