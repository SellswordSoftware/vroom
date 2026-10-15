#include "Appearance.h"
namespace look {
uint8_t recipe(uint8_t id) {
    // 37 is coprime with 100: this permutation visits every combination once.
    return (uint8_t)(((uint16_t)(id%OPPONENT_LOOKS)*37u+11u)%OPPONENT_LOOKS);
}
Style opponent(uint8_t id) {
    const uint8_t n=recipe(id);
    return {(uint8_t)(n%BODY_COUNT),(uint8_t)((n/BODY_COUNT)%PAINT_COUNT),
            (uint8_t)(n/(BODY_COUNT*PAINT_COUNT))};
}
const char *bodyName(uint8_t body) {
    static const char *const names[]={"COUPE","HATCH","MUSCLE","PICKUP"};
    return names[body%BODY_COUNT];
}
Building building(int32_t worldTile) {
    // Periodic world-space recipe. Identity stays attached to the building
    // when it crosses a tile boundary, including its window pattern.
    uint8_t n=(uint8_t)((worldTile%SKY_PERIOD+SKY_PERIOD)%SKY_PERIOD);
    return {(uint8_t)(10+(n*7+3)%15),(uint8_t)(15+(n*3)%6),
            (uint8_t)((n*5+1)%7)};
}
int32_t firstSkyTile(int32_t distance) {
    return (distance/SKY_SCALE)/SKY_SPACING;
}
int16_t skyX(int32_t distance,int8_t localTile) {
    return (int16_t)(localTile*SKY_SPACING-(distance/SKY_SCALE)%SKY_SPACING);
}
}
