#include <rtc/rtc.h>

#include "game.h"

class CleanupClass {
public:
    CleanupClass() = default;
    ~CleanupClass() {
        rtcCleanup();
    }
};

static CleanupClass cleanup_class;

void Game::init() {
    rtcPreload();
}
