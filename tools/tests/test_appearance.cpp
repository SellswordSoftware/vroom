#include "../../Appearance.h"
#include <stdio.h>
using namespace look;
static int checks=0,failures=0;
#define CHECK(x) do{++checks;if(!(x)){++failures;printf("FAIL line %d: %s\n",__LINE__,#x);}}while(0)
int main() {
    bool seen[BODY_COUNT][PAINT_COUNT][DECAL_COUNT]={};
    uint8_t bodies[BODY_COUNT]={},paints[PAINT_COUNT]={},decals[DECAL_COUNT]={};
    for(int i=0;i<OPPONENT_LOOKS;++i) {
        Style s=opponent(i);
        CHECK(s.body<BODY_COUNT && s.paint<PAINT_COUNT && s.decal<DECAL_COUNT);
        CHECK(!seen[s.body][s.paint][s.decal]);
        seen[s.body][s.paint][s.decal]=true;
        ++bodies[s.body];++paints[s.paint];++decals[s.decal];
        Style again=opponent(i);CHECK(s.body==again.body && s.paint==again.paint && s.decal==again.decal);
    }
    for(int b=0;b<BODY_COUNT;++b) CHECK(bodies[b]==25);
    for(int p=0;p<PAINT_COUNT;++p) CHECK(paints[p]==20);
    for(int d=0;d<DECAL_COUNT;++d) CHECK(decals[d]==20);
    // When the scroll wraps one tile, the tile at old local+1 is the tile
    // at new local, shifted just one pixel rather than regenerated.
    for(int boundary=1;boundary<=SKY_PERIOD*2;++boundary) {
        int32_t after=(int32_t)boundary*SKY_SPACING*SKY_SCALE;
        int32_t before=after-SKY_SCALE;
        for(int local=-1;local<6;++local) {
            int32_t oldId=firstSkyTile(before)+local+1;
            int32_t newId=firstSkyTile(after)+local;
            CHECK(oldId==newId);
            Building a=building(oldId), b=building(newId);
            CHECK(a.height==b.height && a.width==b.width && a.lights==b.lights);
            CHECK(skyX(after,local)==skyX(before,local+1)-1);
        }
    }
    for(int id=-1;id<SKY_PERIOD;++id) {
        Building a=building(id),b=building(id+SKY_PERIOD);
        CHECK(a.height==b.height && a.width==b.width && a.lights==b.lights);
    }
    printf("Appearance: 100 unique recipes; skyline wraps continuously; %d checks, %d failures\n",checks,failures);
    return failures ? 1 : 0;
}
