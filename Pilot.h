#pragma once
#include "Game.h"
// Deterministic test/debug pilot. Unreferenced code is absent from release images.
namespace pilot {
struct Skill { uint16_t shiftRpm, launchRpm; uint8_t reaction, nos, pulse; };
inline Skill perfect() { return {vr::IDEAL,vr::LAUNCH_RPM,0,1,1}; }
inline Skill good() { return {6800,3850,6,1,2}; }
inline vr::Input input(const vr::Race &r,Skill skill=perfect()) {
    vr::Input in={};
    if(r.playerBuild.manual&&!r.player.launched) {
        int holdFrames=(skill.launchRpm-1800)/40;
        if(holdFrames<1)holdFrames=1;
        in.aHeld=r.phase==vr::COUNTDOWN?r.countdown<=holdFrames:r.elapsed<skill.reaction;
        in.aReleased=r.player.heldA&&!in.aHeld;
    } else if(r.phase==vr::RUNNING) {
        in.aPressed=r.player.gear<r.playerBuild.gears&&r.player.rpm>=skill.shiftRpm&&!r.player.shiftLeft;
        in.aHeld=in.aPressed;
    }
    const bool longRace=r.length>vr::FINISH;
    in.bHeld=r.player.fuel&&(skill.nos==2 || (skill.nos==1&&r.player.gear>=(longRace?4:3)&&
        !r.player.shiftLeft&&!r.player.pulseLeft&&r.player.rpm>=4700));
    uint8_t pulseGear=longRace?(uint8_t)(r.playerBuild.gears-r.player.charges+1):3;
    in.upPressed=r.player.charges&&r.player.gear>=pulseGear&&((skill.pulse==1&&r.player.window==14) ||
        (skill.pulse==2&&r.player.window==7));
    return in;
}
}
