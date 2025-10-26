#include "sunny_land/game/scene/menu_scene.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/core/game_state.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/engine/scene/scene_manager.h"
#include "sunny_land/engine/ui/ui_button.h"
#include "sunny_land/engine/ui/ui_label.h"
#include "sunny_land/engine/ui/ui_manager.h"
#include "sunny_land/engine/utils/macro.h"
#include "sunny_land/game/data/session_data.h"
#include "sunny_land/game/scene/title_scene.h"

namespace pyc::sunny_land {

MenuScene::MenuScene(Context& context, SceneManager& scene_manager, std::shared_ptr<SessionData> session_data)
    : Scene("MenuScene", context, scene_manager), session_data_(std::move(session_data)) {
    if (!session_data_) {  // 如果没有传入SessionData，则创建一个默认的
        spdlog::info("未提供 SessionData, 使用默认值。");
    }
    spdlog::trace("MenuScene 构造完成。");
}

void MenuScene::init() {
    if (is_initialized_) {
        spdlog::warn("MenuScene 已经初始化过了，重复调用 init()。");
        return;
    }
    spdlog::trace("MenuScene 初始化开始...");
    context_.getGameState().setState(State::Paused);

    // 创建 UI 元素
    createUI();

    Scene::init();
    spdlog::trace("MenuScene 初始化完成.");
}

void MenuScene::handleInput() {  // 先让 UIManager 处理交互
    Scene::handleInput();

    // 检查暂停键，允许按暂停键恢复游戏
    if (context_.getInputManager().isActionPressed("pause")) {
        spdlog::debug("在菜单场景中按下暂停键，正在恢复游戏...");
        scene_manager_.requestPopScene();  // 弹出自身以恢复底层的GameScene
        context_.getGameState().setState(State::Playing);
    }
}

void MenuScene::createUI() {
    spdlog::trace("创建 MenuScene UI...");
    auto window_size = context_.getGameState().getLogicalSize();

    if (!ui_manager_->init(window_size)) {
        spdlog::error("初始化 UIManager 失败!");
        return;
    }

    // "PAUSE"标签
    auto pause_label =
        std::make_unique<UILabel>(context_.getTextRenderer(), "PAUSE", ASSET("fonts/VonwaonBitmap-16px.ttf"), 32);
    // 放在中间靠上的位置
    auto size = pause_label->getSize();
    auto label_y = window_size.y * 0.2;
    pause_label->setPosition(glm::vec2((window_size.x - size.x) / 2.0f, label_y));
    ui_manager_->addElement(std::move(pause_label));

    // --- 创建按钮 --- (4个按钮，设定好大小、间距)
    float button_width = 96.0f;  // 按钮稍微小一点
    float button_height = 32.0f;
    float button_spacing = 10.0f;
    float start_y = label_y + 80.0f;                         // 从标签下方开始，增加间距
    float button_x = (window_size.x - button_width) / 2.0f;  // 水平居中

    // Resume Button
    auto resume_button = std::make_unique<UIButton>(
        context_, ASSET("textures/UI/buttons/Resume1.png"), ASSET("textures/UI/buttons/Resume2.png"),
        ASSET("textures/UI/buttons/Resume3.png"), glm::vec2{button_x, start_y},
        glm::vec2{button_width, button_height}, [this]() { this->onResumeClicked(); });
    ui_manager_->addElement(std::move(resume_button));

    // Save Button
    start_y += button_height + button_spacing;
    auto save_button = std::make_unique<UIButton>(
        context_, ASSET("textures/UI/buttons/Save1.png"), ASSET("textures/UI/buttons/Save2.png"),
        ASSET("textures/UI/buttons/Save3.png"), glm::vec2{button_x, start_y},
        glm::vec2{button_width, button_height}, [this]() { this->onSaveClicked(); });
    ui_manager_->addElement(std::move(save_button));

    // Back Button
    start_y += button_height + button_spacing;
    auto back_button = std::make_unique<UIButton>(
        context_, ASSET("textures/UI/buttons/Back1.png"), ASSET("textures/UI/buttons/Back2.png"),
        ASSET("textures/UI/buttons/Back3.png"), glm::vec2{button_x, start_y},
        glm::vec2{button_width, button_height}, [this]() { this->onBackClicked(); });
    ui_manager_->addElement(std::move(back_button));

    // Quit Button
    start_y += button_height + button_spacing;
    auto quit_button = std::make_unique<UIButton>(
        context_, ASSET("textures/UI/buttons/Quit1.png"), ASSET("textures/UI/buttons/Quit2.png"),
        ASSET("textures/UI/buttons/Quit3.png"), glm::vec2{button_x, start_y},
        glm::vec2{button_width, button_height}, [this]() { this->onQuitClicked(); });
    ui_manager_->addElement(std::move(quit_button));

    spdlog::trace("MenuScene UI 创建完成.");
}

void MenuScene::onResumeClicked() {
    spdlog::debug("继续游戏按钮被点击。");
    scene_manager_.requestPopScene();  // 弹出当前场景
    context_.getGameState().setState(State::Playing);
}

void MenuScene::onSaveClicked() {
    spdlog::debug("保存游戏按钮被点击。");
    if (session_data_ && session_data_->saveToFile(ASSET("save.json"))) {
        spdlog::debug("菜单场景中成功保存游戏数据。");
    } else {
        spdlog::error("菜单场景中保存游戏数据失败。");
    }
}

void MenuScene::onBackClicked() {
    spdlog::debug("返回按钮被点击。弹出菜单场景和游戏场景，返回标题界面。");
    // 直接替换为TitleScene
    scene_manager_.requestReplaceScene(std::make_unique<TitleScene>(context_, scene_manager_, session_data_));
}

void MenuScene::onQuitClicked() {
    spdlog::debug("退出按钮被点击。请求应用程序退出。");
    context_.getInputManager().setShouldQuit(true);  // 输入管理器设置退出标志
}

}  // namespace pyc::sunny_land