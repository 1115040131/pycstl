#include "monster_war/game/scene/game_scene.h"

#include <entt/core/hashed_string.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/audio/audio_player.h"
#include "monster_war/engine/component/animation_component.h"
#include "monster_war/engine/component/sprite_component.h"
#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/component/velocity_component.h"
#include "monster_war/engine/core/context.h"
#include "monster_war/engine/resource/resource_manager.h"
#include "monster_war/engine/system/animation_system.h"
#include "monster_war/engine/system/movement_system.h"
#include "monster_war/engine/system/render_system.h"
#include "monster_war/engine/ui/ui_image.h"
#include "monster_war/engine/ui/ui_label.h"
#include "monster_war/engine/ui/ui_manager.h"
#include "monster_war/engine/utils/events.h"

namespace pyc::monster_war {

using namespace entt::literals;

GameScene::GameScene(Context& context) : Scene("GameScene", context) {
    // 初始化系统
    render_system_ = std::make_unique<RenderSystem>();
    movement_system_ = std::make_unique<MovementSystem>();
    animation_system_ = std::make_unique<AnimationSystem>();

    spdlog::trace("GameScene 构造完成。");
}

GameScene::~GameScene() = default;

void GameScene::init() {
    // 测试资源管理器
    testResourceManager();
    // 测试ECS
    testECS();

    Scene::init();
}

void GameScene::update(std::chrono::duration<float> delta_time) {
    movement_system_->update(registry_, delta_time);
    animation_system_->update(registry_, delta_time);

    Scene::update(delta_time);
}

void GameScene::render() {
    render_system_->update(registry_, context_.getRenderer(), context_.getCamera());

    Scene::render();
}

void GameScene::clean() { Scene::clean(); }

void GameScene::testResourceManager() {
    // 载入资源
    context_.getResourceManager().loadTexture("assets/textures/Buildings/Castle.png"_hs);
    // 播放音乐
    context_.getAudioPlayer().playMusic("battle_bgm"_hs);

    // 测试UI元素（使用载入的资源）
    ui_manager_->addElement(std::make_unique<UIImage>("assets/textures/Buildings/Castle.png"_hs));
    ui_manager_->addElement(std::make_unique<UILabel>(context_.getTextRenderer(), "Hello, World!",
                                                      "assets/fonts/VonwaonBitmap-16px.ttf"));
}

void GameScene::testECS() {
    auto entity = registry_.create();

    // 变换、速度、精灵组件
    registry_.emplace<TransformComponent>(entity, glm::vec2(100.0f));
    registry_.emplace<VelocityComponent>(entity, glm::vec2(10.0f));
    registry_.emplace<SpriteComponent>(entity, Sprite{
                                                   "assets/textures/Units/Archer.png",
                                                   Rect{glm::vec2(0.0f), glm::vec2(192.0f)},
                                               });

    // 动画组件 (单一动画 -> 动画map -> AnimationComponent)
    auto animation = Animation{std::vector<AnimationFrame>{
        AnimationFrame{Rect{glm::vec2(0.0f), glm::vec2(192.0f)}, 100ms},
        AnimationFrame{Rect{glm::vec2(192.0f, 0.0f), glm::vec2(192.0f)}, 100ms},
        AnimationFrame{Rect{glm::vec2(384.0f, 0.0f), glm::vec2(192.0f)}, 100ms},
        AnimationFrame{Rect{glm::vec2(576.0f, 0.0f), glm::vec2(192.0f)}, 100ms},
        AnimationFrame{Rect{glm::vec2(768.0f, 0.0f), glm::vec2(192.0f)}, 100ms},
        AnimationFrame{Rect{glm::vec2(960.0f, 0.0f), glm::vec2(192.0f)}, 100ms},
    }};
    auto animation_map = std::unordered_map<entt::id_type, Animation>{
        {"idle"_hs, std::move(animation)},
    };
    registry_.emplace<AnimationComponent>(entity, std::move(animation_map), "idle"_hs);
}

}  // namespace pyc::monster_war