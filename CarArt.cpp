#include <CHGame.h>
#include "CarArt.h"
namespace view {
static void polygon(int x,int y,const uint8_t *points,uint8_t n,uint8_t color) {
    int16_t xy[16];
    for(uint8_t i=0;i<n;++i) { xy[i*2]=(x+points[i*2])*16; xy[i*2+1]=(y+points[i*2+1])*16; }
    fillConvex(xy,n,color);
}
void car(int x,int y,const look::Style &style,bool player,bool boosting,bool slip,
         uint16_t tick,uint8_t number) {
    if(x+look::CAR_WIDTH+18<0 || x>140) return;
    static const uint8_t paints[5]={RED,BLUE,CYAN,SILVER,FELT_LT};
    static const uint8_t shades[5]={WINE,NAVY,BLUE,BLUE,FELT_DK};
    static const uint8_t accents[5]={GOLD,WHITE,GOLD,RED,GOLD};
    const uint8_t paint=style.paint%5;
    const uint8_t color=player ? GOLD : paints[paint];
    const uint8_t shade=player ? WOOD : shades[paint];
    const uint8_t accent=player ? WHITE : accents[paint];
    const uint8_t body=style.body%look::BODY_COUNT;
    if(boosting) {
        gfx_fillRect(x-12,y+16,15,4,RED);
        gfx_fillRect(x-8,y+16,11,3,GOLD);
        gfx_fillRect(x-4,y+17,7,2,CYAN);
        gfx_hline(x-18+(tick%4),y+18,6,GOLD);
    }
    if(slip) {
        gfx_fillCircle(x+8-(tick%4)*2,y+21,4,SILVER);
        gfx_fillCircle(x+2-(tick%6),y+18,3,SILVER);
    }
    gfx_fillRect(x+3,y+24,53,2,NAVY);
    // Native-resolution silhouettes, rather than enlarging the old sprite.
    static const uint8_t coupe[]={1,13, 12,11, 22,3, 36,3, 46,11, 58,12, 60,18, 2,20};
    static const uint8_t hatch[]={2,12, 8,2, 33,2, 44,11, 57,12, 60,18, 56,20, 2,20};
    static const uint8_t muscle[]={1,11, 15,10, 22,4, 36,4, 43,10, 58,10, 60,18, 2,20};
    static const uint8_t pickup[]={2,10, 25,10, 25,2, 40,2, 46,10, 58,11, 60,19, 2,20};
    const uint8_t *silhouette=body==look::COUPE ? coupe : body==look::HATCH ? hatch :
        body==look::MUSCLE ? muscle : pickup;
    polygon(x,y,silhouette,8,color);
    gfx_fillRect(x+4,y+18,52,3,shade);
    gfx_hline(x+4,y+12,52,accent); // shoulder highlight
    gfx_hline(x+18,y+20,23,INK);
    static const uint8_t coupeGlass[]={22,5, 35,5, 42,10, 16,10};
    static const uint8_t hatchGlass[]={10,4, 32,4, 39,10, 8,10};
    static const uint8_t muscleGlass[]={23,6, 35,6, 40,10, 18,10};
    static const uint8_t pickupGlass[]={28,4, 38,4, 42,9, 28,9};
    const uint8_t *glass=body==look::COUPE ? coupeGlass : body==look::HATCH ? hatchGlass :
        body==look::MUSCLE ? muscleGlass : pickupGlass;
    polygon(x,y,glass,4,NAVY);
    const int gx=body==look::HATCH ? 11 : body==look::PICKUP ? 29 : 24;
    gfx_hline(x+gx,y+(body==look::MUSCLE ? 6 : body==look::COUPE ? 5 : 4),6,CYAN);
    if(body!=look::PICKUP) gfx_vline(x+30,y+5,6,color);
    // Reusable liveries all fit the shared side panel between the wheel arches.
    if(style.decal==look::STRIPE) {
        gfx_fillRect(x+5,y+14,50,2,accent);
        gfx_hline(x+19,y+17,23,accent);
    } else if(style.decal==look::ROUNDEL) {
        gfx_fillRect(x+21,y+13,17,7,WHITE);
        char digits[5]; fmtInt(digits,number ? number : 1);
        text35(x+29-text35Width(digits)/2,y+14,digits,INK);
    } else if(style.decal==look::CHECKER) {
        for(int row=0;row<3;++row)
            for(int col=0;col<11;++col)
                gfx_fillRect(x+18+col*2,y+13+row*2,2,2,(row+col)%2 ? INK : accent);
    } else if(style.decal==look::BOLT) {
        // The bolt is concave, so draw it as two filled wedges.
        static const uint8_t top[]={32,13, 23,16, 32,16};
        static const uint8_t bottom[]={29,15, 24,20, 39,15};
        polygon(x,y,top,3,accent); polygon(x,y,bottom,3,accent);
    }
    const int door=body==look::PICKUP ? 43 : 40;
    gfx_vline(x+door,y+11,8,shade);
    gfx_hline(x+door-5,y+12,3,INK);
    if(body==look::PICKUP) {
        gfx_fillRect(x+4,y+11,19,2,INK); // open bed, tailgate
        gfx_vline(x+23,y+11,9,shade);
    } else if(body==look::MUSCLE) {
        gfx_fillRect(x+45,y+9,7,2,INK); // hood scoop
    } else if(body==look::COUPE) {
        gfx_fillRect(x+1,y+8,9,2,color); gfx_vline(x+5,y+9,3,shade);
    } else {
        gfx_fillRect(x+4,y+7,3,4,shade); // hatch rear pillar
    }
    gfx_fillRect(x+1,y+14,3,3,RED);
    gfx_fillRect(x+55,y+13,4,2,WHITE);
    gfx_fillRect(x+57,y+16,3,3,INK); gfx_pixel(x+58,y+17,SILVER);
    gfx_hline(x+2,y+20,6,SILVER); gfx_hline(x+52,y+20,6,SILVER);
    for(uint8_t wheel=0;wheel<2;++wheel) {
        int wx=x+(wheel ? 47 : 13);
        gfx_fillCircle(wx,y+20,5,INK);
        gfx_fillCircle(wx,y+20,3,SILVER);
        gfx_fillCircle(wx,y+20,1,INK);
        if(tick&1) {
            gfx_pixel(wx-2,y+18,WHITE); gfx_pixel(wx+2,y+22,INK);
        } else {
            gfx_hline(wx-3,y+20,7,INK); gfx_vline(wx,y+17,7,WHITE);
        }
    }
}
void playerCar(int x,int y,bool boosting,bool slip,uint16_t tick) {
    const look::Style player={look::COUPE,0,look::STRIPE};
    car(x,y,player,true,boosting,slip,tick);
}
}
