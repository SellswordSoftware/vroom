// Historical 0.2.0 core, retained only for the offline balance comparison.
#include "Game.h"
#include <string.h>
namespace vr {
static const uint16_t ratios[7]={600,430,320,250,210,160,145};
static const uint8_t caps[UPGRADE_COUNT]={7,4,6,4,5,4,4,3};
static const uint8_t availability[STAGES][UPGRADE_COUNT]={
    {2,1,2,0,0,0,0,0}, {3,2,3,1,0,0,0,0},
    {4,2,4,2,1,1,0,0}, {5,3,4,2,2,2,1,1},
    {6,3,5,3,4,3,2,2}, {7,4,6,4,5,4,4,3}
};
static int32_t clamp(int32_t n,int32_t lo,int32_t hi) { return n<lo?lo:n>hi?hi:n; }
static int32_t absolute(int32_t n) { return n<0?-n:n; }
uint8_t stageOf(uint8_t n) { return (n<RIVALS?n:RIVALS-1)/PER_STAGE; }
void reset(Progress &p) { memset(&p,0,sizeof(p)); }
uint8_t limit(Upgrade u) { return u<UPGRADE_COUNT?caps[u]:0; }
bool hasWon(const Progress &p,uint8_t n) { return n<RIVALS && n<p.wins; }
bool valid(const Progress &p) {
    if(p.cash>999999 || p.wins>RIVALS || p.unlocked!=(p.wins<RIVALS?p.wins:RIVALS-1) ||
       p.champion!=(p.wins==RIVALS) || p.legacyWins>10 || p.importNotice>1) return false;
    for(uint8_t i=0;i<UPGRADE_COUNT;++i) if(p.levels[i]>caps[i]) return false;
    for(uint8_t i=0;i<RIVALS;++i)
        if(p.best[i] && (i>p.unlocked || p.best[i]>3600)) return false;
    for(uint8_t i=0;i<10;++i) if(p.legacyBest[i]>3600) return false;
    return true;
}
bool migrate(const LegacyProgress &old,Progress &p) {
    if(old.cash>999999 || old.unlocked>9 || old.wonMask>1023 || old.champion>1) return false;
    static const uint8_t oldCaps[6]={5,3,5,1,1,1};
    for(uint8_t i=0;i<6;++i) if(old.levels[i]>oldCaps[i]) return false;
    uint8_t won=0; while(won<10 && (old.wonMask&(1u<<won))) ++won;
    if(old.wonMask!=((1u<<won)-1u) || old.unlocked!=(won<10?won:9) ||
       old.champion!=(won==10) || (old.levels[4]&&!old.levels[3]) ||
       (old.unlocked<3 && (old.levels[3]||old.levels[5])) ||
       (old.unlocked<6 && old.levels[4])) return false;
    for(uint8_t i=0;i<10;++i) if(old.best[i]>3600) return false;
    reset(p);
    p.cash=(uint32_t)clamp(old.cash+old.levels[3]*900u+old.levels[4]*1400u+
                          old.levels[5]*1000u,0,999999);
    p.wins=won; p.unlocked=won; p.legacyWins=won; p.importNotice=1;
    for(uint8_t i=0;i<3;++i) p.levels[i]=old.levels[i];
    memcpy(p.legacyBest,old.best,sizeof(old.best));
    return valid(p);
}
static Build makeBuild(const uint8_t *levels,uint8_t wins) {
    Build b={levels[ENGINE],levels[GEARBOX],levels[TIRES],
        (uint8_t)(3+(wins>=6)+(wins>=12)+(wins>=18)+(wins>=24)),
        levels[LAUNCH],levels[NOS_CAPACITY],levels[NOS_PRESSURE],
        levels[OVERDRIVE_POWER],levels[OVERDRIVE_CHARGES],
        wins>=6,wins>=12,wins>=18};
    return b;
}
Build build(const Progress &p) { return makeBuild(p.levels,p.wins); }
uint8_t availableLevel(const Progress &p,Upgrade u) {
    return u<UPGRADE_COUNT?availability[stageOf(p.unlocked)][u]:0;
}
uint8_t unlockStage(Upgrade u,uint8_t nextLevel) {
    for(uint8_t s=0;s<STAGES;++s)
        if(u<UPGRADE_COUNT && availability[s][u]>=nextLevel) return s;
    return STAGES-1;
}
uint32_t price(const Progress &p,Upgrade u) {
    if(u>=UPGRADE_COUNT || p.levels[u]>=caps[u]) return 0;
    static const uint16_t costs[UPGRADE_COUNT][7]={
        {350,650,950,1250,1550,1900,2300},
        {400,700,1050,1450},
        {300,550,850,1150,1450,1800},
        {350,650,950,1250},
        {450,750,1050,1350,1650},
        {400,700,1000,1300},
        {500,850,1200,1550},
        {650,1100,1550}
    };
    return costs[u][p.levels[u]];
}
bool unlocked(const Progress &p,Upgrade u) { return u<UPGRADE_COUNT && p.levels[u]<availableLevel(p,u); }
bool buy(Progress &p,Upgrade u) {
    uint32_t cost=price(p,u);
    if(!cost || !unlocked(p,u) || p.cash<cost) return false;
    p.cash-=cost; ++p.levels[u]; return true;
}
const char *stageName(uint8_t stage) {
    static const char *const names[]={"STREET SCHOOL","GREEN LIGHT","NITRO NIGHTS",
        "PULSE CIRCUIT","DISTANCE RUN","SHIFT ROYALTY"};
    return names[stage<STAGES?stage:STAGES-1];
}
const char *opponentName(uint8_t n) {
    static const char *const names[RIVALS]={
        "CURBSIDE","NIGHT OWL","RED SIGNAL","SIDEWINDER","SPARK","STREET KING",
        "QUICKDRAW","GREEN GHOST","HOOKSHOT","REDLINE","TREE MASTER","HOLESHOT",
        "BLUE BOTTLE","TORCH","PRESSURE","LONG FUSE","NITRO FOX","AFTERBURN",
        "SWITCHBACK","DOUBLE TAP","LIVE WIRE","PULSE RUNNER","CHAIN REACT","OVERLORD",
        "SHORT FUSE","EIGHTH ACE","LONG SHADOW","MIDNIGHT","SPRINT KING","HORIZON",
        "BLACKTOP","QUICKSILVER","NIGHTFALL","VOLT","LAST LIGHT","SHIFT KING"};
    return names[n<RIVALS?n:RIVALS-1];
}
const char *opponentTrait(uint8_t n) {
    static const char *const traits[]={"FAST LAUNCH","HIGH POWER","SPRINTER",
        "LATE BOOST","ALL ROUNDER","STAGE BOSS"};
    return traits[n%PER_STAGE];
}
int32_t raceLength(uint8_t n) {
    const uint8_t s=stageOf(n), pos=n%PER_STAGE;
    if(s<2) return FINISH/2; // short launch/shift lessons before boost systems
    if(s<4) return FINISH;   // quarter mile remains the most common format
    if(s==4) return pos==1?FINISH/2:pos==2||pos==5?FINISH*2:FINISH;
    return pos==1?FINISH/2:pos==2||pos==4||pos==5?FINISH*2:FINISH;
}
const char *distanceName(uint8_t n) {
    int32_t len=raceLength(n); return len==FINISH/2?"1/8 MI":len==FINISH?"1/4 MI":"1/2 MI";
}
Build opponentBuild(uint8_t n) {
    n=n<RIVALS?n:RIVALS-1;
    const uint8_t s=stageOf(n),pos=n%PER_STAGE;
    uint8_t levels[UPGRADE_COUNT]={};
    // Fixed chapter builds with varied launch, power, and boost specialties.
    for(uint8_t u=0;u<UPGRADE_COUNT;++u) {
        uint8_t lo=s?availability[s-1][u]:0,hi=availability[s][u];
        levels[u]=(uint8_t)(lo+(hi-lo)*pos/5);
    }
    if(pos==0 && levels[ENGINE]) --levels[ENGINE];
    if(pos==1) {
        if(levels[TIRES])--levels[TIRES];
        if(levels[ENGINE]<availability[s][ENGINE])++levels[ENGINE];
    }
    if(pos==2)levels[GEARBOX]=availability[s][GEARBOX];
    if(pos==2 && levels[GEARBOX]) --levels[GEARBOX];
    if(pos==3 && levels[NOS_CAPACITY]) --levels[NOS_CAPACITY];
    // Bosses have near-cap builds but retain a small equipment disadvantage.
    if(pos==5 && s<3 && levels[GEARBOX]) --levels[GEARBOX];
    if(pos==5 && s==5 && levels[ENGINE]) --levels[ENGINE];
    return makeBuild(levels,s*PER_STAGE);
}
uint16_t opponentShift(uint8_t n) { return (uint16_t)(6400+stageOf(n)*95+(n%PER_STAGE)*95); }
uint16_t fuelCapacity(const Build &b) {
    static const uint16_t fuel[6]={120,156,204,252,312,420};
    return b.nos?fuel[b.capacity<=5?b.capacity:5]:0;
}
uint8_t pulseCapacity(const Build &b) { return b.overdrive?(uint8_t)(1+b.charges):0; }
void resetCar(Car &c,const Build &b) {
    memset(&c,0,sizeof(c)); c.gear=1;c.rpm=2500;c.stagingRpm=1800;
    c.fuel=fuelCapacity(b);c.charges=pulseCapacity(b);
    c.launchEfficiency=100;c.launched=!b.manual;
}
static Shift launch(Car &c,const Build &b,bool falseStart) {
    c.launched=true;c.falseStart=falseStart;
    const int32_t error=absolute((int32_t)c.stagingRpm-LAUNCH_RPM);
    const int tolerance=400+b.launch*120;
    c.launchGrade=error<=tolerance?PERFECT:error<=tolerance+700?GOOD:
        c.stagingRpm<LAUNCH_RPM?EARLY:LATE;
    c.launchEfficiency=(uint8_t)clamp(100-(error>tolerance?(error-tolerance)/24:0),45,100);
    c.launchDelay=falseStart?30:0;
    c.launchTicks=60;c.launchFeedback=90;
    if(falseStart) c.launchGrade=LATE;
    return c.launchGrade;
}
Shift shift(Car &c,const Build &b) {
    if(!c.launched || c.gear>=b.gears || c.shiftLeft || c.finishTime || c.launchDelay) return NONE;
    const int32_t delta=(int32_t)c.rpm-IDEAL;
    Shift quality=delta>=-300&&delta<=300?PERFECT:delta>=-650&&delta<=450?GOOD:delta<0?EARLY:LATE;
    c.feedback=quality;c.feedbackLeft=40;
    if(quality==PERFECT)++c.perfects;
    if(quality==GOOD)++c.goods;
    ++c.gear;c.shiftLeft=(uint16_t)(24-b.gearbox*4);
    c.window=0;c.windowDelay=c.shiftLeft;
    c.windowArmed=b.overdrive&&(quality==PERFECT||quality==GOOD);
    c.rpm=(uint16_t)clamp(c.speed*ratios[c.gear-1]/1000,1200,8500);
    return quality;
}
bool pulsePerfect(const Car &c) { return c.window>=10&&c.window<=18; }
bool overdrive(Car &c,const Build &b) {
    if(!b.overdrive || !c.charges || !c.launched || c.finishTime || c.pulseLeft) return false;
    --c.charges;
    if(c.window) {
        c.pulseQuality=pulsePerfect(c)?100:60;
        c.pulseLeft=(uint16_t)(42+b.pulse*6);++c.pulses;
    } else { c.pulseQuality=0;++c.pulseMisses; }
    c.pulseFeedback=45;c.window=0;c.windowArmed=false;
    return true;
}
void stepCar(Car &c,const Build &b,int32_t finish,bool nosHeld) {
    if(c.finishTime) return;
    ++c.time;
    c.boosting=false;c.slipping=false;
    if(c.feedbackLeft)--c.feedbackLeft;
    if(c.launchFeedback)--c.launchFeedback;
    if(c.pulseFeedback)--c.pulseFeedback;
    if(c.window)--c.window;
    if(c.windowDelay)--c.windowDelay;
    if(c.windowArmed && !c.windowDelay) { c.window=24;c.windowArmed=false; }
    if(!c.launched) { c.rpm=c.stagingRpm;return; }
    if(c.launchDelay) { --c.launchDelay;return; }
    c.rpm=(uint16_t)clamp(c.speed*ratios[c.gear-1]/1000,c.gear==1?2500:1200,8500);
    const int32_t power=22000+b.engine*9000;
    const int32_t curve=(int32_t)c.rpm*(12000-c.rpm)/36000;
    int32_t force=power*curve/(c.speed>4500?c.speed:4500);
    if(b.nos&&nosHeld&&c.fuel) {
        force=force*(130+b.pressure*8)/100;
        --c.fuel;++c.fuelUsed;c.boosting=true;
    }
    if(c.pulseLeft) force=force*(100+(50+b.pulse*15)*c.pulseQuality/100)/100;
    const int32_t grip=2600+b.tires*700;
    c.slipping=(force>grip || (c.launchTicks&&c.stagingRpm>5400))&&!c.shiftLeft&&c.rpm<REDLINE;
    if(force>grip)force=grip;
    if(c.launchTicks) { force=force*c.launchEfficiency/100;--c.launchTicks; }
    if(c.shiftLeft || c.rpm>=REDLINE) force=0;
    const int32_t drag=120+(c.speed/10)*(c.speed/10)/18000;
    c.speedRemainder+=(force-drag)*1000/900;
    c.speed=clamp(c.speed+c.speedRemainder/HZ,0,80000);
    c.speedRemainder%=HZ;
    c.distanceRemainder+=c.speed;c.distance+=c.distanceRemainder/HZ;c.distanceRemainder%=HZ;
    if(c.distance>=finish)c.finishTime=c.time;
    if(c.shiftLeft)--c.shiftLeft;
    if(c.pulseLeft)--c.pulseLeft;
}
void start(Race &r,const Progress &p,uint8_t n) {
    memset(&r,0,sizeof(r));
    r.opponent=n<=p.unlocked&&n<RIVALS?n:p.unlocked;
    r.length=raceLength(r.opponent);
    r.playerBuild=build(p);r.rivalBuild=opponentBuild(r.opponent);
    resetCar(r.player,r.playerBuild);resetCar(r.rival,r.rivalBuild);
    r.countdown=180;r.phase=COUNTDOWN;r.launchInputReady=!r.playerBuild.manual;
}
void tick(Race &r,Input in) {
    if(r.paused||r.phase==RESULTS)return;
    Car &p=r.player;
    const bool manual=r.playerBuild.manual;
    // The A press selecting a race must be released before staging can arm.
    if(manual&&!r.launchInputReady) {
        if(!in.aHeld)r.launchInputReady=true;
        in.aHeld=false;in.aPressed=false;in.aReleased=false;
    }
    if(manual&&!p.launched) {
        if(in.aHeld) { p.stagingRpm=(uint16_t)clamp(p.stagingRpm+40,1800,8000);p.launchArmed=true; }
        else if(!p.launchArmed) p.stagingRpm=1800;
        if(in.aReleased && p.launchArmed) launch(p,r.playerBuild,r.phase==COUNTDOWN);
        p.rpm=p.stagingRpm;
    }
    p.heldA=in.aHeld;
    if(r.phase==COUNTDOWN) {
        if(--r.countdown==0) {
            r.phase=RUNNING;
            if(r.rivalBuild.manual) {
                static const uint16_t launchRevs[6]={4200,3500,4100,3800,4000,4200};
                r.rival.stagingRpm=launchRevs[r.opponent%6];
                launch(r.rival,r.rivalBuild,false);
                static const uint8_t reactions[6]={2,10,4,8,6,10};
                r.rival.launchDelay=reactions[r.opponent%6];
            }
        }
        return;
    }
    if(in.aPressed) shift(p,r.playerBuild);
    if(in.upPressed)overdrive(p,r.playerBuild);
    if(r.rival.rpm>=opponentShift(r.opponent))shift(r.rival,r.rivalBuild);
    const bool longRace=r.length>FINISH;
    uint8_t pulseGear=longRace?(uint8_t)(r.rivalBuild.gears-r.rival.charges+1):3;
    if(r.rival.gear>=pulseGear&&r.rival.window==14)overdrive(r.rival,r.rivalBuild);
    uint8_t boostGear=longRace?4:r.opponent%6==2?2:r.opponent%6==3?4:3;
    const bool aiNos=r.rival.gear>=boostGear&&!r.rival.shiftLeft&&
        !r.rival.pulseLeft&&r.rival.rpm>=4700;
    stepCar(p,r.playerBuild,r.length,in.bHeld);
    stepCar(r.rival,r.rivalBuild,r.length,aiNos);
    ++r.elapsed;
    if(p.finishTime || r.elapsed>=3600) {
        r.phase=RESULTS;r.dnf=!p.finishTime;
        r.won=!r.dnf&&(!r.rival.finishTime || p.finishTime<r.rival.finishTime);
    }
}
bool settle(Race &r,Progress &p) {
    if(r.phase!=RESULTS||r.settled)return false;
    r.settled=true;
    if(r.dnf)return true;
    r.firstWin=r.won&&!hasWon(p,r.opponent);
    r.basePay=r.won?160+r.opponent*20:70+r.opponent*15;
    r.skillPay=r.player.perfects*30+r.player.goods*12+r.player.pulses*25+
        (r.playerBuild.manual&&!r.player.falseStart?(r.player.launchGrade==PERFECT?60:
        r.player.launchGrade==GOOD?25:0):0);
    r.firstPay=r.firstWin?220+r.opponent*15:0;
    r.payout=r.basePay+r.skillPay+r.firstPay;
    p.cash=(uint32_t)clamp(p.cash+r.payout,0,999999);
    r.record=!p.best[r.opponent]||r.player.finishTime<p.best[r.opponent];
    if(r.record)p.best[r.opponent]=r.player.finishTime;
    if(r.firstWin && r.opponent==p.wins) {
        ++p.wins;p.unlocked=p.wins<RIVALS?p.wins:RIVALS-1;p.champion=p.wins==RIVALS;
        if(p.wins%PER_STAGE==0) r.award=(Award)(p.wins/PER_STAGE);
    }
    return true;
}
uint16_t mph(const Car &c) { return (uint16_t)(c.speed*100/44704); }
}
