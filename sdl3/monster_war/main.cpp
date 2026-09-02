#include <filesystem>

#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/core/game_app.h"
#include "monster_war/engine/utils/events.h"
#include "monster_war/game/scene/title_scene.h"
#include "tools/cpp/runfiles/runfiles.h"

namespace fs = std::filesystem;
using bazel::tools::cpp::runfiles::Runfiles;
using namespace pyc::monster_war;

// 初始化工作目录到 runfiles 根目录
bool setup_working_directory(int, char** argv) {
    std::string error;
    std::unique_ptr<Runfiles> runfiles(Runfiles::Create(argv[0], &error));

    if (runfiles == nullptr) {
        spdlog::error("Failed to create runfiles: {}", error);
        return false;
    }

    // 获取 runfiles 目录
    std::string runfiles_dir = runfiles->Rlocation("pycstl/sdl3/monster_war");

    if (runfiles_dir.empty()) {
        spdlog::error("Could not locate runfiles directory");
        return false;
    }

    try {
        fs::path target_dir(runfiles_dir);

        // 验证目录存在
        if (!fs::exists(target_dir)) {
            spdlog::error("Directory does not exist: {}", target_dir.string());
            return false;
        }

        if (!fs::is_directory(target_dir)) {
            spdlog::error("Not a directory: {}", target_dir.string());
            return false;
        }

        // 切换工作目录
        fs::current_path(target_dir);

        spdlog::info("Working directory set to: {}", fs::current_path().string());
        return true;

    } catch (const fs::filesystem_error& e) {
        spdlog::error("Filesystem error: {}", e.what());
        return false;
    }
}

void setupInitialScene(Context& context) {
    // GameApp在调用run方法之前，先创建并设置初始场景
    auto title_scene = std::make_unique<TitleScene>(context);
    context.getDispatcher().enqueue(PushSceneEvent{std::move(title_scene)});
}

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::info);

    // 设置工作目录
    if (!setup_working_directory(argc, argv)) {
        spdlog::warn("Could not set working directory, trying to continue...");
    }

    GameApp app;
    app.registerSceneSetup(setupInitialScene);
    app.run();
    return 0;
}
