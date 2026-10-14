#include "Game.h"
#include <string.h>
namespace vr {
static const uint16_t ratios[5] = {600, 400, 270, 200, 150};
static const char *const names[10] = {
    "CURBSIDE", "NIGHT OWL", "RED SIGNAL", "MIDNIGHT",
    "AFTERHOURS", "GHOSTLINE", "NEON VIPER", "BLACKTOP",
    "NIGHTFALL", "SHIFT KING"
};
static int32_t clamp(int32_t x, int32_t lo, int32_t hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}
void reset(Progress &p) { memset(&p, 0, sizeof(p)); }
uint8_t limit(Upgrade u) {
    static const uint8_t caps[UPGRADE_COUNT] = {5, 3, 5, 1, 1, 1};
    return u < UPGRADE_COUNT ? caps[u] : 0;
}
bool valid(const Progress &p) {
    if (p.unlocked > 9 || p.cash > 999999 || p.wonMask > 1023 || p.champion > 1) return false;
    for (uint8_t i=0; i<UPGRADE_COUNT; ++i)
        if (p.levels[i] > limit((Upgrade)i)) return false;
    if (p.levels[FIFTH] && !p.levels[FOURTH]) return false;
    if (p.unlocked < 3 && (p.levels[FOURTH] || p.levels[NOS])) return false;
    if (p.unlocked < 6 && p.levels[FIFTH]) return false;
    const uint16_t allowed = (uint16_t)((1u << (p.unlocked + 1)) - 1u);
    if (p.wonMask & ~allowed) return false;
    if (p.unlocked && (p.wonMask & ((1u << p.unlocked)-1u)) != ((1u << p.unlocked)-1u)) return false;
    if (p.champion != ((p.wonMask & 512) != 0)) return false;
    for (uint8_t i=0; i<10; ++i)
        if (p.best[i] && (i > p.unlocked || p.best[i] > 3600)) return false;
    return true;
}
Build build(const Progress &p) {
    return {p.levels[ENGINE], p.levels[GEARBOX], p.levels[TIRES],
        (uint8_t)(3+p.levels[FOURTH]+p.levels[FIFTH]), p.levels[NOS] != 0};
}
Build opponentBuild(uint8_t n) {
    // Fixed builds: wins unlock the next setup, never scale with player upgrades.
    static const Build rivals[10] = {
        {0,0,0,3,false}, {1,0,0,3,false}, {1,1,1,3,false},
        {2,1,1,4,false}, {2,1,2,4,true}, {3,1,2,4,true},
        {3,2,3,5,true}, {4,2,3,5,true}, {4,3,4,5,true},
        {5,3,4,5,true}
    };
    return rivals[n < 10 ? n : 9];
}
uint16_t opponentShift(uint8_t n) { return (uint16_t)(6400 + (n < 10 ? n : 9)*90); }
const char *opponentName(uint8_t n) { return names[n < 10 ? n : 9]; }
uint32_t price(const Progress &p, Upgrade u) {
    if (u >= UPGRADE_COUNT || p.levels[u] >= limit(u)) return 0;
    static const uint16_t engine[5] = {350,650,1000,1400,1700};
    static const uint16_t box[3] = {400,800,1300};
    static const uint16_t tires[5] = {300,550,850,1200,1500};
    if (u == ENGINE) return engine[p.levels[u]];
    if (u == GEARBOX) return box[p.levels[u]];
    if (u == TIRES) return tires[p.levels[u]];
    return u == FOURTH ? 900 : u == FIFTH ? 1400 : 1000;
}
bool unlocked(const Progress &p, Upgrade u) {
    if (u >= UPGRADE_COUNT) return false;
    if (u == FOURTH || u == NOS) return p.unlocked >= 3;
    if (u == FIFTH) return p.unlocked >= 6 && p.levels[FOURTH];
    return true;
}
bool buy(Progress &p, Upgrade u) {
    const uint32_t cost = price(p,u);
    if (!cost || !unlocked(p,u) || p.cash < cost) return false;
    p.cash -= cost;
    ++p.levels[u];
    return true;
}
void resetCar(Car &c) {
    memset(&c,0,sizeof(c)); c.gear=1; c.rpm=2500;
}
Shift shift(Car &c, const Build &b) {
    if (c.gear >= b.gears || c.shiftLeft || c.finishTime) return NONE;
    const int32_t delta = (int32_t)c.rpm - IDEAL;
    Shift quality = delta >= -300 && delta <= 300 ? PERFECT :
        delta >= -650 && delta <= 450 ? GOOD : delta < 0 ? EARLY : LATE;
    c.feedback=quality; c.feedbackLeft=45;
    if (quality == PERFECT) ++c.perfects;
    if (quality == GOOD) ++c.goods;
    ++c.gear;
    c.shiftLeft=(uint16_t)(24 - b.gearbox*5); // 400 -> 150 ms
    c.rpm=(uint16_t)clamp(c.speed*ratios[c.gear-1]/1000, 1200, 8500);
    return quality;
}
bool boost(Car &c, const Build &b) {
    if (!b.nos || c.boostUsed || c.finishTime) return false;
    c.boostUsed=true; c.boostLeft=120; return true;
}
void stepCar(Car &c, const Build &b) {
    if (c.finishTime) return;
    ++c.time;
    c.rpm=(uint16_t)clamp(c.speed*ratios[c.gear-1]/1000, c.gear==1 ? 2500 : 1200, 8500);
    const int32_t power = 22000 + b.engine*9000;
    // Broad power peak at 6000 RPM. Shifting near 7300 keeps both gears
    // close to their peak without synthetic speed/accuracy bonuses.
    const int32_t curve = (int32_t)c.rpm*(12000-c.rpm)/36000; // per mille
    int32_t force = (power*curve)/(c.speed > 4500 ? c.speed : 4500);
    if (c.boostLeft) force=force*145/100;
    const int32_t grip=2600+b.tires*750;
    c.slipping=force>grip && !c.shiftLeft && c.rpm<REDLINE;
    if (force>grip) force=grip;
    if (c.shiftLeft || c.rpm >= REDLINE) force=0;
    const int32_t drag=120+(c.speed/10)*(c.speed/10)/18000;
    const int32_t accel=(force-drag)*1000/900;
    c.speedRemainder += accel;
    c.speed=clamp(c.speed+c.speedRemainder/HZ,0,65000);
    c.speedRemainder%=HZ;
    const int32_t before=c.distance;
    c.distanceRemainder+=c.speed;
    c.distance+=c.distanceRemainder/HZ;
    c.distanceRemainder%=HZ;
    if (c.distance>=FINISH && before<FINISH) c.finishTime=c.time;
    if (c.shiftLeft) --c.shiftLeft;
    if (c.boostLeft) --c.boostLeft; // boost burns even with clutch disengaged
    if (c.feedbackLeft) --c.feedbackLeft;
}
void start(Race &r, const Progress &p, uint8_t n) {
    memset(&r,0,sizeof(r));
    r.opponent=n <= p.unlocked && n<10 ? n : p.unlocked;
    r.playerBuild=build(p); r.rivalBuild=opponentBuild(r.opponent);
    resetCar(r.player); resetCar(r.rival);
    r.countdown=180; r.phase=COUNTDOWN;
}
void tick(Race &r, bool shiftPressed, bool boostPressed) {
    if (r.paused || r.phase==RESULTS) return;
    if (r.phase==COUNTDOWN) {
        if (--r.countdown==0) r.phase=RUNNING;
        return; // input during countdown cannot shift or spend boost
    }
    if (shiftPressed) shift(r.player,r.playerBuild);
    if (boostPressed) boost(r.player,r.playerBuild);
    if (r.rival.rpm>=opponentShift(r.opponent)) shift(r.rival,r.rivalBuild);
    if (r.rival.gear==3 && !r.rival.shiftLeft && r.rival.rpm>=5000) boost(r.rival,r.rivalBuild);
    stepCar(r.player,r.playerBuild); stepCar(r.rival,r.rivalBuild); ++r.elapsed;
    if (r.player.finishTime || r.elapsed>=3600) {
        r.phase=RESULTS; r.dnf=!r.player.finishTime;
        r.won=!r.dnf && (!r.rival.finishTime || r.player.finishTime<r.rival.finishTime);
    }
}
bool settle(Race &r, Progress &p) {
    if (r.phase!=RESULTS || r.settled) return false;
    r.settled=true;
    if (r.dnf) return true;
    const uint16_t bit=(uint16_t)(1u<<r.opponent);
    r.firstWin=r.won && !(p.wonMask&bit);
    r.basePay= r.won ? 160+r.opponent*65 : 70+r.opponent*25;
    r.skillPay=r.player.perfects*30+r.player.goods*12;
    r.firstPay=r.firstWin ? 220+r.opponent*90 : 0;
    r.payout=r.basePay+r.skillPay+r.firstPay;
    p.cash=(uint32_t)clamp((int32_t)(p.cash+r.payout),0,999999);
    r.record=!p.best[r.opponent] || r.player.finishTime<p.best[r.opponent];
    if (r.record) p.best[r.opponent]=r.player.finishTime;
    if (r.won) {
        p.wonMask|=bit;
        if (r.opponent==p.unlocked && p.unlocked<9) ++p.unlocked;
        if (r.opponent==9) p.champion=1;
    }
    return true;
}
uint16_t mph(const Car &c) { return (uint16_t)(c.speed*100/44704); }
}
