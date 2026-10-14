#include "../../Game.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
using namespace vr;
static int fails=0, checks=0;
#define CHECK(x) do { ++checks; if (!(x)) { ++fails; printf("FAIL line %d: %s\n",__LINE__,#x); } } while(0)
static Race drive(Progress p, int n, int rpm=IDEAL, bool nos=true) {
    Race r; start(r,p,(uint8_t)n);
    for (int i=0;i<4000 && r.phase!=RESULTS;++i) {
        bool use=nos && r.player.gear==3 && !r.player.shiftLeft && r.player.rpm>=5000;
        tick(r,r.player.rpm>=rpm,use);
    }
    return r;
}
int main() {
    Progress p; reset(p); CHECK(valid(p)); CHECK(sizeof(p)<=244);
    CHECK(build(p).gears==3); CHECK(!buy(p,ENGINE));
    p.cash=999999; CHECK(!buy(p,FOURTH)); CHECK(!buy(p,NOS)); CHECK(!buy(p,FIFTH));
    for(int u=0;u<3;++u) {
        for(int i=0;i<limit((Upgrade)u);++i) CHECK(buy(p,(Upgrade)u));
        uint32_t old=p.cash; CHECK(!buy(p,(Upgrade)u)); CHECK(p.cash==old);
    }
    Progress starter; reset(starter);
    Race a=drive(starter,0), b=drive(starter,0);
    CHECK(a.phase==RESULTS && a.won); CHECK(a.player.finishTime==b.player.finishTime);
    CHECK(a.rival.finishTime==b.rival.finishTime);
    Race early=drive(starter,0,4000), late=drive(starter,0,8100), unshifted=drive(starter,0,9000);
    CHECK(early.player.finishTime>a.player.finishTime);
    CHECK(late.player.finishTime>a.player.finishTime);
    CHECK(unshifted.player.finishTime>a.player.finishTime);
    Race frozen; start(frozen,starter,0); frozen.paused=true;
    Race copy=frozen; tick(frozen,true,true); CHECK(!memcmp(&frozen,&copy,sizeof(copy)));
    frozen.paused=false; tick(frozen,true,true);
    CHECK(frozen.player.gear==1 && !frozen.player.boostUsed && frozen.player.distance==0);
    CHECK(settle(a,starter)); CHECK(valid(starter)); CHECK(starter.unlocked==1);
    CHECK(a.firstWin && a.firstPay && a.payout);
    CHECK(a.player.perfects==2 && a.payout==440);
    // Each starter path is affordable from the first perfect win.
    for(int u=0;u<3;++u) {
        Progress firstPurchase=starter;
        CHECK(buy(firstPurchase,(Upgrade)u));
        CHECK(firstPurchase.levels[u]==1 && valid(firstPurchase));
    }
    uint32_t cash=starter.cash; CHECK(!settle(a,starter)); CHECK(starter.cash==cash);
    Race again=drive(starter,0); CHECK(settle(again,starter)); CHECK(!again.firstWin && !again.firstPay);
    Race loss=drive(starter,1,4000); CHECK(!loss.won); CHECK(settle(loss,starter));
    CHECK(loss.payout>0 && !loss.firstWin && starter.unlocked==1);
    Car c; resetCar(c); Build big={5,3,0,5,true};
    stepCar(c,big); CHECK(c.slipping);
    c.gear=3; c.rpm=7300; CHECK(shift(c,big)==PERFECT);
    CHECK(boost(c,big)); CHECK(!boost(c,big)); CHECK(c.boostLeft==120);
    stepCar(c,big); CHECK(c.boostLeft==119 && c.shiftLeft>0);
    for(int i=0;i<119;++i) stepCar(c,big); CHECK(c.boostLeft==0 && !boost(c,big));
    Car gripA,gripB; resetCar(gripA); resetCar(gripB);
    Build sticky=big; sticky.tires=5;
    for(int i=0;i<100;++i){ stepCar(gripA,big);stepCar(gripB,sticky); }
    CHECK(gripB.distance>gripA.distance);
    Progress maxed; reset(maxed); maxed.unlocked=9; maxed.wonMask=511; maxed.cash=999999;
    for(int u=0;u<UPGRADE_COUNT;++u)
        for(int i=0;i<limit((Upgrade)u);++i) CHECK(buy(maxed,(Upgrade)u));
    CHECK(valid(maxed)); CHECK(build(maxed).gears==5);
    uint16_t prev=65535;
    for(int i=0;i<10;++i) {
        Race race=drive(maxed,i);
        CHECK(race.won && !race.dnf);
        // Measure each opponent's full ET independently.
        Car rival; resetCar(rival); Build rb=opponentBuild(i);
        for(int t=0;t<3600 && !rival.finishTime;++t) {
            if(rival.rpm>=opponentShift(i)) shift(rival,rb);
            if(rival.gear==3 && !rival.shiftLeft && rival.rpm>=5000) boost(rival,rb);
            stepCar(rival,rb);
        }
        CHECK(rival.finishTime && rival.finishTime<prev); prev=rival.finishTime;
        printf("Opponent %d %-12s %.2fs | max player %.2fs\n",i+1,opponentName(i),rival.finishTime/60.0,race.player.finishTime/60.0);
        settle(race,maxed);
    }
    CHECK(maxed.champion && maxed.wonMask==1023 && valid(maxed));
    Progress broken=maxed; broken.levels[ENGINE]=255; CHECK(!valid(broken));
    broken=maxed; broken.unlocked=255; CHECK(!valid(broken));
    broken=maxed; broken.cash=1000000; CHECK(!valid(broken));
    Race timed; start(timed,maxed,9); timed.phase=RESULTS; timed.dnf=true;
    cash=maxed.cash; CHECK(settle(timed,maxed)); CHECK(maxed.cash==cash && !timed.payout);

    // A rational driver farms the most lucrative beaten rival when blocked,
    // and buys affordable upgrades that improve ET. This catches economy
    // softlocks and measures progression without granting money or unlocks.
    Progress campaign; reset(campaign);
    int races=0, purchases=0, replays=0, replayGap=0, maxReplayGap=0; uint32_t seconds=0;
    while(!campaign.champion && races<500) {
        bool purchased=true;
        while(purchased) {
            purchased=false;
            Race baseline=drive(campaign,campaign.unlocked);
            int bestUpgrade=-1; int32_t bestValue=0;
            for(int u=0;u<UPGRADE_COUNT;++u) {
                Progress candidate=campaign;
                uint32_t cost=price(candidate,(Upgrade)u);
                if(!cost || !buy(candidate,(Upgrade)u)) continue;
                Race after=drive(candidate,candidate.unlocked);
                int gain=(int)baseline.player.finishTime-after.player.finishTime;
                int32_t value=gain>0 ? gain*100000/(int32_t)cost : 0;
                if(value>bestValue) { bestValue=value; bestUpgrade=u; }
            }
            if(bestUpgrade>=0) {
                CHECK(buy(campaign,(Upgrade)bestUpgrade)); ++purchases; purchased=true;
                if(replayGap>maxReplayGap) maxReplayGap=replayGap;
                CHECK(replayGap<=2);
                replayGap=0;
            }
        }
        Race next=drive(campaign,campaign.unlocked);
        if(!next.won && campaign.unlocked) next=drive(campaign,campaign.unlocked-1);
        if(campaign.wonMask & (1u<<next.opponent)) { ++replays; ++replayGap; }
        CHECK(!next.dnf);
        CHECK(settle(next,campaign));
        CHECK(valid(campaign));
        seconds+=(next.player.finishTime+180)/60+8; // countdown + menu allowance
        ++races;
    }
    CHECK(campaign.champion);
    // Progress is measured by upgrade cadence rather than an elapsed-time floor.
    CHECK(replayGap<=2 && maxReplayGap<=2);
    CHECK(races<=20);
    printf("Replay races: %d; longest replay stretch between upgrades: %d\n", replays,maxReplayGap);
    printf("Campaign: %d races, %d purchases, %u modeled seconds (%.1f minutes)\n",
        races,purchases,seconds,seconds/60.0);
    printf("%d checks, %d failures\n",checks,fails);
    return fails ? 1 : 0;
}
