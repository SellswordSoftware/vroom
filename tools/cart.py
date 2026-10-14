"""Original pixel cover for Vroom's visual-menu cartridge.
Regenerate with: python3 vroom/tools/vroom.py boxart
"""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "chgamerepo/tools"))
import boxart as b
from pixkit import INK, NAVY, BLUE, CYAN, WHITE, SILVER, RED, GOLD, WOOD

def draw():
    fb = b.canvas(NAVY)
    # Moon and city skyline: windows are coherent rectangular clusters.
    fb.ellipse(105, 47, 8, 8, SILVER)
    fb.ellipse(102, 44, 8, 8, NAVY)
    for i in range(9):
        x = i*16-4
        h = 15 + i*11%23
        fb.fill_rect(x, 75-h, 13, h, INK)
        for y in range(78-h, 71, 6):
            for w in range(2):
                fb.fill_rect(x+3+w*5, y, 2, 3, BLUE)
    fb.fill_rect(0, 75, 128, 53, INK)
    fb.line(0, 75, 127, 75, CYAN)
    fb.line(0, 112, 127, 112, BLUE)
    for x in range(-8, 128, 32):
        fb.fill_rect(x, 96, 18, 2, SILVER)
    # Far-lane rival and foreground yellow coupe.
    def car(x, y, color, scale):
        def rect(rx,ry,w,h,c):
            fb.fill_rect(x+rx*scale,y+ry*scale,w*scale,h*scale,c)
        rect(1,6,36,6,color)
        rect(10,1,16,7,color)
        fb.line(x+7*scale,y+6*scale,x+11*scale,y+scale,color)
        fb.line(x+26*scale,y+scale,x+31*scale,y+6*scale,color)
        rect(12,2,6,4,NAVY); rect(20,2,6,4,NAVY)
        rect(13,2,4,1,CYAN); rect(21,2,4,1,CYAN)
        rect(2,7,33,1,WHITE)
        rect(34,8,4,2,GOLD)
        rect(0,3,6,2,color); rect(3,4,1,3,color)
        rect(13,12,12,1,WOOD)
        for wheel in (8,29):
            fb.ellipse(x+wheel*scale,y+12*scale,4*scale,4*scale,INK)
            fb.ellipse(x+wheel*scale,y+12*scale,2*scale,2*scale,SILVER)
            rect(wheel,12,1,1,INK)
    car(78,78,RED,1)
    car(18,83,GOLD,2)
    fb.fill_rect(4,4,120,40,NAVY)
    b.title(fb,"VROOM",8,scale=2,fill=GOLD,shadow=WOOD)
    fb.text35(44,38,"SHIFT KING",CYAN)
    fb.text35(16,118,"SHIFT. EARN. BUILD.",WHITE)
    fb.rect(2,2,124,124,BLUE)
    return fb
