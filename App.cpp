#include <CHGame.h>
#include "config.h"
#include "App.h"
#include "View.h"
#include "Sounds.h"
#if CHGAME_DEBUG
#include <stdio.h>
#endif
namespace app {
vr::Progress progress;
vr::Race race;
Screen screen=HOME;
uint8_t cursor=0, selected=0;
bool soundOn=true, saveFailed=false;
const char *message="";
uint8_t messageTicks=0;
static bool dirty=false;
static const uint32_t MAGIC=save::magic("VRM1");
static uint8_t garageBack=LIST;
static bool newlyChampion=false;
#if CHGAME_DEBUG
static bool autoDrive=false;
#endif
static const char *const upgrades[]={"ENGINE","GEARBOX","TIRES","4TH GEAR","5TH GEAR","NOS"};
static const char *const effects[]={"MORE POWER / FASTER REVS","SHORTER SHIFT DELAY","MORE GRIP / LESS SPIN",
    "HIGHER TOP SPEED","HIGHER TOP SPEED","ONE 2-SECOND BOOST"};
static void go(Screen s,uint8_t c=0) { screen=s; cursor=c; chgame.clearButtonState(); }
static void toast(const char *s) { message=s; messageTicks=90; }
static void startRace() { vr::start(race,progress,selected); go(RACING); audio::sfx(0); }
static void navigation(uint8_t count) {
    if(chgame.repeat(UP_BUTTON)) { cursor=(cursor+count-1)%count; audio::sfx(0); }
    if(chgame.repeat(DOWN_BUTTON)) { cursor=(cursor+1)%count; audio::sfx(0); }
}
#if CHGAME_DEBUG
static bool hook(char cmd,const char *args) {
    if(cmd=='V') {
        char b[250];
        snprintf(b,sizeof(b),"VROOM screen=%u cash=%lu unlocked=%u wins=%u champion=%u gear=%u rpm=%u distance=%ld time=%u phase=%u paused=%u payout=%lu nos=%u engine=%u box=%u tires=%u gears=%u saved=%u\n",
            (unsigned)screen,(unsigned long)progress.cash,(unsigned)progress.unlocked,
            (unsigned)progress.wonMask,(unsigned)progress.champion,(unsigned)race.player.gear,
            (unsigned)race.player.rpm,(long)race.player.distance,(unsigned)race.elapsed,
            (unsigned)race.phase,(unsigned)race.paused,(unsigned long)race.payout,
            (unsigned)race.player.boostLeft,(unsigned)progress.levels[vr::ENGINE],
            (unsigned)progress.levels[vr::GEARBOX],(unsigned)progress.levels[vr::TIRES],
            (unsigned)vr::build(progress).gears,(unsigned)(!dirty&&!saveFailed));
        dbg::print(b); return true;
    }
    if(cmd=='O') { autoDrive=dbg::parseNum(args,10)!=0; return true; }
    if(cmd=='Z') { vr::reset(progress); dirty=true; selected=0; go(HOME); return true; }
    if(cmd=='E') {
        vr::reset(progress); progress.cash=99999; progress.unlocked=9; progress.wonMask=511;
        for(uint8_t i=0;i<vr::UPGRADE_COUNT;++i) progress.levels[i]=vr::limit((vr::Upgrade)i);
        dirty=true; selected=9; go(LIST,9); return true;
    }
    if(cmd=='U') {
        vr::Progress loaded={};
        if(dirty || !save::load(MAGIC,1,loaded) || !vr::valid(loaded)) return false;
        progress=loaded; selected=progress.unlocked; go(HOME); return true;
    }
    return false;
}
#endif
void begin() {
    vr::reset(progress);
    if(!save::load(MAGIC,1,progress) || !vr::valid(progress)) vr::reset(progress);
    selected=progress.unlocked;
    audio::begin(SOUNDS,(uint8_t)Sfx::COUNT);
#if CHGAME_DEBUG
    dbg::hook=hook;
#endif
}
void savePending() {
    if(!dirty) return;
    saveFailed=!save::store(MAGIC,1,progress);
    dirty=false; // do not repeatedly write failing flash every frame
}
void update() {
    if(messageTicks) --messageTicks;
    if(screen==RACING) {
        if(chgame.justPressed(START_BUTTON)) {
            race.paused=!race.paused; audio::sfx(0);
            return;
        }
        bool press=chgame.justPressed(A_BUTTON), boost=chgame.justPressed(B_BUTTON);
#if CHGAME_DEBUG
        if(autoDrive) {
            press=race.player.rpm>=vr::IDEAL;
            boost=race.player.gear==3 && !race.player.shiftLeft && race.player.rpm>=5000;
        }
#endif
        const uint8_t oldGear=race.player.gear;
        const uint16_t oldCount=race.countdown;
        vr::tick(race,press,boost);
        if(race.player.gear!=oldGear) audio::sfx(1);
        if(!race.paused && race.phase==vr::COUNTDOWN && oldCount/60!=race.countdown/60) audio::sfx(0);
        if(!race.paused && race.phase==vr::RUNNING && !audio::playing() && chgame.everyXFrames(8))
            audio::blip(200+race.player.rpm/10,30,true);
        if(race.phase==vr::RESULTS) {
            const bool wasChampion=progress.champion;
            if(vr::settle(race,progress)) dirty=true;
            newlyChampion=!wasChampion && progress.champion;
            audio::sfx(race.won ? 2 : 3);
            go(RESULT,race.won ? 0 : 1);
        }
        return;
    }
    if(screen==HOME) {
        navigation(5);
        if(chgame.justPressed(A_BUTTON)) {
            if(cursor==0) go(LIST,selected);
            else if(cursor==1) { garageBack=HOME; go(GARAGE); }
            else if(cursor==2) go(HELP);
            else if(cursor==3) { soundOn=!soundOn; audio::setOn(soundOn); }
            else go(RESET_CONFIRM);
        }
    } else if(screen==LIST) {
        navigation(10); selected=cursor;
        if(chgame.justPressed(B_BUTTON)) go(HOME);
        else if(chgame.justPressed(SELECT_BUTTON)) { garageBack=LIST; go(GARAGE); }
        else if(chgame.justPressed(A_BUTTON)) {
            if(selected<=progress.unlocked) startRace(); else toast("BEAT THE PREVIOUS RIVAL");
        }
    } else if(screen==GARAGE) {
        navigation(vr::UPGRADE_COUNT);
        if(chgame.justPressed(B_BUTTON)) go((Screen)garageBack,garageBack==LIST ? selected : 0);
        else if(chgame.justPressed(SELECT_BUTTON)) go(LIST,selected);
        else if(chgame.justPressed(A_BUTTON)) {
            vr::Upgrade u=(vr::Upgrade)cursor;
            if(vr::buy(progress,u)) { dirty=true; toast("UPGRADE INSTALLED"); audio::sfx(4); }
            else if(!vr::price(progress,u)) toast("ALREADY MAXED");
            else if(!vr::unlocked(progress,u)) toast(cursor==vr::FIFTH && !progress.levels[vr::FOURTH] ?
                "BUY 4TH GEAR FIRST" : "BEAT MORE RIVALS");
            else toast("NEED MORE CASH");
        }
    } else if(screen==RESULT) {
        navigation(4);
        if(chgame.justPressed(A_BUTTON)) {
            if(cursor==0) {
                if(newlyChampion) { newlyChampion=false; go(CHAMPION); }
                else if(selected<9 && selected<progress.unlocked) { ++selected; startRace(); }
                else if(selected==9 && progress.champion) go(CHAMPION);
                else toast("WIN TO UNLOCK NEXT RIVAL");
            } else if(cursor==1) startRace();
            else if(cursor==2) { garageBack=RESULT; go(GARAGE); }
            else go(LIST,selected);
        } else if(chgame.justPressed(B_BUTTON)) go(LIST,selected);
    } else if(screen==CHAMPION) {
        if(chgame.justPressed(A_BUTTON) || chgame.justPressed(B_BUTTON)) go(LIST,selected);
    } else if(screen==HELP) {
        if(chgame.justPressed(A_BUTTON) || chgame.justPressed(B_BUTTON)) go(HOME);
    } else if(screen==RESET_CONFIRM) {
        if(chgame.justPressed(B_BUTTON)) go(HOME,4);
        else if(chgame.justPressed(A_BUTTON)) {
            vr::reset(progress); selected=0; dirty=true; newlyChampion=false;
            go(HOME); toast("NEW SEASON STARTED");
        }
    }
}
void render() {
    using namespace view;
    if(screen==RACING) { raceView(race); return; }
    if(screen==HOME) {
        gfx_clear(INK); title();
        const char *rows[5]={"RACE","GARAGE","HOW TO PLAY",soundOn ? "SOUND ON" : "SOUND OFF","RESET PROGRESS"};
        for(int i=0;i<5;++i) row(76+i*8,rows[i],cursor==i,SILVER);
        cash(7,65,progress.cash,GOLD);
        text(76,65,progress.champion ? "CHAMPION" : "STREET ROOKIE",CYAN);
        footer("UP/DOWN  A SELECT");
    } else if(screen==LIST) {
        header("RIVALS",progress.cash);
        int first=cursor>4 ? cursor-4 : 0; if(first>4) first=4;
        for(int i=first;i<first+6;++i) {
            int y=27+(i-first)*12;
            if(cursor==i) { gfx_fillRect(4,y-3,120,11,NAVY); gfx_fillRect(4,y-3,2,11,CYAN); }
            number(9,y,i+1,i<=progress.unlocked ? CYAN : BLUE);
            text(21,y,vr::opponentName(i),i<=progress.unlocked ? WHITE : BLUE);
            if(i>progress.unlocked) text(106,y,"LOCK",BLUE);
            else if(progress.wonMask&(1u<<i)) text(112,y,"OK",GOLD);
        }
        text(8,103,"BEST",SILVER); time(30,103,progress.best[selected],GOLD);
        if(messageTicks) { gfx_fillRect(0,99,128,13,INK); center(104,message,RED); }
        footer("A RACE B BACK SEL GARAGE");
    } else if(screen==GARAGE) {
        header("GARAGE",progress.cash);
        for(int i=0;i<6;++i) {
            int y=27+i*10;
            if(cursor==i) { gfx_fillRect(4,y-2,120,9,NAVY); gfx_fillRect(4,y-2,2,9,CYAN); }
            text(10,y,upgrades[i],cursor==i ? WHITE : SILVER);
            number(55,y,progress.levels[i],CYAN); text(60,y,"/",BLUE);
            number(65,y,vr::limit((vr::Upgrade)i),BLUE);
            uint32_t cost=vr::price(progress,(vr::Upgrade)i);
            if(!cost) text(94,y,"MAX",GOLD);
            else if(!vr::unlocked(progress,(vr::Upgrade)i)) text(94,y,"LOCK",BLUE);
            else cash(83,y,cost,progress.cash>=cost ? GOLD : RED);
        }
        text(8,89,effects[cursor],CYAN);
        if(!vr::unlocked(progress,(vr::Upgrade)cursor))
            text(8,101,cursor==vr::FIFTH ? "NEEDS WIN 6 + 4TH GEAR" : "UNLOCKS AFTER WIN 3",SILVER);
        else if(messageTicks) center(101,message,GOLD);
        else text(8,101,"PERMANENT / AUTO SAVED",SILVER);
        footer("A BUY B BACK SEL RIVALS");
    } else if(screen==RESULT) {
        header(race.dnf ? "DID NOT FINISH" : race.won ? "YOU WIN" : "RIVAL WINS",progress.cash);
        text(8,26,"YOUR TIME",SILVER); time(76,26,race.player.finishTime,WHITE);
        text(8,35,"RIVAL TIME",SILVER);
        if(race.rival.finishTime) time(76,35,race.rival.finishTime,RED);
        else text(76,35,"BEHIND",RED);
        text(8,46,"FINISH",SILVER); cash(40,46,race.basePay,GOLD);
        text(75,46,"SKILL",SILVER); cash(99,46,race.skillPay,CYAN);
        text(8,55,"FIRST WIN",SILVER); cash(52,55,race.firstPay,GOLD);
        if(race.record) text(88,55,"NEW BEST",CYAN);
        text(8,64,"EARNED",WHITE); cash(40,64,race.payout,GOLD);
        const char *rows[4]={selected==9 && progress.champion ? "CHAMPIONSHIP" :
            selected<progress.unlocked ? "NEXT RACE" : "NEXT RACE [LOCKED]",
            "REMATCH","GARAGE","RIVAL LIST"};
        for(int i=0;i<4;++i) row(78+i*9,rows[i],cursor==i,SILVER);
        footer(messageTicks ? message : "A SELECT B RIVAL LIST");
    } else if(screen==CHAMPION) {
        gfx_clear(INK); city(0);
        gfx_fillRect(0,36,128,92,INK);
        text35x2s(20,9,"CHAMPION",GOLD);
        center(28,"ALL TEN RIVALS BEATEN",CYAN);
        car(44,45,GOLD,false,false,0);
        center(77,"THE STREET IS YOURS",WHITE);
        center(91,"KEEP BUILDING. BEAT YOUR BEST.",SILVER);
        footer("A CONTINUE");
    } else if(screen==HELP) {
        header("DRIVER'S GUIDE",progress.cash);
        const char *lines[]={"AUTO LAUNCH AT GREEN","A: SHIFT UP   B: NOS","SHIFT IN THE GOLD ZONE","EARLY: REVS DROP","LATE: REV LIMITER","POWER NEEDS TIRE GRIP","NOS BURNS DURING SHIFTS","START: PAUSE / RESUME","FINISHES PAY. WINS PAY MORE.","UPGRADES AND BESTS AUTO SAVE"};
        for(int i=0;i<10;++i) text(6,26+i*8,lines[i],i==2 ? GOLD : SILVER);
        footer("A/B BACK");
    } else {
        header("RESET PROGRESS?",progress.cash);
        center(42,"LOSE CASH, UPGRADES,",WHITE);
        center(54,"WINS AND RECORDS?",WHITE);
        center(78,"A: RESET EVERYTHING",RED);
        center(92,"B: KEEP MY PROGRESS",CYAN);
        footer("THIS CANNOT BE UNDONE");
    }
    if(saveFailed) footer("SAVE FAILED - NOT STORED");
    else if(screen==HOME && messageTicks) footer(message);
}
}
