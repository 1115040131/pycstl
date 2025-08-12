#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/game_app.h"

int main() {
    spdlog::set_level(spdlog::level::trace);
    pyc::sunny_land::GameApp app;
    app.run();
    return 0;
}
