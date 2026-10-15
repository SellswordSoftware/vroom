#include "../../Game.h"
#include "../../Pilot.h"
#include <stdio.h>
#include <string.h>
using namespace vr;
static int checks=0,failures=0;
#define CHECK(x) do{++checks;if(!(x)){++failures;printf("FAIL line %d: %s\n",__LINE__,#x);}}while(0)
static Progress at(int wins,bool cap=false) {
    Progress p;reset(p);p.wins=wins;p.unlocked=wins<RIVALS?wins:RIVALS-1;p.champion=wins==RIVALS;
    if(cap)for(int u=0;u<UPGRADE_COUNT;++u)p.levels[u]=availableLevel(p,(Upgrade)u);
    return p;
}
static Race drive(Progress p,int n,pilot::Skill skill=pilot::perfect(),int *tail=nullptr) {
    Race r;start(r,p,n);int lastAction=0;
    for(int i=0;i<4000&&r.phase!=RESULTS;++i) {
        Input in=pilot::input(r,skill);
        if(in.aPressed||in.aReleased||in.bHeld||in.upPressed||r.player.pulseLeft)lastAction=r.elapsed;
        tick(r,in);
    }
    if(tail)*tail=r.elapsed-lastAction;
    return r;
}
static void testBasics() {
    Progress p;reset(p);CHECK(valid(p));CHECK(sizeof(p)<=244);CHECK(sizeof(LegacyProgress)==36);
    int purchases=0;for(int u=0;u<UPGRADE_COUNT;++u)purchases+=limit((Upgrade)u);
    CHECK(purchases==37);CHECK(!buy(p,ENGINE));CHECK(!price(p,(Upgrade)255));
    CHECK(!unlocked(p,(Upgrade)255));
    for(int rpm=7000;rpm<=7500;rpm+=100) {
        auto gold=pilot::perfect();gold.shiftRpm=rpm;
        Race clean=drive(p,0,gold);
        CHECK(clean.player.perfects==2&&clean.won);
    }
    Race first=drive(p,0);CHECK(first.won&&!first.dnf&&first.player.perfects==2);
    CHECK(settle(first,p));CHECK(first.payout==440&&first.firstWin&&p.wins==1&&valid(p));
    for(int u=0;u<3;++u){Progress copy=p;CHECK(buy(copy,(Upgrade)u));CHECK(valid(copy));}
    uint32_t money=p.cash;CHECK(!settle(first,p));CHECK(p.cash==money);
    Race repeat=drive(p,0);settle(repeat,p);CHECK(!repeat.firstPay&&p.wins==1);
    auto bad=pilot::perfect();bad.shiftRpm=8100;
    Race lost=drive(at(1),1,bad);CHECK(!lost.won&&!lost.dnf);
    Progress loss=at(1);settle(lost,loss);CHECK(loss.cash>0&&loss.wins==1&&!lost.firstPay);
    Race dnf;start(dnf,at(6),6);
    for(int i=0;i<3800;++i)tick(dnf);
    CHECK(dnf.dnf&&dnf.phase==RESULTS);
    Progress stay=at(6);settle(dnf,stay);CHECK(!dnf.payout&&!stay.cash&&stay.wins==6);
    Progress gated=at(0);gated.cash=99999;
    CHECK(buy(gated,ENGINE));CHECK(!buy(gated,ENGINE));
    gated.wins=2;gated.unlocked=2;CHECK(buy(gated,ENGINE));CHECK(!buy(gated,ENGINE));
    CHECK(!buy(gated,LAUNCH)&&!buy(gated,NOS_CAPACITY)&&!buy(gated,OVERDRIVE_POWER));
    CHECK(build(at(5)).gears==3&&build(at(6)).gears==4);
    CHECK(build(at(12)).gears==5&&build(at(18)).gears==6&&build(at(24)).gears==7);
    CHECK(!build(at(5)).manual&&build(at(6)).manual);
    CHECK(!build(at(11)).nos&&build(at(12)).nos);
    CHECK(!build(at(17)).overdrive&&build(at(18)).overdrive);
    Progress broken=at(0);broken.unlocked=35;CHECK(!valid(broken));
    broken=at(36);broken.champion=0;CHECK(!valid(broken));
    broken=at(0);broken.levels[ENGINE]=255;CHECK(!valid(broken));
}
static void testProgression() {
    const uint8_t gates[UPGRADE_COUNT][7]={
        {0,2,7,13,19,26,31},{0,10,23,32},{0,4,9,15,25,31},{6,16,29,32},
        {12,21,24,27,30},{14,22,28,33},{18,27,33,35},{20,28,34}
    };
    int previous[UPGRADE_COUNT]={},purchases=0,lastUnlock=0;
    uint32_t catalog=0;
    for(int n=0;n<=RIVALS;++n) {
        Progress p=at(n);int batch=0;
        for(int u=0;u<UPGRADE_COUNT;++u) {
            Upgrade type=(Upgrade)u;int expected=0;
            for(int l=0;l<limit(type);++l) {
                CHECK(unlockWins(type,l+1)==gates[u][l]);
                expected+=n>=gates[u][l];
            }
            CHECK(availableLevel(p,type)==expected);
            CHECK(expected>=previous[u]);batch+=expected-previous[u];previous[u]=expected;
        }
        CHECK(batch<=(n?2:3));purchases+=batch;
        if(batch){CHECK(n-lastUnlock<=2);lastUnlock=n;}
    }
    CHECK(purchases==37);
    for(int u=0;u<UPGRADE_COUNT;++u) {
        Progress p=at(0);Upgrade type=(Upgrade)u;
        for(int l=0;l<limit(type);++l){p.levels[u]=l;catalog+=price(p,type);}
        CHECK(unlockWins(type,0)==RIVALS);
        CHECK(unlockWins(type,limit(type)+1)==RIVALS);
    }
    CHECK(catalog==37900);CHECK(unlockWins((Upgrade)255,1)==RIVALS);
    Progress saved=at(1);saved.cash=12345;saved.levels[ENGINE]=5;
    CHECK(valid(saved)&&build(saved).engine==5&&!unlocked(saved,ENGINE));
    CHECK(saved.cash==12345);
    static const uint16_t bases[STAGES]={160,190,280,405,665,945};
    static const uint16_t bonuses[STAGES]={220,190,250,310,670,1375};
    for(int n=0;n<RIVALS;++n) {
        Progress p=at(n);Race r;start(r,p,n);
        r.phase=RESULTS;r.won=true;r.player.finishTime=1000;
        CHECK(settle(r,p));CHECK(r.basePay==(uint32_t)(bases[n/6]+n%6*10));
        CHECK(r.firstPay==bonuses[n/6]);
        p=at(n);start(r,p,n);r.phase=RESULTS;r.player.finishTime=1000;
        r.player.perfects=10;r.player.pulses=10;
        CHECK(settle(r,p));CHECK(r.skillPay==100);
        CHECK(r.payout==(uint32_t)(170+n/6*25)&&r.payout<=295&&!r.firstPay);
    }
}
static void testInputs() {
    Progress p=at(6,true);Race launchRace;start(launchRace,p,6);
    while(launchRace.phase==COUNTDOWN)tick(launchRace,pilot::input(launchRace));
    CHECK(!launchRace.player.launched&&launchRace.player.stagingRpm==LAUNCH_RPM);
    tick(launchRace,pilot::input(launchRace));
    CHECK(launchRace.player.launched&&launchRace.player.launchGrade==PERFECT&&!launchRace.player.falseStart);
    CHECK(launchRace.player.gear==1);
    Race early;start(early,p,6);
    tick(early);early.player.stagingRpm=4200;early.player.launchArmed=true;early.player.heldA=true;
    Input release={};release.aReleased=true;
    tick(early,release);CHECK(early.player.falseStart&&early.player.launchDelay==30);
    while(early.phase==COUNTDOWN)tick(early);
    for(int i=0;i<30;++i)tick(early);
    CHECK(early.player.distance==0);tick(early);CHECK(early.player.distance>0);
    Race held;start(held,p,6);Input selecting={true,true,false,false,false};
    for(int i=0;i<20;++i)tick(held,selecting);
    CHECK(!held.player.launchArmed&&!held.player.launched);
    tick(held,release);CHECK(!held.player.falseStart&&held.launchInputReady);
    Race paused;start(paused,p,6);paused.paused=true;Race before=paused;
    Input all={true,true,true,true,true};tick(paused,all);
    CHECK(memcmp(&before,&paused,sizeof(paused))==0);
    Build boosted=build(at(18));
    Car car;resetCar(car,boosted);car.launched=true;car.stagingRpm=4200;
    CHECK(car.fuel==120&&car.charges==1);
    stepCar(car,boosted,FINISH,false);CHECK(car.fuel==120);
    stepCar(car,boosted,FINISH,true);CHECK(car.fuel==119&&car.boosting);
    stepCar(car,boosted,FINISH,false);CHECK(car.fuel==119&&!car.boosting);
    car.rpm=IDEAL;car.gear=2;
    CHECK(shift(car,boosted)==PERFECT);
    uint16_t fuel=car.fuel;stepCar(car,boosted,FINISH,true);
    CHECK(car.fuel==fuel-1&&car.shiftLeft>0);
    while(!car.window)stepCar(car,boosted,FINISH,false);
    CHECK(car.window==24);
    while(car.window>14)stepCar(car,boosted,FINISH,false);
    CHECK(pulsePerfect(car));CHECK(overdrive(car,boosted));
    CHECK(car.pulseQuality==100&&car.pulses==1&&car.charges==0);
    CHECK(!overdrive(car,boosted));
    Car miss;resetCar(miss,boosted);miss.launched=true;
    CHECK(overdrive(miss,boosted));CHECK(miss.pulseMisses==1&&!miss.pulseLeft&&!miss.charges);
    Car good;resetCar(good,boosted);good.launched=true;good.window=7;
    CHECK(overdrive(good,boosted));CHECK(good.pulseQuality==80&&good.pulses==1);
    Car dirty;resetCar(dirty,boosted);dirty.launched=true;dirty.rpm=4000;
    CHECK(shift(dirty,boosted)==EARLY);CHECK(!dirty.windowArmed);
    while(dirty.shiftLeft)stepCar(dirty,boosted,FINISH,false);
    CHECK(!dirty.window);
    Build big=build(at(35,true));resetCar(car,big);car.launched=true;
    for(int i=0;i<500;++i)stepCar(car,big,FINISH*2,true);
    CHECK(car.fuel==0&&car.fuelUsed==fuelCapacity(big));
    uint16_t used=car.fuelUsed;stepCar(car,big,FINISH*2,true);CHECK(car.fuelUsed==used);
}
static void testMigration() {
    LegacyProgress old={};old.cash=1234;old.wonMask=1023;old.unlocked=9;old.champion=1;
    const uint8_t levels[6]={5,3,5,1,1,1};
    memcpy(old.levels,levels,6);
    for(int i=0;i<10;++i)old.best[i]=1200+i;
    Progress p;CHECK(migrate(old,p));CHECK(valid(p));
    CHECK(p.cash==4534&&p.wins==10&&!p.champion&&p.importNotice);
    CHECK(p.levels[ENGINE]==5&&p.levels[GEARBOX]==3&&p.levels[TIRES]==5);
    CHECK(p.legacyWins==10&&p.legacyBest[9]==1209&&!p.best[9]);
    old.wonMask=5;CHECK(!migrate(old,p));
    old.wonMask=1023;old.best[0]=65000;CHECK(!migrate(old,p));
    LegacyProgress fresh={};CHECK(migrate(fresh,p));CHECK(valid(p)&&p.wins==0);
}
static void testRivals() {
    for(int n=0;n<RIVALS;++n) {
        Progress p=at(n,true);
        int tail=0;Race race=drive(p,n,pilot::perfect(),&tail);
        CHECK(race.won&&!race.dnf);
        CHECK(race.player.finishTime>300&&race.player.finishTime<1800);
        CHECK(race.length==FINISH/2||race.length==FINISH||race.length==FINISH*2);
        if(race.length>FINISH) {
            // Check the same full-stage build as before staggered purchase gates.
            Progress full=at((stageOf(n)+1)*PER_STAGE-1,true);
            full.wins=n;full.unlocked=n;
            drive(full,n,pilot::perfect(),&tail);CHECK(tail<=360);
        }
        if(n%6!=5){Race regular=drive(p,n,pilot::good());CHECK(regular.won);}
        if(n%6==5) {
            CHECK(settle(race,p)&&p.wins==n+1&&valid(p));
            CHECK(race.award==(Award)(p.wins/6));
            CHECK(build(p).nos==(p.wins>=12));
            CHECK(build(p).overdrive==(p.wins>=18));
            printf("Boss %2d %-12s %.2fs, action tail %.2fs, reward %d\n",n+1,opponentName(n),
                   race.player.finishTime/60.,tail/60.,race.award);
        }
    }
    auto noNos=pilot::perfect();noNos.nos=0;
    CHECK(!drive(at(17,true),17,noNos).won);
    auto noPulse=pilot::perfect();noPulse.pulse=0;
    Race boosted=drive(at(23,true),23),unboosted=drive(at(23,true),23,noPulse);
    CHECK(unboosted.player.finishTime>=boosted.player.finishTime+12);
    CHECK(!drive(at(35,true),35,noPulse).won);
    CHECK(!drive(at(35,true),35,noNos).won);
    for(int n=17;n<RIVALS;n+=6)CHECK(drive(at(n,true),n,pilot::good()).won);
    auto weakLaunch=pilot::perfect();weakLaunch.launchRpm=1800;
    CHECK(!drive(at(11,true),11,weakLaunch).won);
}
static void campaign(int policy,pilot::Skill skill) {
    Progress p;reset(p);
    int races=0,buys=0,replays=0,gap=0,maxgap=0,maximum=0;unsigned seconds=0;
    while(!p.champion&&races<150) {
        bool bought=true;
        while(bought) {
            bought=false;Race base=drive(p,p.unlocked,skill);
            int pick=-1,value=0;
            for(int u=0;u<UPGRADE_COUNT;++u) {
                Progress next=p;Upgrade type=(Upgrade)u;uint32_t cost=price(next,type);
                if(!buy(next,type))continue;
                Race after=drive(next,next.unlocked,skill);
                int gain=(int)base.player.finishTime-after.player.finishTime;
                int score=gain>0?gain*100000/(int)cost:0;
                if(policy==1)score=gain>0?1000000-(int)cost:0;
                if(policy==2)score=gain>0?gain*10000+(u==TIRES?50000:0):0;
                if(score>value){value=score;pick=u;}
            }
            if(pick>=0) {
                CHECK(gap<=2);if(gap>maxgap)maxgap=gap;gap=0;
                CHECK(buy(p,(Upgrade)pick));++buys;bought=true;
            }
        }
        Race next=drive(p,p.unlocked,skill);
        if(!next.won&&p.wins)next=drive(p,p.unlocked-1,skill);
        if(hasWon(p,next.opponent)){++replays;++gap;}
        CHECK(!next.dnf);CHECK(settle(next,p));CHECK(valid(p));
        seconds+=(next.player.finishTime+180)/60+8;
        if(next.player.finishTime>maximum)maximum=next.player.finishTime;
        ++races;
    }
    CHECK(p.champion&&p.wins==RIVALS);CHECK(gap<=2);CHECK(races<=RIVALS+20);
    printf("Campaign policy%d shift%u: %d races, %d replays, %d purchases, max gap %d, %.1f minutes\n",
        policy,skill.shiftRpm,races,replays,buys,maxgap,seconds/60.);
}
int main() {
    testBasics();testProgression();testInputs();testMigration();testRivals();
    for(int policy=0;policy<3;++policy)campaign(policy,pilot::perfect());
    auto competent=pilot::good();competent.pulse=1;competent.shiftRpm=6800;
    campaign(0,competent);
    printf("%d checks, %d failures\n",checks,failures);
    return failures?1:0;
}
