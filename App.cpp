#include <CHGame.h>
#include "config.h"
#include "App.h"
#include "View.h"
#include "Sounds.h"
#if CHGAME_DEBUG
#include "Pilot.h"
#include <stdio.h>
#endif
namespace app {
vr::Progress progress;
vr::Race race;
Screen screen=HOME;
uint8_t cursor=0,selected=0;
bool soundOn=true,saveFailed=false;
const char *message="";
uint8_t messageTicks=0;
static bool dirty=false;
static const uint32_t MAGIC=save::magic("VRM1");
static uint8_t garageBack=LIST,helpPage=0;
#if CHGAME_DEBUG
static uint8_t autoDrive=0,previewId=0;
static int32_t previewDistance=0;
static bool legacyPending=false;
static uint8_t legacySeedWins=10;
#endif
static const char *const upgrades[]={"ENGINE","GEARBOX","TIRES","LAUNCH","NOS FUEL","NOS POWER","OD POWER","OD CHARGES"};
static const char *const effects[]={"MORE POWER / FASTER REVS","SHORTER SHIFT DELAY","MORE GRIP / LESS SPIN",
    "WIDER LAUNCH RPM ZONE","MORE HOLD-TO-USE FUEL","STRONGER NOS BOOST","STRONGER / LONGER PULSE","ONE MORE PULSE PER RACE"};
static void go(Screen s,uint8_t c=0) { screen=s;cursor=c;chgame.clearButtonState(); }
static void toast(const char *s) { message=s;messageTicks=90; }
static bool hasArchive() {
    if(progress.legacyWins)return true;
    for(uint8_t i=0;i<10;++i)if(progress.legacyBest[i])return true;
    return false;
}
static void loadProgress() {
    vr::reset(progress);
    if(!save::load(MAGIC,vr::SAVE_VERSION,progress)||!vr::valid(progress)) {
        vr::LegacyProgress old={};
        vr::reset(progress);
        if(save::load(MAGIC,1,old)&&vr::migrate(old,progress))dirty=true;
    }
    selected=progress.unlocked;
    go(progress.importNotice?MIGRATION:HOME);
}
static void startRace() { vr::start(race,progress,selected);go(RACING);audio::sfx(0); }
static void navigation(uint8_t count) {
    if(chgame.repeat(UP_BUTTON)){cursor=(cursor+count-1)%count;audio::sfx(0);}
    if(chgame.repeat(DOWN_BUTTON)){cursor=(cursor+1)%count;audio::sfx(0);}
}
#if CHGAME_DEBUG
static bool hook(char cmd,const char *args) {
    if(cmd=='V') {
        char b[512];
        snprintf(b,sizeof(b),"VROOM screen=%u cash=%lu unlocked=%u wins=%u champion=%u gear=%u rpm=%u distance=%ld time=%u phase=%u paused=%u payout=%lu nos=%u engine=%u box=%u tires=%u gears=%u saved=%u stage=%u countdown=%u launched=%u launchrpm=%u false=%u charges=%u window=%u pulses=%u misses=%u boosting=%u award=%u legacy=%u oldbest=%u\n",
            (unsigned)screen,(unsigned long)progress.cash,(unsigned)progress.unlocked,(unsigned)progress.wins,
            (unsigned)progress.champion,(unsigned)race.player.gear,(unsigned)race.player.rpm,
            (long)race.player.distance,(unsigned)race.elapsed,(unsigned)race.phase,
            (unsigned)race.paused,(unsigned long)race.payout,(unsigned)race.player.fuel,
            (unsigned)progress.levels[vr::ENGINE],(unsigned)progress.levels[vr::GEARBOX],
            (unsigned)progress.levels[vr::TIRES],(unsigned)vr::build(progress).gears,
            (unsigned)(!dirty&&!saveFailed),(unsigned)(vr::stageOf(progress.unlocked)+1),
            (unsigned)race.countdown,(unsigned)race.player.launched,(unsigned)race.player.stagingRpm,
            (unsigned)race.player.falseStart,(unsigned)race.player.charges,(unsigned)race.player.window,
            (unsigned)race.player.pulses,(unsigned)race.player.pulseMisses,(unsigned)race.player.boosting,
            (unsigned)race.award,(unsigned)progress.legacyWins,(unsigned)progress.legacyBest[0]);
        dbg::print(b);return true;
    }
    if(cmd=='I') {
        uint32_t n=dbg::parseNum(args,10);if(n>=look::OPPONENT_LOOKS)return false;
        previewId=(uint8_t)n;go(CAR_PREVIEW);return true;
    }
    if(cmd=='C') {
        uint32_t n=dbg::parseNum(args,10);if(n>10000000)return false;
        previewDistance=(int32_t)n;go(SKY_PREVIEW);return true;
    }
    if(cmd=='O') { autoDrive=(uint8_t)dbg::parseNum(args,10);return autoDrive<=2; }
    if(cmd=='Z') {vr::reset(progress);dirty=true;selected=0;go(HOME);return true;}
    if(cmd=='E') {
        uint32_t n=(*args)?dbg::parseNum(args,10):vr::RIVALS-1;
        if(n>=vr::RIVALS)return false;
        vr::reset(progress);progress.cash=99999;progress.wins=(uint8_t)n;progress.unlocked=(uint8_t)n;
        for(uint8_t i=0;i<vr::UPGRADE_COUNT;++i)progress.levels[i]=vr::availableLevel(progress,(vr::Upgrade)i);
        dirty=true;selected=(uint8_t)n;go(LIST,selected);return true;
    }
    if(cmd=='U') {
        vr::Progress loaded={};
        if(dirty||!save::load(MAGIC,vr::SAVE_VERSION,loaded)||!vr::valid(loaded))return false;
        loadProgress();return true;
    }
    if(cmd=='J') {
        uint32_t n=(*args)?dbg::parseNum(args,10):10;
        if(n>10)return false;
        legacySeedWins=(uint8_t)n;legacyPending=true;return true;
    }
    return false;
}
#endif
void begin() {
    loadProgress();audio::begin(SOUNDS,(uint8_t)Sfx::COUNT);
#if CHGAME_DEBUG
    dbg::hook=hook;
#endif
}
void savePending() {
#if CHGAME_DEBUG
    if(legacyPending) {
        legacyPending=false;
        vr::LegacyProgress old={};
        old.cash=1234;old.wonMask=(uint16_t)((1u<<legacySeedWins)-1u);
        old.unlocked=legacySeedWins<10?legacySeedWins:9;old.champion=legacySeedWins==10;
        const uint8_t levels[]={5,3,5,1,1,1};
        if(legacySeedWins>=6)for(uint8_t i=0;i<6;++i)old.levels[i]=levels[i];
        for(uint8_t i=0;i<=old.unlocked;++i)old.best[i]=1200+i;
        // Both pages represent an actual old installation before reloading.
        saveFailed=!save::store(MAGIC,1,old)||!save::store(MAGIC,1,old);
        if(!saveFailed)loadProgress();
        return;
    }
#endif
    if(!dirty)return;
    saveFailed=!save::store(MAGIC,vr::SAVE_VERSION,progress);dirty=false;
}
void update() {
    if(messageTicks)--messageTicks;
    if(screen==RACING) {
        if(chgame.justPressed(START_BUTTON)){race.paused=!race.paused;audio::sfx(0);return;}
        const bool held=chgame.pressed(A_BUTTON);
        vr::Input input={held,chgame.justPressed(A_BUTTON),
            chgame.justReleased(A_BUTTON)||(race.player.heldA&&!held),
            chgame.pressed(B_BUTTON),chgame.justPressed(UP_BUTTON)};
#if CHGAME_DEBUG
        if(autoDrive)input=pilot::input(race,autoDrive==2?pilot::good():pilot::perfect());
#endif
        uint8_t oldGear=race.player.gear,oldCharges=race.player.charges;
        const bool oldFalse=race.player.falseStart;
        uint16_t oldCount=race.countdown;
        vr::tick(race,input);
        if(race.player.gear!=oldGear)audio::sfx(Sfx::Shift);
        if(!oldFalse&&race.player.falseStart)audio::sfx(Sfx::Lose);
        if(race.player.charges<oldCharges)audio::sfx(race.player.pulseQuality?Sfx::Buy:Sfx::Lose);
        if(!race.paused&&oldCount&&oldCount!=race.countdown&&oldCount%60==0)audio::sfx(Sfx::Click);
        if(!race.paused&&oldCount&&race.countdown==0)audio::blip(2300,80);
        if(!race.paused&&race.phase==vr::RUNNING&&!audio::playing()&&chgame.everyXFrames(8))
            audio::blip(200+race.player.rpm/10,30,true);
        if(race.phase==vr::RESULTS) {
            if(vr::settle(race,progress))dirty=true;
            audio::sfx(race.won?Sfx::Win:Sfx::Lose);go(RESULT,race.won?0:1);
        }
        return;
    }
#if CHGAME_DEBUG
    if(screen==CAR_PREVIEW||screen==SKY_PREVIEW)return;
#endif
    if(screen==HOME) {
        navigation(5);
        if(chgame.justPressed(A_BUTTON)) {
            if(cursor==0)go(LIST,selected);
            else if(cursor==1){garageBack=HOME;go(GARAGE);}
            else if(cursor==2){helpPage=0;go(HELP);}
            else if(cursor==3){soundOn=!soundOn;audio::setOn(soundOn);}
            else go(RESET_CONFIRM);
        }
    } else if(screen==LIST) {
        navigation(vr::RIVALS);
        if(chgame.justPressed(LEFT_BUTTON))cursor=(cursor+vr::RIVALS-vr::PER_STAGE)%vr::RIVALS;
        if(chgame.justPressed(RIGHT_BUTTON))cursor=(cursor+vr::PER_STAGE)%vr::RIVALS;
        selected=cursor;
        if(chgame.justPressed(B_BUTTON))go(HOME);
        else if(chgame.justPressed(SELECT_BUTTON)){garageBack=LIST;go(GARAGE);}
        else if(chgame.justPressed(A_BUTTON)) {
            if(selected<=progress.unlocked)startRace();else toast("BEAT THE PREVIOUS RIVAL");
        }
    } else if(screen==GARAGE) {
        navigation(vr::UPGRADE_COUNT);
        if(chgame.justPressed(B_BUTTON))go((Screen)garageBack,garageBack==LIST?selected:0);
        else if(chgame.justPressed(SELECT_BUTTON))go(LIST,selected);
        else if(chgame.justPressed(A_BUTTON)) {
            vr::Upgrade u=(vr::Upgrade)cursor;
            if(vr::buy(progress,u)){dirty=true;toast("UPGRADE INSTALLED");audio::sfx(Sfx::Buy);}
            else if(!vr::price(progress,u))toast("ALREADY MAXED");
            else if(!vr::unlocked(progress,u))toast("WIN MORE RIVALS TO UNLOCK");
            else toast("NEED MORE CASH");
        }
    } else if(screen==RESULT) {
        navigation(4);
        if(chgame.justPressed(A_BUTTON)) {
            if(cursor==0) {
                if(race.award!=vr::NO_AWARD)go(race.award==vr::TITLE_AWARD?CHAMPION:MILESTONE);
                else if(selected<progress.unlocked){++selected;startRace();}
                else if(progress.champion&&selected==vr::RIVALS-1)go(CHAMPION);
                else toast("WIN TO UNLOCK NEXT RIVAL");
            } else if(cursor==1)startRace();
            else if(cursor==2){garageBack=RESULT;go(GARAGE);}
            else go(LIST,selected);
        } else if(chgame.justPressed(B_BUTTON))go(LIST,selected);
    } else if(screen==MILESTONE) {
        if(chgame.justPressed(A_BUTTON)||chgame.justPressed(B_BUTTON)){
            selected=progress.unlocked;go(LIST,selected);
        }
    } else if(screen==CHAMPION) {
        if(chgame.justPressed(A_BUTTON)||chgame.justPressed(B_BUTTON))go(LIST,selected);
    } else if(screen==MIGRATION) {
        if(chgame.justPressed(A_BUTTON)){progress.importNotice=0;dirty=true;go(HOME);}
    } else if(screen==HELP) {
        const uint8_t pages=hasArchive()?4:3;
        if(chgame.justPressed(RIGHT_BUTTON))helpPage=(helpPage+1)%pages;
        if(chgame.justPressed(LEFT_BUTTON))helpPage=(helpPage+pages-1)%pages;
        if(chgame.justPressed(A_BUTTON)||chgame.justPressed(B_BUTTON))go(HOME);
    } else if(screen==RESET_CONFIRM) {
        if(chgame.justPressed(B_BUTTON))go(HOME,4);
        else if(chgame.justPressed(A_BUTTON)){
            vr::reset(progress);selected=0;dirty=true;go(HOME);toast("NEW CAMPAIGN STARTED");
        }
    }
}
static void milestone() {
    using namespace view;
    header("STAGE CLEAR",progress.cash);
    const vr::Build b=vr::build(progress);
    const char *title=race.award==vr::MANUAL_AWARD?"MANUAL LAUNCH":
        race.award==vr::NOS_AWARD?"NOS UNLOCKED":race.award==vr::OVERDRIVE_AWARD?
        "OVERDRIVE UNLOCKED":race.award==vr::GEAR_AWARD?"DISTANCE SERIES":"FINAL UPGRADES";
    center(30,title,GOLD);
    const char *one=race.award==vr::MANUAL_AWARD?"HOLD A AT THE LAST 1":
        race.award==vr::NOS_AWARD?"HOLD B TO SPEND FUEL":race.award==vr::OVERDRIVE_AWARD?
        "CLEAN SHIFT OPENS A WINDOW":race.award==vr::GEAR_AWARD?"SPRINTS AND HALF-MILES":"FINAL LEVELS OPEN AS YOU WIN";
    const char *two=race.award==vr::MANUAL_AWARD?"RELEASE AT GREEN AT 4200 RPM":
        race.award==vr::NOS_AWARD?"RELEASE TO SAVE FOR LATER":race.award==vr::OVERDRIVE_AWARD?
        "TAP UP WHEN IT TURNS GOLD":"COMBINE LAUNCH, NOS AND PULSES";
    center(48,one,WHITE);center(60,two,CYAN);
    center(82,"GEARS NOW",SILVER);number(86,82,b.gears,GOLD);
    center(98,"NEW GARAGE LEVELS AVAILABLE",SILVER);
    footer("A CONTINUE");
}
void render() {
    using namespace view;
#if CHGAME_DEBUG
    if(screen==CAR_PREVIEW){appearancePreview(previewId);return;}
    if(screen==SKY_PREVIEW){city(previewDistance);return;}
#endif
    if(screen==RACING){raceView(race);return;}
    if(screen==HOME) {
        gfx_clear(INK);title();
        const char *rows[5]={"RACE","GARAGE","DRIVER GUIDE",soundOn?"SOUND ON":"SOUND OFF","RESET PROGRESS"};
        for(int i=0;i<5;++i)row(76+i*8,rows[i],cursor==i,SILVER);
        cash(7,65,progress.cash,GOLD);
        if(progress.champion)text(88,65,"CHAMPION",CYAN);
        else {text(86,65,"STAGE",CYAN);number(110,65,vr::stageOf(progress.unlocked)+1,WHITE);text(116,65,"/6",SILVER);}
        footer("UP/DOWN A SELECT");
    } else if(screen==LIST) {
        header(vr::stageName(vr::stageOf(selected)),progress.cash);
        const look::Style style=look::opponent(selected);
        car(34,25,style,false,false,false,0,selected+1);
        number(9,37,selected+1,CYAN);
        if(selected>progress.unlocked)text(103,37,"LOCK",BLUE);
        text(8,55,vr::distanceName(selected),CYAN);
        text(60,55,"BEST",SILVER);time(80,55,progress.best[selected],GOLD);
        int first=cursor>2?cursor-2:0;if(first>vr::RIVALS-4)first=vr::RIVALS-4;
        for(int i=first;i<first+4;++i) {
            int y=65+(i-first)*12;
            if(cursor==i){gfx_fillRect(4,y-3,120,11,NAVY);gfx_fillRect(4,y-3,2,11,CYAN);}
            number(8,y,i+1,i<=progress.unlocked?CYAN:BLUE);
            text(21,y,vr::opponentName(i),i<=progress.unlocked?WHITE:BLUE);
            if(i%6==5)text(83,y,"BOSS",GOLD);
            if(i>progress.unlocked)text(106,y,"LOCK",BLUE);
            else if(vr::hasWon(progress,i))text(112,y,"OK",GOLD);
        }
        center(109,vr::opponentTrait(selected),CYAN);
        if(messageTicks){gfx_fillRect(0,52,128,10,INK);center(55,message,RED);}
        footer("A RACE B BACK SEL GARAGE");
    } else if(screen==GARAGE) {
        header("GARAGE",progress.cash);
        int first=cursor>4?cursor-4:0;if(first>2)first=2;
        for(int i=first;i<first+6;++i) {
            int y=27+(i-first)*10;
            if(cursor==i){gfx_fillRect(4,y-2,120,9,NAVY);gfx_fillRect(4,y-2,2,9,CYAN);}
            text(9,y,upgrades[i],cursor==i?WHITE:SILVER);
            number(57,y,progress.levels[i],CYAN);text(62,y,"/",BLUE);
            number(67,y,vr::limit((vr::Upgrade)i),BLUE);
            uint32_t cost=vr::price(progress,(vr::Upgrade)i);
            if(!cost)text(94,y,"MAX",GOLD);
            else if(!vr::unlocked(progress,(vr::Upgrade)i))text(94,y,"LOCK",BLUE);
            else cash(83,y,cost,progress.cash>=cost?GOLD:RED);
        }
        text(8,89,effects[cursor],CYAN);
        if(messageTicks)center(102,message,GOLD);
        else if(vr::price(progress,(vr::Upgrade)cursor)&&!vr::unlocked(progress,(vr::Upgrade)cursor)){
            text(8,102,"UNLOCK AT WINS",SILVER);
            number(72,102,vr::unlockWins((vr::Upgrade)cursor,progress.levels[cursor]+1),GOLD);
        } else text(8,102,"PERMANENT / AUTO SAVED",SILVER);
        footer("A BUY B BACK SEL RIVALS");
    } else if(screen==RESULT) {
        header(race.dnf?"DID NOT FINISH":race.won?"YOU WIN":"RIVAL WINS",progress.cash);
        text(8,26,"YOUR TIME",SILVER);time(76,26,race.player.finishTime,WHITE);
        text(8,35,"RIVAL TIME",SILVER);
        if(race.rival.finishTime)time(76,35,race.rival.finishTime,RED);else text(76,35,"BEHIND",RED);
        text(8,46,"FINISH",SILVER);cash(40,46,race.basePay,GOLD);
        text(75,46,"SKILL",SILVER);cash(99,46,race.skillPay,CYAN);
        text(8,55,"FIRST WIN",SILVER);cash(52,55,race.firstPay,GOLD);
        if(race.record)text(88,55,"NEW BEST",CYAN);
        text(8,64,"EARNED",WHITE);cash(40,64,race.payout,GOLD);
        const char *rows[4]={race.award!=vr::NO_AWARD?"STAGE REWARD":
            selected<progress.unlocked?"NEXT RACE":progress.champion&&selected==35?
            "CHAMPIONSHIP":"NEXT RACE [LOCKED]","REMATCH","GARAGE","RIVAL LIST"};
        for(int i=0;i<4;++i)row(78+i*9,rows[i],cursor==i,SILVER);
        footer(messageTicks?message:"A SELECT B RIVAL LIST");
    } else if(screen==MILESTONE)milestone();
    else if(screen==MIGRATION) {
        header("MVP SAVE IMPORTED",progress.cash);
        center(31,"CASH AND MAIN UPGRADES KEPT",WHITE);
        center(44,"GEAR / NOS COSTS CREDITED",GOLD);
        center(57,"OLD TIMES IN DRIVER GUIDE",CYAN);
        center(73,"NEW SKILLS UNLOCK BY STAGE",SILVER);
        text(22,92,"RIVAL WINS KEPT:",WHITE);number(86,92,progress.legacyWins,GOLD);
        footer("A CONTINUE / AUTO SAVE");
    } else if(screen==CHAMPION) {
        gfx_clear(INK);city(0);gfx_fillRect(0,36,128,92,INK);
        text35x2s(20,9,"CHAMPION",GOLD);center(28,"ALL 36 RIVALS BEATEN",CYAN);
        playerCar(34,42);center(77,"THE STREET IS YOURS",WHITE);
        center(91,"KEEP BUILDING. BEAT YOUR BEST.",SILVER);footer("A CONTINUE");
    } else if(screen==HELP) {
        header("DRIVER GUIDE",progress.cash);
        if(helpPage==0) {
            const char *lines[]={"A: SHIFT UP IN THE GOLD","LAUNCH UNLOCKS AFTER WIN 6","HOLD A WHEN THE LIGHT SAYS 1","RELEASE AT GREEN AT 4200 RPM",
                "EARLY RELEASE: +0.5 SECOND","HIGH REVS: SPIN / LOW: BOG","LAUNCH UPGRADES WIDEN ZONE","START PAUSES / RESUMES"};
            for(int i=0;i<8;++i)text(6,27+i*10,lines[i],i==3?GOLD:SILVER);
        } else if(helpPage==1) {
            const char *lines[]={"NOS UNLOCKS AFTER WIN 12","HOLD B TO USE / RELEASE SAVE","FUEL REFILLS EVERY RACE","USING NOS IN SHIFTS WASTES IT",
                "OVERDRIVE AFTER WIN 18","A CLEAN SHIFT OPENS A WINDOW","TAP UP WHEN THE METER IS GOLD","BAD TIMING SPENDS A CHARGE"};
            for(int i=0;i<8;++i)text(6,27+i*10,lines[i],i==6?GOLD:SILVER);
        } else if(helpPage==2) {
            const char *lines[]={"SIX STAGES / 36 RIVALS","LEFT/RIGHT: JUMP RIVAL STAGE","1/8, 1/4 AND 1/2 MILE RACES","EIGHT PERMANENT UPGRADE PATHS",
                "ABILITY BASES ARE FREE REWARDS","FINISHES PAY / WINS PAY MORE","REPLAY RIVALS FOR CASH / BESTS","ALL PROGRESS SAVES AUTOMATICALLY"};
            for(int i=0;i<8;++i)text(6,27+i*10,lines[i],SILVER);
        } else {
            text(8,26,"ARCHIVED MVP TIMES / 1/4 MI",CYAN);
            for(int i=0;i<10;++i){int x=i<5?8:72,y=41+(i%5)*12;number(x,y,i+1,SILVER);time(x+12,y,progress.legacyBest[i],GOLD);}
        }
        footer("LEFT/RIGHT PAGE A/B BACK");
    } else {
        header("RESET PROGRESS?",progress.cash);center(42,"LOSE CASH, UPGRADES,",WHITE);
        center(54,"WINS AND RECORDS?",WHITE);center(78,"A: RESET EVERYTHING",RED);
        center(92,"B: KEEP MY PROGRESS",CYAN);footer("THIS CANNOT BE UNDONE");
    }
    if(saveFailed)footer("SAVE FAILED - NOT STORED");
    else if(screen==HOME&&messageTicks)footer(message);
}
}
