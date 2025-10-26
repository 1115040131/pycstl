#include "sunny_land/game/scene/title_scene.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/audio/audio_player.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/core/game_state.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/engine/render/camera.h"
#include "sunny_land/engine/resource/resource_manager.h"
#include "sunny_land/engine/scene/level_loader.h"
#include "sunny_land/engine/scene/scene_manager.h"
#include "sunny_land/engine/ui/ui_button.h"
#include "sunny_land/engine/ui/ui_image.h"
#include "sunny_land/engine/ui/ui_label.h"
#include "sunny_land/engine/ui/ui_manager.h"
#include "sunny_land/engine/ui/ui_panel.h"
#include "sunny_land/game/data/session_data.h"
#include "sunny_land/game/scene/game_scene.h"
#include "sunny_land/game/scene/helps_scene.h"

namespace pyc::sunny_land {

TitleScene::TitleScene(Context& context, SceneManager& scene_manager, std::shared_ptr<SessionData> session_data)
    : Scene("TitleScene", context, scene_manager), session_data_(std::move(session_data)) {
    if (!session_data_) {  // 如果没有传入SessionData，则创建一个默认的
        session_data_ = std::make_shared<SessionData>();
        spdlog::info("未提供 SessionData, 使用默认值。");
    }
    spdlog::trace("TitleScene 构造完成。");
}

void TitleScene::init() {
    if (is_initialized_) {
        spdlog::warn("TitleScene 已经初始化过了，重复调用 init()。");
        return;
    }
    spdlog::trace("TitleScene 初始化开始...");
    context_.getGameState().setState(State::Title);
    session_data_->syncHighScore("assets/save.json");  // 更新最高分

    // 加载背景
    LevelLoader level_loader;
    if (!level_loader.loadLevel("assets/maps/level0.tmj", *this)) {
        spdlog::error("背景加载失败");
        return;
    }

    // 创建 UI 元素
    createUI();

    Scene::init();
    spdlog::trace("TitleScene 初始化完成.");
}

void TitleScene::update(std::chrono::duration<float> delta_time) {
    Scene::update(delta_time);

    // 相机自动向右移动
    context_.getCamera().move(glm::vec2(delta_time.count() * 100.0f, 0.0f));
}

void TitleScene::createUI() {
    spdlog::trace("创建 TitleScene UI...");
    auto window_size = context_.getGameState().getLogicalSize();

    if (!ui_manager_->init(window_size)) {
        spdlog::error("初始化 UIManager 失败!");
        return;
    }

    // 设置相机无边界
    context_.getCamera().setLimitBounds(std::nullopt);
    context_.getCamera().setPosition(glm::vec2(0.0f));

    // 设置背景音乐
    context_.getAudioPlayer().playMusic("assets/audio/platformer_level03_loop.ogg");

    // 创建标题图片 (假设不知道大小)
    auto title_image = std::make_unique<UIImage>("assets/textures/UI/title-screen.png");
    auto size = context_.getResourceManager().getTextureSize(title_image->getTextureId());
    title_image->setSize(size * 2.0f);  // 放大为2倍

    // 水平居中
    auto title_pos = (window_size - title_image->getSize()) / 2.0f - glm::vec2(0.0f, 50.0f);
    title_image->setPosition(title_pos);
    ui_manager_->addElement(std::move(title_image));

    // --- 创建按钮面板并居中 --- (4个按钮，设定好大小、间距)
    float button_width = 96.0f;
    float button_height = 32.0f;
    float button_spacing = 20.0f;
    int num_buttons = 4;

    // 计算面板总宽度
    float panel_width = num_buttons * button_width + (num_buttons - 1) * button_spacing;
    float panel_height = button_height;

    // 计算面板位置使其居中
    float panel_x = (window_size.x - panel_width) / 2.0f;
    float panel_y = window_size.y * 0.65f;  // 垂直位置中间靠下

    auto button_panel =
        std::make_unique<UIPanel>(glm::vec2(panel_x, panel_y), glm::vec2(panel_width, panel_height));

    // --- 创建按钮并添加到 UIPanel (位置是相对于 UIPanel 的 0,0) ---
    glm::vec2 current_button_pos = glm::vec2(0.0f, 0.0f);
    glm::vec2 button_size = glm::vec2(button_width, button_height);

    // Start Button
    auto start_button = std::make_unique<UIButton>(context_, "assets/textures/UI/buttons/Start1.png",
                                                   "assets/textures/UI/buttons/Start2.png",
                                                   "assets/textures/UI/buttons/Start3.png", current_button_pos,
                                                   button_size, [this]() { this->onStartGameClick(); });
    button_panel->addChild(std::move(start_button));

    // Load Button
    current_button_pos.x += button_width + button_spacing;
    auto load_button =
        std::make_unique<UIButton>(context_, "assets/textures/UI/buttons/Load1.png",
                                   "assets/textures/UI/buttons/Load2.png", "assets/textures/UI/buttons/Load3.png",
                                   current_button_pos, button_size, [this]() { this->onLoadGameClick(); });
    button_panel->addChild(std::move(load_button));

    // Helps Button
    current_button_pos.x += button_width + button_spacing;
    auto helps_button = std::make_unique<UIButton>(context_, "assets/textures/UI/buttons/Helps1.png",
                                                   "assets/textures/UI/buttons/Helps2.png",
                                                   "assets/textures/UI/buttons/Helps3.png", current_button_pos,
                                                   button_size, [this]() { this->onHelpsClick(); });
    button_panel->addChild(std::move(helps_button));

    // Quit Button
    current_button_pos.x += button_width + button_spacing;
    auto quit_button =
        std::make_unique<UIButton>(context_, "assets/textures/UI/buttons/Quit1.png",
                                   "assets/textures/UI/buttons/Quit2.png", "assets/textures/UI/buttons/Quit3.png",
                                   current_button_pos, button_size, [this]() { this->onQuitClick(); });
    button_panel->addChild(std::move(quit_button));

    // 将 UIPanel 添加到UI管理器
    ui_manager_->addElement(std::move(button_panel));

    // 创建 Credits 标签
    auto credits_label =
        std::make_unique<UILabel>(context_.getTextRenderer(), "SunnyLand Credits: XXX - 2025",
                                  "assets/fonts/VonwaonBitmap-16px.ttf", 16, FColor{0.8f, 0.8f, 0.8f, 1.0f});
    credits_label->setPosition(glm::vec2{(window_size.x - credits_label->getSize().x) / 2.0f,
                                         window_size.y - credits_label->getSize().y - 10.0f});
    ui_manager_->addElement(std::move(credits_label));

    spdlog::trace("TitleScene UI 创建完成.");
}

void TitleScene::onStartGameClick() {
    spdlog::debug("开始游戏按钮被点击。");
    if (session_data_) {
        session_data_->reset();
    }
    scene_manager_.requestReplaceScene(std::make_unique<GameScene>(context_, scene_manager_, session_data_));
}

void TitleScene::onLoadGameClick() {
    spdlog::debug("加载游戏按钮被点击。");
    if (!session_data_) {
        spdlog::error("游戏状态为空，无法加载。");
        return;
    }

    if (session_data_->loadFromFile("assets/save.json")) {
        spdlog::debug("保存文件加载成功。开始游戏...");
        scene_manager_.requestReplaceScene(std::make_unique<GameScene>(context_, scene_manager_, session_data_));
    } else {
        spdlog::warn("加载保存文件失败。");
    }
}

void TitleScene::onHelpsClick() {
    spdlog::debug("帮助按钮被点击。");
    scene_manager_.requestPushScene(std::make_unique<HelpsScene>(context_, scene_manager_));
}

void TitleScene::onQuitClick() {
    spdlog::debug("退出按钮被点击。");
    session_data_->syncHighScore("assets/save.json");  // 更新最高分
    context_.getInputManager().setShouldQuit(true);
}

}  // namespace pyc::sunny_land