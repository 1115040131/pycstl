#include <filesystem>

#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/game_app.h"
#include "sunny_land/engine/scene/scene_manager.h"
#include "sunny_land/game/scene/title_scene.h"
#include "tools/cpp/runfiles/runfiles.h"

namespace fs = std::filesystem;
using bazel::tools::cpp::runfiles::Runfiles;

// 初始化工作目录到 runfiles 根目录
bool setup_working_directory(int, char** argv) {
    std::string error;
    std::unique_ptr<Runfiles> runfiles(Runfiles::Create(argv[0], &error));

    if (runfiles == nullptr) {
        spdlog::error("Failed to create runfiles: {}", error);
        return false;
    }

    // 获取 runfiles 目录
    std::string runfiles_dir = runfiles->Rlocation("pycstl/sdl3/sunny_land");

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

void setupInitialScene(pyc::sunny_land::SceneManager& scene_manager) {
    // GameApp在调用run方法之前，先创建并设置初始场景
    auto title_scene = std::make_unique<pyc::sunny_land::TitleScene>(scene_manager.getContext(), scene_manager);
    scene_manager.requestPushScene(std::move(title_scene));
}

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::info);

    // 设置工作目录
    if (!setup_working_directory(argc, argv)) {
        spdlog::warn("Could not set working directory, trying to continue...");
    }

    pyc::sunny_land::GameApp app;
    app.registerSceneSetup(setupInitialScene);
    app.run();
    return 0;
}
