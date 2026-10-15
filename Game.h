#pragma once
#include <stdint.h>
namespace vr {
constexpr uint16_t HZ=60;
constexpr uint8_t RIVALS=36, STAGES=6, PER_STAGE=6, SAVE_VERSION=2;
constexpr int32_t FINISH=402336;
constexpr uint16_t REDLINE=8000, IDEAL=7300, LAUNCH_RPM=4200;
enum Upgrade : uint8_t { ENGINE, GEARBOX, TIRES, LAUNCH, NOS_CAPACITY, NOS_PRESSURE,
    OVERDRIVE_POWER, OVERDRIVE_CHARGES, UPGRADE_COUNT };
enum Shift : uint8_t { NONE, EARLY, GOOD, PERFECT, LATE };
enum Phase : uint8_t { COUNTDOWN, RUNNING, RESULTS };
enum Award : uint8_t { NO_AWARD, MANUAL_AWARD, NOS_AWARD, OVERDRIVE_AWARD, GEAR_AWARD, FINAL_LEVELS, TITLE_AWARD };
struct Progress {
    uint32_t cash;
    uint16_t best[RIVALS];
    uint16_t legacyBest[10]; // archived MVP quarter-mile records
    uint8_t levels[UPGRADE_COUNT];
    uint8_t wins, unlocked, champion;
    uint8_t legacyWins, importNotice;
};
// Exact original ABI for importing version-1 records.
struct LegacyProgress {
    uint32_t cash;
    uint16_t wonMask;
    uint8_t unlocked, levels[6], champion;
    uint16_t best[10];
};
struct Build {
    uint8_t engine, gearbox, tires, gears, launch, capacity, pressure, pulse, charges;
    bool manual, nos, overdrive;
};
struct Input { bool aHeld, aPressed, aReleased, bHeld, upPressed; };
struct Car {
    int32_t speed, distance, speedRemainder, distanceRemainder;
    uint16_t rpm, time, finishTime, fuel, fuelUsed, shiftLeft, pulseLeft, windowDelay;
    uint16_t stagingRpm, launchDelay;
    uint8_t gear, perfects, goods, launchEfficiency, launchTicks;
    uint8_t charges, window, pulseQuality, pulses, pulseMisses, pulseFeedback;
    uint8_t feedbackLeft, launchFeedback;
    Shift feedback, launchGrade;
    bool slipping, boosting, launched, launchArmed, heldA, windowArmed, falseStart;
};
struct Race {
    Build playerBuild, rivalBuild;
    Car player, rival;
    int32_t length;
    uint16_t countdown, elapsed;
    uint8_t opponent;
    Phase phase;
    Award award;
    bool paused, settled, won, record, firstWin, dnf, launchInputReady;
    uint32_t payout, basePay, skillPay, firstPay;
};
void reset(Progress &p);
bool valid(const Progress &p);
bool migrate(const LegacyProgress &old,Progress &p);
bool hasWon(const Progress &p,uint8_t n);
Build build(const Progress &p);
Build opponentBuild(uint8_t n);
uint16_t opponentShift(uint8_t n);
const char *opponentName(uint8_t n);
const char *opponentTrait(uint8_t n);
const char *stageName(uint8_t stage);
const char *distanceName(uint8_t n);
int32_t raceLength(uint8_t n);
uint8_t stageOf(uint8_t n);
uint8_t limit(Upgrade u);
uint8_t availableLevel(const Progress &p,Upgrade u);
uint8_t unlockWins(Upgrade u,uint8_t nextLevel);
uint32_t price(const Progress &p,Upgrade u);
bool unlocked(const Progress &p,Upgrade u);
bool buy(Progress &p,Upgrade u);
uint16_t fuelCapacity(const Build &b);
uint8_t pulseCapacity(const Build &b);
void resetCar(Car &c,const Build &b);
Shift shift(Car &c,const Build &b);
bool overdrive(Car &c,const Build &b);
void stepCar(Car &c,const Build &b,int32_t finish,bool nosHeld);
void start(Race &r,const Progress &p,uint8_t opponent);
void tick(Race &r,Input input={});
bool settle(Race &r,Progress &p);
uint16_t mph(const Car &c);
bool pulsePerfect(const Car &c);
}
