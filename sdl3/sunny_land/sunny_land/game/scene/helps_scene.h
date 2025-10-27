#pragma once

#include "sunny_land/engine/scene/scene.h"

namespace pyc::sunny_land {

class SessionData;

/**
 * @brief 显示帮助信息的场景。
 *
 * 显示一张帮助图片，按鼠标左键退出。
 */
class HelpsScene final : public Scene {
public:
    /**
     * @brief 构造函数。
     * @param context 引擎上下文引用。
     * @param scene_manager 场景管理器引用。
     */
    HelpsScene(Context& context, SceneManager& scene_manager);

    // --- 核心方法 --- //
    void init() override;
    void handleInput() override;
};

}  // namespace pyc::sunny_land