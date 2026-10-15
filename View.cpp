#include <CHGame.h>
#include "View.h"
namespace view {
void text(int x,int y,const char *s,uint8_t color) { text35(x,y,s,color); }
void center(int y,const char *s,uint8_t color) { text((128-text35Width(s))/2,y,s,color); }
void number(int x,int y,int32_t n,uint8_t color) { char buf[12]; fmtInt(buf,n); text(x,y,buf,color); }
void cash(int x,int y,uint32_t n,uint8_t color) { text(x,y,"$",color); number(x+4,y,n,color); }
void time(int x,int y,uint16_t ticks,uint8_t color) {
    if(!ticks) { text(x,y,"--.--",color); return; }
    uint16_t cs=(uint16_t)((uint32_t)ticks*100/60);
    char buf[12]; fmtInt(buf,cs/100); text(x,y,buf,color);
    int nx=x+text35Width(buf); text(nx,y,".",color);
    gfx_pixel(nx, y+4,color);
    if(cs%100<10) { text(nx+4,y,"0",color); number(nx+8,y,cs%100,color); }
    else number(nx+4,y,cs%100,color);
}
void header(const char *title,uint32_t money) {
    gfx_clear(INK);
    gfx_fillRect(0,0,128,21,NAVY); text(6,5,title,CYAN);
    gfx_hline(0,20,128,BLUE);
    char b[12]; fmtInt(b,money); cash(123-text35Width(b)-4,12,money,GOLD);
}
void footer(const char *hint) {
    gfx_fillRect(0,115,128,13,NAVY);
    gfx_hline(0,115,128,BLUE); center(120,hint,SILVER);
}
void row(int y,const char *label,bool active,uint8_t color) {
    if(active) { gfx_fillRect(4,y-3,120,12,NAVY); gfx_fillRect(4,y-3,2,12,CYAN); }
    text(10,y,label,active ? WHITE : color);
    if(active) text(116,y,">",CYAN);
}
void city(int32_t distance) {
    gfx_fillRect(0,0,128,28,NAVY);
    gfx_fillCircle(107,12,6,SILVER); gfx_fillCircle(104,10,6,NAVY);
    const int32_t first=look::firstSkyTile(distance);
    for(int8_t local=-1;local<7;++local) {
        const look::Building b=look::building(first+local);
        const int x=look::skyX(distance,local);
        gfx_fillRect(x,28-b.height,b.width,b.height,INK);
        for(int row=0;row<(b.height-3)/5;++row)
            for(int w=0;w<3;++w)
                if((b.lights+row*3+w)%4)
                    gfx_fillRect(x+2+w*5,30-b.height+row*5,2,2,BLUE);
    }
    gfx_hline(0,27,128,CYAN);
    gfx_fillRect(0,28,128,56,INK);
    gfx_fillRect(0,29,128,2,WOOD);
    const int road=(distance/look::ROAD_SCALE)%look::ROAD_PERIOD;
    for(int x=-look::ROAD_PERIOD-road;x<128;x+=look::ROAD_PERIOD)
        gfx_fillRect(x,55,look::ROAD_MARK,2,SILVER);
    // The nearer verge moves faster than the lane markings, adding depth.
    const int verge=(distance/look::VERGE_SCALE)%look::VERGE_PERIOD;
    for(int x=-look::VERGE_PERIOD-verge;x<128;x+=look::VERGE_PERIOD) {
        gfx_hline(x,80,22,BLUE);
        gfx_hline(x+26,81,12,SILVER);
    }
    gfx_hline(0,83,128,BLUE);
}
void title() {
    city(0);
    text35x2s(13,7,"VROOM",GOLD);
    text(30,22,"SHIFT / EARN / BUILD",CYAN);
    playerCar(34,37);
    gfx_fillRect(0,63,128,65,INK);
}
void raceView(const vr::Race &r) {
    const vr::Car &c=r.player;
    city(c.distance);
    int gap=(r.rival.distance-c.distance)/1400;
    int rivalX=24+gap;
    const look::Style rival=look::opponent(r.opponent);
    car(rivalX,29,rival,false,r.rival.boosting||r.rival.pulseLeft,r.rival.slipping,
        (uint16_t)(r.rival.distance/250),r.opponent+1);
    if(rivalX>112) { text(120,40,">",RED); }
    if(rivalX+look::CAR_WIDTH<16) { text(3,40,"<",RED); }
    playerCar(24,57,c.boosting||c.pulseLeft,c.slipping,(uint16_t)(c.distance/250));
    gfx_fillRect(0,0,128,8,INK);
    text(3,1,vr::distanceName(r.opponent),SILVER);
    if(r.elapsed) time(99,1,r.elapsed,WHITE); else text(99,1,"0.00",WHITE);
    gfx_fillRect(0,84,128,44,NAVY);
    gfx_fillRect(4,84,120,1,BLUE);
    number(7,87,vr::mph(c),WHITE); text(27,87,"MPH",SILVER);
    text(69,87,"GEAR",SILVER); number(91,87,c.gear,GOLD);
    text(98,87,"/",SILVER); number(105,87,r.playerBuild.gears,SILVER);
    const bool staging=r.playerBuild.manual&&(r.phase==vr::COUNTDOWN||!c.launched);
    // One instrument changes from launch RPM to shift RPM once launched.
    gfx_fillRect(7,97,114,10,INK);
    if(staging) {
        int tolerance=400+r.playerBuild.launch*120;
        int lo=(vr::LAUNCH_RPM-tolerance)*112/8500;
        int hi=(vr::LAUNCH_RPM+tolerance)*112/8500;
        gfx_fillRect(8+lo,98,hi-lo,8,GOLD);
        gfx_fillRect(8+6000*112/8500,98,33,8,RED);
    } else {
        gfx_fillRect(8+6650*112/8500,98,(7750-6650)*112/8500,8,WOOD);
        gfx_fillRect(8+7000*112/8500,98,(7600-7000)*112/8500,8,GOLD);
        gfx_fillRect(8+8000*112/8500,98,7,8,RED);
    }
    int bar=c.rpm*112/8500;
    gfx_fillRect(8,102,bar,4,c.rpm>=vr::REDLINE?RED:CYAN);
    gfx_vline(8+bar,96,12,WHITE);
    text(7,111,"RPM",SILVER);number(23,111,c.rpm,WHITE);
    const char *feedback="";
    uint8_t fc=CYAN;
    if(c.falseStart&&r.phase==vr::COUNTDOWN) {
        feedback="FALSE +0.5S";fc=RED;
    } else if(staging) {
        feedback=r.phase==vr::COUNTDOWN?"HOLD A AT 1":"RELEASE A!";
        fc=r.phase==vr::COUNTDOWN?CYAN:GOLD;
    } else if(c.window) {
        feedback=vr::pulsePerfect(c)?"UP NOW!":"UP READY";
        fc=vr::pulsePerfect(c)?GOLD:CYAN;
    } else if(c.pulseFeedback) {
        feedback=c.pulseQuality==100?"PULSE PERFECT":c.pulseQuality?"PULSE GOOD":"PULSE MISS";
        fc=c.pulseQuality?GOLD:RED;
    } else if(c.launchFeedback) {
        feedback=c.falseStart?"FALSE +0.5S":c.launchGrade==vr::PERFECT?"GREAT LAUNCH":
            c.launchGrade==vr::GOOD?"GOOD LAUNCH":c.launchGrade==vr::EARLY?"BOGGED":"WHEELSPIN";
        fc=c.falseStart?RED:CYAN;
    } else if(c.feedbackLeft) {
        feedback=c.feedback==vr::PERFECT?"PERFECT":c.feedback==vr::GOOD?"GOOD":
            c.feedback==vr::EARLY?"EARLY":"LATE";
        fc=c.feedback==vr::PERFECT?GOLD:c.feedback==vr::GOOD?CYAN:RED;
    } else if(c.rpm>=vr::REDLINE&&c.gear<r.playerBuild.gears){feedback="SHIFT!";fc=RED;}
    else if(c.slipping){feedback="WHEELSPIN";fc=SILVER;}
    else if(c.gear==r.playerBuild.gears)feedback="TOP GEAR";
    else feedback="A SHIFT";
    text(124-text35Width(feedback),111,feedback,fc);
    if(r.playerBuild.nos) {
        text(6,121,"B",c.boosting?CYAN:SILVER);
        gfx_rect(14,120,49,7,BLUE);
        gfx_fillRect(15,121,(uint32_t)c.fuel*47/vr::fuelCapacity(r.playerBuild),5,CYAN);
    } else text(6,121,"NOS @ STAGE 3",SILVER);
    if(r.playerBuild.overdrive) {
        text(69,121,"UP",c.window?GOLD:SILVER);number(82,121,c.charges,GOLD);
        gfx_rect(94,120,30,7,BLUE);
        if(c.window) {
            gfx_fillRect(95+6*27/24,121,9*27/24,5,GOLD);
            gfx_vline(95+(24-c.window)*27/24,120,7,WHITE);
        } else if(c.pulseLeft)gfx_fillRect(95,121,c.pulseLeft*28/(42+r.playerBuild.pulse*6),5,GOLD);
    } else text(93,121,"UP LOCK",SILVER);
    if(r.phase==vr::COUNTDOWN || r.elapsed<40 || !c.launched) {
        gfx_fillRoundRect(49,9,30,18,3,INK); gfx_roundRect(49,9,30,18,3,GOLD);
        if(r.phase==vr::COUNTDOWN) {
            char b[4]; fmtInt(b,(r.countdown+59)/60); text35x2(59,12,b,GOLD);
        } else text35x2(56,12,"GO",CYAN);
    }
    if(r.paused) {
        gfx_fillRoundRect(15,24,98,34,3,INK); gfx_roundRect(15,24,98,34,3,CYAN);
        center(31,"PAUSED",WHITE); center(43,"START RESUMES",CYAN);
    }
}
#if CHGAME_DEBUG
void appearancePreview(uint8_t id) {
    gfx_clear(INK); city(0);
    const look::Style style=look::opponent(id);
    gfx_fillRect(0,0,128,27,NAVY);
    text(6,5,"OPPONENT",CYAN); number(45,5,id+1,WHITE);
    text(6,16,look::bodyName(style.body),GOLD);
    car(33,36,style,false,false,false,0,id+1);
    gfx_fillRect(0,84,128,44,NAVY);
    text(8,91,"PAINT",SILVER); number(33,91,style.paint+1,CYAN);
    text(70,91,"DECAL",SILVER); number(95,91,style.decal+1,GOLD);
    static const char *const decals[]={"PLAIN","STRIPE","NUMBER","CHECKER","LIGHTNING"};
    center(106,decals[style.decal],WHITE);
}
#endif

}
