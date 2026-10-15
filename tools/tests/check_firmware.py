#!/usr/bin/env python3
"""Optional compiled-firmware check using the workspace's existing emulator.
    python3 vroom/tools/tests/check_firmware.py
No physical console is accessed. All ROM/save copies are in out/firmware/.
"""
from pathlib import Path
import json
import re
import shutil
import struct
import subprocess
import zlib

GAME=Path(__file__).resolve().parents[2]
ROOT=GAME.parent
OUT=GAME/"out/firmware"
EMU=ROOT/".build/chgame-emulator/build/chg_headless"
ROM=GAME/"build/release/vroom.ino.bin"
ELF=GAME/"build/release/vroom.ino.elf"
HELPER=r'''
#include "Game.h"
#include "Pilot.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
using namespace vr;
int main(int argc,char **argv) {
    printf("layout cash %zu\n",offsetof(Progress,cash));
    printf("layout wins %zu\n",offsetof(Progress,wins));
    printf("layout imported %zu\n",offsetof(Progress,importNotice));
    printf("layout legacy %zu\n",offsetof(Progress,legacyBest));
    printf("layout phase %zu\n",offsetof(Race,phase));
    Progress p;reset(p);int n=atoi(argv[1]);p.wins=n;p.unlocked=n;
    if(n)for(int u=0;u<UPGRADE_COUNT;++u)p.levels[u]=availableLevel(p,(Upgrade)u);
    printf("levels");for(int u=0;u<UPGRADE_COUNT;++u)printf(" %u",p.levels[u]);puts("");
    Race r;start(r,p,n);
    const char *names[]={"a","b","up"};
    bool held[3]={};int began[3]={};
    int tickN=0;
    while(r.phase!=RESULTS&&tickN<4000) {
        Input in=pilot::input(r);
        bool now[]={in.aHeld||in.aPressed,in.bHeld,in.upPressed};
        for(int b=0;b<3;++b) {
            if(now[b]&&!held[b])began[b]=tickN;
            if(!now[b]&&held[b])printf("event %s %.6f %.6f\n",names[b],began[b]/60.0,
                (tickN-began[b])/60.0);
            held[b]=now[b];
        }
        tick(r,in);++tickN;
    }
    for(int b=0;b<3;++b)if(held[b])printf("event %s %.6f %.6f\n",names[b],began[b]/60.0,
        (tickN-began[b])/60.0);
    printf("finish %.6f\n",tickN/60.0);
}
'''
def run(argv):
    r=subprocess.run(list(map(str,argv)),capture_output=True,text=True,check=True)
    return r.stdout

def page(data,version=2):
    content=bytearray(b"\xff"*256)
    content[:8]=struct.pack("<4sBBH",b"VRM1",version,0,1)
    content[8:8+len(data)]=data
    end=(8+len(data)+3)&~3
    content[end:end+4]=struct.pack("<I",zlib.crc32(content[:end]))
    return struct.pack("<I",0xF600)+content

def main():
    if not EMU.is_file():
        raise SystemExit("Build the workspace emulator first; see README.")
    OUT.mkdir(parents=True,exist_ok=True)
    helper=OUT/"layout.cpp";helper.write_text(HELPER)
    exe=OUT/"layout"
    run(["g++","-std=c++11","-O2","-I",GAME,helper,GAME/"Game.cpp","-o",exe])
    nm=run(["nm","-C",ELF])
    addresses={}
    for line in nm.splitlines():
        for key in ("progress","screen","race"):
            if line.endswith("app::"+key):
                addresses[key]=int(line.split()[0],16)
    assert len(addresses)==3,addresses
    start=min(addresses.values())
    count=(max(addresses.values())+256-start+3)//4
    def emulate(rom,seconds,presses,tag):
        args=[EMU,rom,seconds,OUT/(tag+".ppm"),"--save","--mem",f"{start:x}",count]
        for name,at,duration in presses:
            args+=["--press",f"{name}@{at:.6f}:{duration:.6f}"]
        output=run(args)
        assert "halted" not in output.lower() and "hardfault" not in output.lower(),output[-1000:]
        memory={}
        for address,value in re.findall(r"mem ([0-9a-f]+) (\d+)",output):
            memory[int(address,16)]=struct.pack("<I",int(value))
        def byte(address):
            aligned=address&~3
            return memory[aligned][address-aligned]
        def word(address):
            return sum(byte(address+i)<<(8*i) for i in range(4))
        fps=re.search(r"game ([\d.]+) fps",output)
        return byte,word,(fps.group(1) if fps else "?")
    layouts={}
    for n in (0,6,12,18,26,35):
        output=run([exe,n])
        for key,value in re.findall(r"layout (\w+) (\d+)",output):
            layouts[key]=int(value)
        levels=list(map(int,re.search(r"levels([^\n]+)",output).group(1).split()))
        data=bytearray(112)
        seed_cash=0 if n==0 else 99999
        struct.pack_into("<I",data,0,seed_cash)
        data[96:104]=bytes(levels)
        data[104]=data[105]=n
        rom=OUT/f"stage-{n//6+1}.bin"
        shutil.copyfile(ROM,rom)
        rom.with_suffix(".sav").write_bytes(page(data))
        presses=[("a",1.5,0.15),("a",2.5,0.15)]
        for name,at,duration in re.findall(r"event (\w+) ([\d.]+) ([\d.]+)",output):
            presses.append((name,float(at)+2.55,max(float(duration),0.05)))
        end=float(re.search(r"finish ([\d.]+)",output).group(1))+4.55
        byte,word,fps=emulate(rom,end,presses,f"stage-{n//6+1}-result")
        base=addresses["progress"]
        assert byte(addresses["screen"])==4,(n,"screen",byte(addresses["screen"]))
        assert byte(base+layouts["wins"])==n+1,(n,"wins",byte(base+layouts["wins"]))
        cash=word(base+layouts["cash"])
        assert cash>seed_cash,(n,"payout",cash)
        if(n==0): assert cash==440,("first perfect win",cash)
        byte,word,_=emulate(rom,2,[],f"stage-{n//6+1}-reload")
        assert byte(addresses["screen"])==0
        assert byte(base+layouts["wins"])==n+1
        assert word(base+layouts["cash"])==cash
        print(f"Firmware stage {n//6+1}: won rival {n+1}; save survives restart; {fps} LCD fps")
    old=bytearray(36)
    struct.pack_into("<IH",old,0,1234,1023)
    old[6]=9;old[7:13]=bytes((5,3,5,1,1,1));old[13]=1
    for i in range(10):struct.pack_into("<H",old,14+2*i,1200+i)
    rom=OUT/"legacy-import.bin";shutil.copyfile(ROM,rom)
    rom.with_suffix(".sav").write_bytes(page(old,1))
    byte,word,fps=emulate(rom,3,[("a",1.5,0.15)],"legacy-import")
    base=addresses["progress"]
    assert byte(addresses["screen"])==0
    assert byte(base+layouts["wins"])==10
    assert word(base+layouts["cash"])==4534
    assert byte(base+layouts["imported"])==0
    assert word(base+layouts["legacy"])&65535==1200
    byte,word,_=emulate(rom,2,[],"legacy-reload")
    assert word(base+layouts["cash"])==4534
    assert byte(base+layouts["wins"])==10
    assert byte(addresses["screen"])==0
    assert word(base+layouts["legacy"])&65535==1200
    print("Firmware migration: old cash/upgrades/wins imported, records archived, v2 reload passed")
if __name__=="__main__":
    main()
