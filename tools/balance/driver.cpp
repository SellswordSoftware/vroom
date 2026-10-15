#include "Game.h"
#include "Pilot.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
// Offline balance experiment; this file is never linked into firmware.
// Profiles: 0 ideal, 1 good shifts/partial boosts, 2 variable clean timing,
// 3 variable timing plus one late shift or missed boost per race.
// The first precision-shift boss uses gold shifts in every profile.
using namespace vr;
namespace balance {
int candidate=0,partial=60,delay[6]={0,0,0,0,0,0},payPreset=0,retryMode=0;
const int caps[6][8]={{2,1,2,0,0,0,0,0},{3,2,3,1,0,0,0,0},{4,2,4,2,1,1,0,0},{5,3,4,2,2,2,1,1},{6,3,5,3,4,3,2,2},{7,4,6,4,5,4,4,3}};
const int gate[8][7]={
 {0,2,7,13,19,26,31},
 {0,10,23,32},
 {0,4,9,15,25,31},
 {6,16,29,32},
 {12,21,24,27,30},
 {14,22,28,33},
 {18,27,33,35},
 {20,28,34}
};
uint8_t available(const Progress&p,Upgrade u){
 if(u>=UPGRADE_COUNT)return 0;
 if(!candidate)return caps[stageOf(p.unlocked)][u];
 int n=0;while(n<limit(u)&&p.wins>=gate[u][n])++n;return n;
}
void reward(Race&r){
 if(!candidate)return;
 int s=stageOf(r.opponent),pos=r.opponent%6;
 // First win total (excluding skill): fixed stage base plus local-rival steps.
 static const int fresh[3][6]={
  {380,420,570,755,1335,1920},
  {380,380,530,715,1335,2320},
  {380,380,530,715,1295,2360}
 };
 static const int repeat[6]={160,190,280,405,665,945};
 r.basePay=r.won?repeat[s]+pos*10:70+s*25;
 if(!r.won)r.skillPay=std::min(r.skillPay,100u);
 r.firstPay=r.firstWin?fresh[payPreset][s]+pos*10-r.basePay:0;
}
}
unsigned hash(unsigned x){x^=x>>16;x*=0x7feb352d;x^=x>>15;x*=0x846ca68b;x^=x>>16;return x;}
Race drive(const Progress&p,int n,int profile,unsigned seed){
 Race r;start(r,p,n);int pulseIndex=0;
 for(int t=0;t<5000&&r.phase!=RESULTS;++t){
  unsigned h=hash(seed*12345+r.player.gear*9323+19);
  pilot::Skill sk=pilot::perfect();
  // Keep the first precision-shift boss as a precision lesson.
  if(profile>0&&profile<4&&n!=5){
   sk.shiftRpm=profile==1?6800:6800+(h%8)*100;
   sk.launchRpm=profile==1?3850:3600+(hash(seed+123)%9)*80;
   sk.reaction=profile==1?6:8+hash(seed+34)%7;
   sk.pulse=profile==1?2:((h%3)?2:1);
   if(profile==3&&r.player.gear==2&&seed%2==0)sk.shiftRpm=7900;
  }
  if(profile==4)sk.nos=0;
  if(profile==5)sk.pulse=0;
  if(profile==6)sk.launchRpm=1800;
  Input in=pilot::input(r,sk);
  if(profile==7)in.bHeld=r.player.fuel>0;
  if(profile>=2&&profile<4&&r.playerBuild.nos){
   in.bHeld=r.player.fuel&&r.player.gear>=(r.length>FINISH?4:3)&&
    !r.player.shiftLeft&&!r.player.pulseLeft&&r.player.rpm>=4700+int(h%4)*100;
   // Occasional fuel wasted through a shift, deterministic by seed/gear.
   if(r.player.shiftLeft&&h%4==0&&r.player.gear>=4)in.bHeld=true;
  }
  if(profile==3&&seed%2==1&&pulseIndex==0&&r.playerBuild.overdrive){
   int pg=r.length>FINISH?r.playerBuild.gears-r.player.charges+1:3;
   in.upPressed=r.player.charges&&r.player.gear>=pg&&r.player.shiftLeft==2;
  }
  int charges=r.player.charges;tick(r,in);if(r.player.charges<charges)++pulseIndex;
 }
 return r;
}
struct Result{int wins,races,replays,losses,buys,maxgap,maxbulk,stuck;unsigned cash,bankmax,income,spent;};
Result campaign(int profile,int policy,unsigned seed,bool detail){
 Progress p;reset(p);Result z={};int gap=0,consecutive=0;
 int stagebuy[6]={},stagereplay[6]={},stageloss[6]={};unsigned stagecash[6]={};
 while(!p.champion&&z.races<180){
  unsigned attemptSeed=seed*1000+z.races;
  int bulk=0,s=stageOf(p.unlocked);
  while(true){
   int pick=-1;long long best=-1;
   Race baseline=drive(p,p.unlocked,profile,attemptSeed);
   for(int u=0;u<8;++u){
    Progress q=p;if(!buy(q,Upgrade(u)))continue;
    long long score=0;
    if(policy==0){
     Race r=drive(q,q.unlocked,profile,attemptSeed);
     int gain=int(baseline.player.finishTime)-int(r.player.finishTime);
     if(gain<=0)continue;
     score=1000000LL*gain/price(p,Upgrade(u));
    } else if(policy==1)score=100000-price(p,Upgrade(u));
    else {static const int priorities[8]={8,6,7,1,5,4,3,2};score=priorities[u]*100000-price(p,Upgrade(u));}
    if(score>best){pick=u;best=score;}
   }
   if(pick<0)break;
   z.spent+=price(p,Upgrade(pick));buy(p,Upgrade(pick));++z.buys;++bulk;++stagebuy[s];z.maxgap=std::max(z.maxgap,gap);gap=0;
  }
  z.maxbulk=std::max(z.maxbulk,bulk);
  // Attempt the next rival, then use a replay after a loss. No result foreknowledge.
  bool farm=balance::retryMode==0&&consecutive>0&&p.wins>0;
  int n=farm?p.wins-1:p.unlocked;
  Race r=drive(p,n,profile,attemptSeed);
  bool replay=hasWon(p,n);
  if(replay){++z.replays;++stagereplay[s];++gap;z.maxgap=std::max(z.maxgap,gap);}
  if(!r.won){++z.losses;++stageloss[s];
   if(detail)printf("LOSS profile%d policy%d rival%d cash%u levels%d,%d,%d,%d,%d,%d,%d,%d\n",profile,policy,n+1,p.cash,p.levels[0],p.levels[1],p.levels[2],p.levels[3],p.levels[4],p.levels[5],p.levels[6],p.levels[7]);
  }
  if(!replay){if(r.won)consecutive=0;else ++consecutive;}else consecutive=0;
  settle(r,p);z.income+=r.payout;++z.races;z.bankmax=std::max(z.bankmax,p.cash);
  if(p.wins%6==0&&r.firstWin)stagecash[s]=p.cash;
  if(z.races>140&&!p.champion)z.stuck=1;
 }
 z.wins=p.wins;z.cash=p.cash;
 if(detail)for(int s=0;s<6;++s)printf("STAGE profile%d policy%d s%d purchases%d replays%d losses%d cash%u\n",profile,policy,s+1,stagebuy[s],stagereplay[s],stageloss[s],stagecash[s]);
 return z;
}
int main(int argc,char**argv){
 if(argc>8)balance::retryMode=atoi(argv[8]);
 balance::candidate=argc>1?atoi(argv[1]):0;
 if(balance::candidate){
  balance::partial=80;
  balance::delay[2]=argc>2?atoi(argv[2]):0;
  balance::delay[3]=argc>3?atoi(argv[3]):6;
  balance::delay[4]=argc>4?atoi(argv[4]):7;
  balance::delay[5]=argc>5?atoi(argv[5]):0;
  balance::payPreset=argc>6?atoi(argv[6]):1;
 }

 {
  Progress p;reset(p);Race first=drive(p,0,0,1);assert(first.won);
  settle(first,p);assert(first.payout==440);
  for(int u=0;u<3;++u){Progress q=p;assert(buy(q,Upgrade(u)));}
  if(balance::candidate){
   int last[8]={},total=0,maxbatch=0;
   for(int n=0;n<=36;++n){
    reset(p);p.wins=n;p.unlocked=n<36?n:35;p.champion=n==36;
    int batch=0;
    for(int u=0;u<8;++u){
     int cap=availableLevel(p,Upgrade(u));assert(cap>=last[u]);
     batch+=cap-last[u];last[u]=cap;
     assert(cap<=balance::caps[stageOf(p.unlocked)][u]);
     if(n%6==5)assert(cap==balance::caps[n/6][u]);
    }
    total+=batch;if(n>0)maxbatch=std::max(maxbatch,batch);
   }
   assert(total==37&&maxbatch<=2);
   unsigned catalog=0;
   for(int u=0;u<8;++u)for(int l=0;l<limit(Upgrade(u));++l){
    p.levels[u]=l;catalog+=price(p,Upgrade(u));
   }
   assert(catalog==37900);
   for(int n=0;n<36;++n){
    reset(p);p.wins=n;p.unlocked=n;Race r;start(r,p,n);
    r.phase=RESULTS;r.player.finishTime=1000;r.player.perfects=10;r.player.pulses=10;
    settle(r,p);assert(!r.won&&r.skillPay<=100&&r.payout<=295);
   }
   printf("INVARIANTS first440 starterChoices3 purchases37 catalog37900 newUnlockBatchMax2 lossesAtMost295 passed\n");
  }
 }
 printf("CONFIG candidate%d delays %d,%d,%d,%d pay%d\n",balance::candidate,balance::delay[2],balance::delay[3],balance::delay[4],balance::delay[5],balance::payPreset);
 for(int n=5;n<36;n+=6){
  Progress p;reset(p);p.wins=n;p.unlocked=n;
  for(int u=0;u<8;++u)p.levels[u]=balance::caps[n/6][u];
  for(int profile=0;profile<4;++profile){
   int wins=0,min=100000,max=-100000;
   for(unsigned seed=1;seed<=1000;++seed){
    Race r=drive(p,n,profile,seed);
    Race a=r;while(!a.rival.finishTime&&a.elapsed<3600){a.phase=RUNNING;tick(a,{});}
    int margin=int(a.rival.finishTime)-int(r.player.finishTime);
    wins+=r.won;min=std::min(min,margin);max=std::max(max,margin);
   }
   printf("BOSS %d profile%d win%d/1000 margin %.3f..%.3f\n",n+1,profile,wins,min/60.,max/60.);
  }
 }

 for(int n=0;n<36;++n)if(n%6!=5){
  Progress p;reset(p);p.wins=n;p.unlocked=n;
  for(int u=0;u<8;++u)p.levels[u]=availableLevel(p,Upgrade(u));
  for(int profile=0;profile<4;++profile){
   int wins=0;for(int seed=1;seed<=200;++seed)wins+=drive(p,n,profile,seed).won;
   if(wins<200)printf("REGULAR %d profile%d win%d/200 levels%d,%d,%d,%d,%d,%d,%d,%d\n",n+1,profile,wins,p.levels[0],p.levels[1],p.levels[2],p.levels[3],p.levels[4],p.levels[5],p.levels[6],p.levels[7]);
  }
 }

 // Fixed-cap ability ablation: skipping an entire ability should still cost time.
 for(int n=11;n<36;n+=6){
  Progress p;reset(p);p.wins=n;p.unlocked=n;
  for(int u=0;u<8;++u)p.levels[u]=balance::caps[n/6][u];
  for(int profile=4;profile<8;++profile){
   Race r=drive(p,n,profile,1);
   Race a=r;while(!a.rival.finishTime&&a.elapsed<3600){a.phase=RUNNING;tick(a,{});}
   printf("ABLATE boss%d profile%d win%d time%.3f margin%+.3f\n",n+1,profile,r.won,r.player.finishTime/60.,(int(a.rival.finishTime)-int(r.player.finishTime))/60.);
  }
 }

 for(int stage=0;stage<6;++stage){
  unsigned income=0,cost=0;
  Progress p;reset(p);
  for(int u=0;u<8;++u)for(int l=stage?balance::caps[stage-1][u]:0;l<balance::caps[stage][u];++l){
   p.levels[u]=l;cost+=price(p,Upgrade(u));
  }
  for(int n=stage*6;n<stage*6+6;++n){
   p.wins=n;p.unlocked=n;
   for(int u=0;u<8;++u)p.levels[u]=balance::caps[stage][u];
   Race r=drive(p,n,0,1);settle(r,p);income+=r.payout;
  }
  printf("INCOME stage%d idealCapFirstWins%u newCatalogCost%u\n",stage+1,income,cost);
 }
 int seeds=argc>7?atoi(argv[7]):5;
 for(int profile=0;profile<4;++profile)for(int policy=0;policy<3;++policy){
  int minwins=36,completed=0,minr=999,maxr=0,maxgap=0,losses=0,replays=0,buys=0,bulk=0;
  unsigned cash=0,peak=0;
  for(int seed=1;seed<=seeds;++seed){
   Result r=campaign(profile,policy,seed,seeds==1);if(balance::candidate){assert(r.wins==36);assert(r.maxgap<=2);assert(r.maxbulk<=3);}completed+=r.wins==36;minwins=std::min(minwins,r.wins);
   minr=std::min(minr,r.races);maxr=std::max(maxr,r.races);maxgap=std::max(maxgap,r.maxgap);losses+=r.losses;replays+=r.replays;buys+=r.buys;cash+=r.cash;peak=std::max(peak,r.bankmax);bulk=std::max(bulk,r.maxbulk);
  }
  printf("CAM profile%d policy%d complete%d/%d minWins%d races%d..%d replayAvg%.1f lossAvg%.1f buysAvg%.1f maxGap%d maxBulk%d cashAvg%u peak%u\n",profile,policy,completed,seeds,minwins,minr,maxr,replays/double(seeds),losses/double(seeds),buys/double(seeds),maxgap,bulk,cash/seeds,peak);
 }
 printf("VERIFY %s assertions passed\n",balance::candidate?"candidate":"baseline");
}
