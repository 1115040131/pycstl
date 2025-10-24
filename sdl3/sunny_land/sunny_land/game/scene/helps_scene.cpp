#include "sunny_land/game/scene/helps_scene.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/engine/scene/scene_manager.h"
#include "sunny_land/engine/ui/ui_image.h"
#include "sunny_land/engine/ui/ui_manager.h"
#include "sunny_land/engine/utils/macro.h"

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
        std::make_unique<UIImage>(ASSET("textures/UI/instructions.png"), glm::vec2(0.0f, 0.0f), window_size);

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