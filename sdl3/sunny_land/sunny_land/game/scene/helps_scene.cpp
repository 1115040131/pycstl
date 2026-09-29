module;

#include <memory>
#include <utility>

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_scancode.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

module sunny_land.game.scene.helps_scene;

import common.string_hash;
import sunny_land.engine.core;
import sunny_land.engine.input.input_manager;
import sunny_land.engine.scene;
import sunny_land.engine.ui.ui_image;
import sunny_land.engine.ui.ui_manager;
import sunny_land.game.data.session_data;

namespace pyc::sunny_land {

HelpsScene::HelpsScene(Context& context, SceneManager& scene_manager)
    : Scene("HelpsScene", context, scene_manager) {
    spdlog::trace("HelpsScene 构造完成。");
}

void HelpsScene::init() {
    if (is_initialized_) {
        spdlog::warn("HelpsScene 已经初始化过了，重复调用 init()。");
        return;
    }

    auto window_size = glm::vec2(640.0f, 360.0f);

    // 创建帮助图片 UIImage （让它覆盖整个屏幕）
    auto help_image =
        std::make_unique<UIImage>("assets/textures/UI/instructions.png", glm::vec2(0.0f, 0.0f), window_size);

    ui_manager_->addElement(std::move(help_image));

    Scene::init();
    spdlog::trace("HelpsScene 初始化完成.");
}

void HelpsScene::handleInput() {
    if (!is_initialized_) return;

    // 检测是否按下鼠标左键
    if (context_.getInputManager().isActionPressed("MouseLeftClick")) {
        spdlog::debug("鼠标左键被按下, 退出 HelpsScene.");
        scene_manager_.requestPopScene();
    }
}

}  // namespace pyc::sunny_land
