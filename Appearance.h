#pragma once
#include <stdint.h>
namespace look {
constexpr uint8_t BODY_COUNT=4, PAINT_COUNT=5, DECAL_COUNT=5, OPPONENT_LOOKS=100;
constexpr uint8_t CAR_WIDTH=61, CAR_HEIGHT=26;
enum Body : uint8_t { COUPE, HATCH, MUSCLE, PICKUP };
enum Decal : uint8_t { PLAIN, STRIPE, ROUNDEL, CHECKER, BOLT };
// One byte identifies a recipe; no per-opponent sprites or saved appearance data.
struct Style { uint8_t body, paint, decal; };
Style opponent(uint8_t id);
uint8_t recipe(uint8_t id);
const char *bodyName(uint8_t body);

constexpr int32_t SKY_SCALE=1500; // mm per parallax pixel: 3x the original skyline motion
constexpr int16_t SKY_SPACING=24;
constexpr uint8_t SKY_PERIOD=16;
// Long, spaced markings avoid temporal aliasing at the faster scroll speed.
constexpr int32_t ROAD_SCALE=100; // mm per pixel: 3.5x the old road scroll
constexpr int16_t ROAD_PERIOD=64, ROAD_MARK=30;
constexpr int32_t VERGE_SCALE=70;
constexpr int16_t VERGE_PERIOD=96;
struct Building { uint8_t height, width, lights; };
Building building(int32_t worldTile);
int32_t firstSkyTile(int32_t distance);
int16_t skyX(int32_t distance, int8_t localTile);
}
