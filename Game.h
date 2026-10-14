#pragma once
#include <stdint.h>
namespace vr {
constexpr uint16_t HZ = 60;
constexpr int32_t FINISH = 402336; // quarter mile, millimetres
constexpr uint16_t REDLINE = 8000, IDEAL = 7300;
enum Upgrade : uint8_t { ENGINE, GEARBOX, TIRES, FOURTH, FIFTH, NOS, UPGRADE_COUNT };
enum Shift : uint8_t { NONE, EARLY, GOOD, PERFECT, LATE };
enum Phase : uint8_t { COUNTDOWN, RUNNING, RESULTS };
struct Progress {
    uint32_t cash;
    uint16_t wonMask;
    uint8_t unlocked; // highest available index, 0..9
    uint8_t levels[UPGRADE_COUNT];
    uint8_t champion;
    uint16_t best[10]; // elapsed 60 Hz ticks; 0 means no record
};
struct Build { uint8_t engine, gearbox, tires, gears; bool nos; };
struct Car {
    int32_t speed, distance; // mm/s and mm
    int32_t speedRemainder, distanceRemainder;
    uint16_t rpm, time, finishTime, boostLeft, shiftLeft;
    uint8_t gear, perfects, goods;
    bool boostUsed, slipping;
    Shift feedback;
    uint8_t feedbackLeft;
};
struct Race {
    Build playerBuild, rivalBuild;
    Car player, rival;
    uint16_t countdown, elapsed;
    uint8_t opponent;
    Phase phase;
    bool paused, settled, won, record, firstWin, dnf;
    uint32_t payout, basePay, skillPay, firstPay;
};
void reset(Progress &p);
bool valid(const Progress &p);
Build build(const Progress &p);
Build opponentBuild(uint8_t n);
uint16_t opponentShift(uint8_t n);
const char *opponentName(uint8_t n);
uint8_t limit(Upgrade u);
uint32_t price(const Progress &p, Upgrade u);
bool unlocked(const Progress &p, Upgrade u);
bool buy(Progress &p, Upgrade u);
void resetCar(Car &c);
Shift shift(Car &c, const Build &b);
bool boost(Car &c, const Build &b);
void stepCar(Car &c, const Build &b);
void start(Race &r, const Progress &p, uint8_t opponent);
void tick(Race &r, bool shiftPressed = false, bool boostPressed = false);
bool settle(Race &r, Progress &p);
uint16_t mph(const Car &c);
}
