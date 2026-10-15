#pragma once
#include "Game.h"
namespace app {
enum Screen : uint8_t { HOME, LIST, GARAGE, RACING, RESULT, CHAMPION, HELP,
    RESET_CONFIRM, CAR_PREVIEW, SKY_PREVIEW, MILESTONE, MIGRATION };
extern vr::Progress progress;
extern vr::Race race;
extern Screen screen;
extern uint8_t cursor, selected;
extern bool soundOn, saveFailed;
extern const char *message;
extern uint8_t messageTicks;
void begin();
void update();
void render();
void savePending();
}
