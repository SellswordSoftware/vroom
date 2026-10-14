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
    gfx_fillRect(0,0,128,34,NAVY);
    gfx_fillCircle(107,12,6,SILVER); gfx_fillCircle(104,10,6,NAVY);
    int offset=(distance/11000)%19;
    for(int i=-1;i<8;++i) {
        int x=i*19-offset, h=10+(i+9)*7%17;
        gfx_fillRect(x,34-h,15,h,INK);
        for(int y=36-h;y<31;y+=5)
            for(int w=0;w<3;++w) if((i+w+y)%3) gfx_fillRect(x+2+w*4,y,2,2,BLUE);
    }
    gfx_hline(0,33,128,CYAN);
    gfx_fillRect(0,34,128,38,INK);
    gfx_fillRect(0,35,128,2,WOOD);
    int road=(distance/350)%25;
    for(int x=-25-road;x<128;x+=25) gfx_fillRect(x,49,13,2,SILVER);
    gfx_hline(0,70,128,BLUE);
}
void car(int x,int y,uint8_t color,bool boosting,bool slip,uint16_t tick) {
    if(boosting) {
        gfx_fillRect(x-9,y+8,9,3,GOLD);
        gfx_fillRect(x-5,y+7,6,5,CYAN);
        gfx_hline(x-13+(tick%3),y+9,5,RED);
    }
    if(slip) {
        gfx_fillCircle(x+4-(tick%4)*2,y+12,3,SILVER);
        gfx_fillCircle(x-(tick%6),y+9,2,SILVER);
    }
    gfx_fillRect(x,y+13,39,2,NAVY); // shadow
    gfx_fillRect(x+1,y+6,36,6,color);
    gfx_fillRect(x+10,y+1,16,7,color);
    gfx_line(x+7,y+6,x+11,y+1,color);
    gfx_line(x+26,y+1,x+31,y+6,color);
    gfx_fillRect(x+12,y+2,6,4,NAVY); gfx_fillRect(x+20,y+2,6,4,NAVY);
    gfx_hline(x+13,y+2,4,CYAN); gfx_hline(x+21,y+2,4,CYAN);
    gfx_hline(x+2,y+7,33,WHITE);
    gfx_fillRect(x+19,y+7,2,1,INK); // handle
    gfx_fillRect(x+34,y+7,4,2,GOLD);
    gfx_pixel(x+1,y+8,RED);
    gfx_fillRect(x,y+3,6,2,color); gfx_vline(x+3,y+4,3,color); // spoiler
    gfx_fillCircle(x+8,y+12,4,INK); gfx_fillCircle(x+29,y+12,4,INK);
    gfx_fillCircle(x+8,y+12,2,SILVER); gfx_fillCircle(x+29,y+12,2,SILVER);
    gfx_pixel(x+8+(tick%2),y+12,INK); gfx_pixel(x+29+(tick%2),y+12,INK);
    gfx_hline(x+13,y+12,12,WOOD);
}
void title() {
    city(0);
    text35x2s(13,7,"VROOM",GOLD);
    text(30,25,"SHIFT / EARN / BUILD",CYAN);
    car(40,48,GOLD,false,false,0);
}
void raceView(const vr::Race &r) {
    const vr::Car &c=r.player;
    city(c.distance);
    int gap=(r.rival.distance-c.distance)/1400;
    int rivalX=32+gap;
    car(rivalX,35,RED,r.rival.boostLeft!=0,r.rival.slipping,r.elapsed);
    if(rivalX>100) { text(118,40,">",RED); }
    if(rivalX<-30) { text(3,40,"<",RED); }
    car(32,54,GOLD,c.boostLeft!=0,c.slipping,r.elapsed);
    gfx_fillRect(0,0,128,8,INK);
    text(3,1,"1/4 MI",SILVER);
    if(r.elapsed) time(99,1,r.elapsed,WHITE); else text(99,1,"0.00",WHITE);
    gfx_fillRect(0,72,128,56,NAVY);
    gfx_fillRect(4,72,120,1,BLUE);
    number(7,77,vr::mph(c),WHITE); text(27,77,"MPH",SILVER);
    text(69,77,"GEAR",SILVER); number(91,77,c.gear,GOLD);
    text(98,77,"/",SILVER); number(105,77,r.playerBuild.gears,SILVER);
    // RPM instrument: broad Good band, narrow Perfect band, red limiter.
    gfx_fillRect(7,88,114,11,INK);
    gfx_fillRect(8+6650*112/8500,89,(7750-6650)*112/8500,9,WOOD);
    gfx_fillRect(8+7000*112/8500,89,(7600-7000)*112/8500,9,GOLD);
    gfx_fillRect(8+8000*112/8500,89,7,9,RED);
    int bar=c.rpm*112/8500;
    gfx_fillRect(8,93,bar,4,c.rpm>=vr::REDLINE ? RED : CYAN);
    gfx_vline(8+bar,87,13,WHITE);
    text(7,103,"RPM",SILVER); number(23,103,c.rpm,WHITE);
    const char *feedback="";
    uint8_t fc=CYAN;
    if(c.feedbackLeft) {
        feedback=c.feedback==vr::PERFECT ? "PERFECT" : c.feedback==vr::GOOD ? "GOOD" :
            c.feedback==vr::EARLY ? "EARLY" : "LATE";
        fc=c.feedback==vr::PERFECT ? GOLD : c.feedback==vr::GOOD ? CYAN : RED;
    } else if(c.rpm>=vr::REDLINE && c.gear<r.playerBuild.gears) { feedback="SHIFT!"; fc=RED; }
    else if(c.slipping) { feedback="WHEELSPIN"; fc=SILVER; }
    else if(c.gear==r.playerBuild.gears) feedback="TOP GEAR";
    else feedback="A SHIFT";
    text(124-text35Width(feedback),103,feedback,fc);
    text(7,116,!r.playerBuild.nos ? "NOS LOCKED" : c.boostLeft ? "BOOST" :
        c.boostUsed ? "NOS USED" : "B NOS READY",c.boostLeft ? CYAN : SILVER);
    if(c.boostLeft) {
        gfx_rect(88,114,33,7,BLUE); gfx_fillRect(89,115,c.boostLeft*31/120,5,CYAN);
    } else {
        gfx_rect(88,114,33,7,BLUE);
        gfx_fillRect(89,115,(int32_t)(c.distance<vr::FINISH ? c.distance : vr::FINISH)*31/vr::FINISH,5,GOLD);
    }
    if(r.phase==vr::COUNTDOWN || r.elapsed<40) {
        gfx_fillRoundRect(49,14,30,20,3,INK); gfx_roundRect(49,14,30,20,3,GOLD);
        if(r.phase==vr::COUNTDOWN) {
            char b[4]; fmtInt(b,(r.countdown+59)/60); text35x2(59,18,b,GOLD);
        } else text35x2(56,18,"GO",CYAN);
    }
    if(r.paused) {
        gfx_fillRoundRect(15,24,98,34,3,INK); gfx_roundRect(15,24,98,34,3,CYAN);
        center(31,"PAUSED",WHITE); center(43,"START RESUMES",CYAN);
    }
}
}
