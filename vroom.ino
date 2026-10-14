// Vroom: an incremental quarter-mile racer for CHGame.
// Game.*: portable vehicle model and progression.
// App.*: input, screens, sound, and flash saves.
// View.*: code-drawn pixel art and the race dashboard.
#include <CHGame.h>
#include "config.h"
#include "App.h"
void setup() {
    chgame.boot();
    dbg::begin("VRM1 " VRM_VERSION);
    gfx_begin(GFX_DIV2, GFX_12BPP);
    pal::init();
    app::begin();
    chgame.setFrameRate(VRM_FPS);
}
void loop() {
    dbg::poll();
    if (!chgame.nextFrame()) return;
    dbg::markUpdateStart();
    // Fixed-rate logic; bounded catch-up keeps racing speed stable on slow frames.
    uint8_t ticks=0;
    do {
        chgame.pollButtons();
        app::update();
        audio::update();
    } while (++ticks<3 && chgame.nextFrame());
    gfx_wait();
    app::savePending();
    dbg::markRenderStart();
    app::render();
    dbg::markRenderEnd();
    gfx_flushAsync();
}
