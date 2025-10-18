#include "sunny_land/game/scene/game_scene.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/audio/audio_player.h"
#include "sunny_land/engine/component/animation_component.h"
#include "sunny_land/engine/component/collider_component.h"
#include "sunny_land/engine/component/health_component.h"
#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/component/sprite_component.h"
#include "sunny_land/engine/component/tilelayer_component.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/physics/physics_engine.h"
#include "sunny_land/engine/render/animation.h"
#include "sunny_land/engine/render/camera.h"
#include "sunny_land/engine/render/text_renderer.h"
#include "sunny_land/engine/scene/level_loader.h"
#include "sunny_land/engine/scene/scene_manager.h"
#include "sunny_land/engine/utils/macro.h"
#include "sunny_land/game/component/ai/jump_behavior.h"
#include "sunny_land/game/component/ai/patrol_behavior.h"
#include "sunny_land/game/component/ai/updown_behavior.h"
#include "sunny_land/game/component/ai_component.h"
#include "sunny_land/game/component/player_component.h"
#include "sunny_land/game/data/session_data.h"

namespace pyc::sunny_land {

using namespace std::chrono_literals;

GameScene::GameScene(Context& context, SceneManager& scene_manager, std::shared_ptr<SessionData> session_data)
    : Scene("GameScene", context, scene_manager), game_session_data_(std::move(session_data)) {
    if (!game_session_data_) {  // 如果没有传入SessionData，则创建一个默认的
        game_session_data_ = std::make_shared<SessionData>();
        spdlog::info("未提供 SessionData, 使用默认值。");
    }
    spdlog::trace("GameScene 构造完成。");
}

void GameScene::init() {
    if (is_initialized_) {
        spdlog::warn("GameScene 已经初始化过了，重复调用 init()。");
        return;
    }
    spdlog::trace("GameScene 初始化开始...");

    if (!initLevel()) {
        spdlog::error("关卡初始化失败，无法继续。");
        context_.getInputManager().setShouldQuit(true);
        return;
    }
    if (!initPlayer()) {
        spdlog::error("玩家初始化失败，无法继续。");
        context_.getInputManager().setShouldQuit(true);
        return;
    }
    if (!initEnemyAndItem()) {
        spdlog::error("敌人和道具初始化失败，无法继续。");
        context_.getInputManager().setShouldQuit(true);
        return;
    }

    Scene::init();
    spdlog::trace("GameScene 初始化完成。");
}

void GameScene::handleInput() { Scene::handleInput(); }

void GameScene::update(std::chrono::duration<float> delta_time) {
    Scene::update(delta_time);
    handleObjectCollisions();
    handleTileTriggers();
}

void GameScene::render() {
    Scene::render();
    testTextRender();
}

void GameScene::clean() { Scene::clean(); }

bool GameScene::initLevel() {
    // 加载关卡
    LevelLoader level_loader;
    auto level_path = levelNameToPath(game_session_data_->getMapPath());
    if (!level_loader.loadLevel(level_path, *this)) {
        spdlog::error("关卡加载失败");
        return false;
    }

    // 注册 main 层到物理引擎
    auto main_layer = findGameObjectByName("main");
    if (!main_layer) {
        spdlog::error("未找到\"main\"层");
        return false;
    }
    auto tile_layer = main_layer->getComponent<TileLayerComponent>();
    if (!tile_layer) {
        spdlog::error("\"main\"层没有 TileLayerComponent 组件");
    }
    context_.getPhysicsEngine().registerCollisionLayer(tile_layer);
    spdlog::info("注册\"main\"层到物理引擎");

    // 设置相机边界
    context_.getCamera().setLimitBounds(Rect{glm::vec2(0.0f), tile_layer->getWorldSize()});

    // 设置世界边界
    context_.getPhysicsEngine().setWorldBounds(Rect{glm::vec2(0.0f), tile_layer->getWorldSize()});

    // 设置音量
    context_.getAudioPlayer().setMusicVolume(0.2f);
    context_.getAudioPlayer().setSoundVolume(0.5f);
    // 播放背景音乐
    context_.getAudioPlayer().playMusic(ASSET("audio/hurry_up_and_run.ogg"), true, 1s);

    spdlog::trace("关卡初始化完成。");
    return true;
}

bool GameScene::initPlayer() {
    // 获取玩家对象
    player_ = findGameObjectByName("player");
    if (!player_) {
        spdlog::error("未找到玩家对象");
        return false;
    }

    // 添加PlayerComponent到玩家对象
    auto player_component = player_->addComponent<PlayerComponent>();
    if (!player_component) {
        spdlog::error("无法添加 PlayerComponent 到玩家对象");
        return false;
    }

    // 相机跟随玩家
    auto player_transform = player_->getComponent<TransformComponent>();
    if (!player_transform) {
        spdlog::error("玩家对象没有 TransformComponent 组件, 无法设置相机目标");
        return false;
    }
    context_.getCamera().setTarget(player_transform);
    spdlog::trace("Player初始化完成。");
    return true;
}

static bool objectPlayerAnimation(std::unique_ptr<GameObject>& game_object, std::string_view animation_name) {
    if (auto ac = game_object->getComponent<AnimationComponent>()) {
        ac->playAnimation(animation_name);
        return true;
    } else {
        spdlog::error("{} 对象缺少 AnimationComponent, 无法播放动画。", game_object->getName());
        return false;
    }
}

bool GameScene::initEnemyAndItem() {
    bool success = true;
    for (auto& game_object : game_objects_) {
        if (game_object->getName() == "eagle") {
            if (auto ai_component = game_object->addComponent<AIComponent>()) {
                auto y_max = game_object->getComponent<TransformComponent>()->getPosition().y;
                auto y_min = y_max - 80.0f;  // 让鹰的飞行范围 (当前位置与上方80像素 的区域)
                ai_component->setBehavior(std::make_unique<UpDownBehavior>(y_min, y_max));
            }
        } else if (game_object->getName() == "frog") {
            if (auto ai_component = game_object->addComponent<AIComponent>()) {
                auto x_max = game_object->getComponent<TransformComponent>()->getPosition().x - 10.0f;
                auto x_min = x_max - 90.0f;  // 青蛙跳跃范围（右侧 - 10.0f 是为了增加稳定性）
                ai_component->setBehavior(std::make_unique<JumpBehavior>(x_min, x_max));
            }
        } else if (game_object->getName() == "opossum") {
            if (auto ai_component = game_object->addComponent<AIComponent>()) {
                auto x_max = game_object->getComponent<TransformComponent>()->getPosition().x;
                auto x_min = x_max - 200.0f;  // 负鼠巡逻范围
                ai_component->setBehavior(std::make_unique<PatrolBehavior>(x_min, x_max));
            }
        } else if (game_object->getTag() == "item") {
            success = success && objectPlayerAnimation(game_object, "idle");
        }
    }
    return success;
}

void GameScene::handleObjectCollisions() {
    // 从物理引擎中获取碰撞对
    for (const auto& [obj1, obj2] : context_.getPhysicsEngine().getCollisionPairs()) {
        // 处理玩家与敌人的碰撞
        if (obj1->getName() == "player" && obj2->getTag() == "enemy") {
            playerVSEnemyCollision(obj1, obj2);
        } else if (obj2->getName() == "player" && obj1->getTag() == "enemy") {
            playerVSEnemyCollision(obj2, obj1);
        }
        // 处理玩家与道具的碰撞
        else if (obj1->getName() == "player" && obj2->getTag() == "item") {
            playerVSItemCollision(obj1, obj2);
        } else if (obj2->getName() == "player" && obj1->getTag() == "item") {
            playerVSItemCollision(obj2, obj1);
        }
        // 处理玩家与"hazard"对象碰撞
        else if (obj1->getName() == "player" && obj2->getTag() == "hazard") {
            obj1->getComponent<PlayerComponent>()->takeDamage(1);
            spdlog::debug("玩家 {} 受到了 HAZARD 对象伤害", obj1->getName());
        } else if (obj2->getName() == "player" && obj1->getTag() == "hazard") {
            obj2->getComponent<PlayerComponent>()->takeDamage(1);
            spdlog::debug("玩家 {} 受到了 HAZARD 对象伤害", obj2->getName());
        }
        // 处理玩家与关底触发器碰撞
        else if (obj1->getName() == "player" && obj2->getTag() == "next_level") {
            toNextLevel(obj2);
        } else if (obj2->getName() == "player" && obj1->getTag() == "next_level") {
            toNextLevel(obj1);
        }
    }
}

void GameScene::handleTileTriggers() {
    for (const auto& [object, tile_type] : context_.getPhysicsEngine().getTileTriggerEvents()) {
        if (tile_type == TileType::HAZARD) {
            // 玩家碰到到危险瓦片，受伤
            if (object->getName() == "player") {
                handlePlayerDamage(1);
                spdlog::debug("玩家 {} 受到了 HAZARD 瓦片伤害", object->getName());
            }
        }
    }
}

void GameScene::handlePlayerDamage(int damage) {
    auto player_component = player_->getComponent<PlayerComponent>();
    if (!player_component->takeDamage(damage)) {
        return;  // 没有受伤, 直接返回
    }
    if (player_component->isDead()) {
        spdlog::info("玩家 {} 死亡", player_->getName());
        // TODO: 可能的死亡逻辑处理
    }
    // 更新游戏数据
    game_session_data_->setCurrentHealth(player_component->getHealthComponent()->getCurrentHealth());
}

void GameScene::playerVSEnemyCollision(GameObject* player, GameObject* enemy) {
    // --- 踩踏判断逻辑：1. 玩家中心点在敌人上方    2. 重叠区域：overlap.x > overlap.y
    auto player_aabb = player->getComponent<ColliderComponent>()->getWorldAABB();
    auto enemy_aabb = enemy->getComponent<ColliderComponent>()->getWorldAABB();
    auto player_center = player_aabb.position + player_aabb.size / 2.0f;
    auto enemy_center = enemy_aabb.position + enemy_aabb.size / 2.0f;
    auto overlap =
        glm::vec2(player_aabb.size / 2.0f + enemy_aabb.size / 2.0f) - glm::abs(player_center - enemy_center);

    // 踩踏判断成功，敌人受伤
    if (overlap.x > overlap.y && player_center.y < enemy_center.y) {
        spdlog::info("玩家 {} 踩踏了敌人 {}", player->getName(), enemy->getName());
        auto enemy_health = enemy->getComponent<HealthComponent>();
        if (!enemy_health) {
            spdlog::error("敌人 {} 没有 HealthComponent 组件，无法处理踩踏伤害", enemy->getName());
            return;
        }
        enemy_health->takeDamage(1);  // 造成1点伤害
        if (!enemy_health->isAlive()) {
            spdlog::info("敌人 {} 被踩踏后死亡", enemy->getName());
            enemy->setNeedRemove(true);                   // 标记敌人为待删除状态
            createEffect(enemy_center, enemy->getTag());  // 创建（死亡）特效
        }
        // 玩家跳起效果
        player->getComponent<PhysicsComponent>()->setVelocityY(-300.0f);  // 向上跳起
        // 播放音效(此音效完全可以放在玩家的音频组件中，这里示例另一种用法：直接用AudioPlayer播放，传入文件路径)
        context_.getAudioPlayer().playSound(ASSET("audio/punch2a.mp3"));
        // 加分
        game_session_data_->addScore(10);
    }
    // 踩踏判断失败，玩家受伤
    else {
        spdlog::info("敌人 {} 对玩家 {} 造成伤害", enemy->getName(), player->getName());
        handlePlayerDamage(1);
        // TODO: 其他受伤逻辑
    }
}

void GameScene::playerVSItemCollision(GameObject* player, GameObject* item) {
    if (item->getName() == "fruit") {
        player->getComponent<HealthComponent>()->heal(1);  // 恢复1点生命值
    } else if (item->getName() == "gem") {
        game_session_data_->addScore(5);
    }
    item->setNeedRemove(true);
    auto item_aabb = item->getComponent<ColliderComponent>()->getWorldAABB();
    createEffect(item_aabb.position + item_aabb.size / 2.0f, item->getTag());  // 创建特效
    context_.getAudioPlayer().playSound(ASSET("audio/poka01.mp3"));            // 播放音效
}

void GameScene::toNextLevel(GameObject* trigger) {
    auto level_name = trigger->getName();
    game_session_data_->setNextLevel(level_name);
    auto next_scene = std::make_unique<GameScene>(context_, scene_manager_, game_session_data_);
    scene_manager_.requestReplaceScene(std::move(next_scene));
}

void GameScene::createEffect(glm::vec2 center_pos, std::string_view tag) {
    // --- 创建游戏对象和变换组件 ---
    auto effect_obj = std::make_unique<GameObject>(fmt::format("effect_{}", tag));
    effect_obj->addComponent<TransformComponent>(center_pos);

    // --- 根据标签创建不同的精灵组件和动画---
    auto animation = std::make_unique<Animation>("effect", false);
    if (tag == "enemy") {
        effect_obj->addComponent<SpriteComponent>(ASSET("textures/FX/enemy-deadth.png"),
                                                  context_.getResourceManager(), Alignment::CENTER);
        for (auto i = 0; i < 6; ++i) {
            animation->addFrame({static_cast<float>(i * 40), 0.0f, 40.0f, 41.0f}, 0.1s);
        }
    } else if (tag == "item") {
        effect_obj->addComponent<SpriteComponent>(ASSET("textures/FX/item-feedback.png"),
                                                  context_.getResourceManager(), Alignment::CENTER);
        for (auto i = 0; i < 5; ++i) {
            animation->addFrame({static_cast<float>(i * 32), 0.0f, 32.0f, 32.0f}, 0.1s);
        }
    } else {
        spdlog::warn("未知特效类型: {}", tag);
        return;
    }

    // --- 根据创建的动画，添加动画组件，并设置为单次播放 ---
    auto* animation_component = effect_obj->addComponent<AnimationComponent>();
    animation_component->addAnimation(std::move(animation));
    animation_component->setOneShotRemoval(true);
    animation_component->playAnimation("effect");

    safeAddGameObject(std::move(effect_obj));  // 安全添加特效对象
    spdlog::debug("创建特效: {}", tag);
}

void GameScene::testTextRender() {
    auto& text_renderer = context_.getTextRenderer();
    const auto& camera = context_.getCamera();
    text_renderer.drawUIText("UI Text", ASSET("fonts/VonwaonBitmap-16px.ttf"), 32, glm::vec2(100.0f),
                             {0, 1.0f, 0, 1.0f});
    text_renderer.drawText(camera, "Map Text", ASSET("fonts/VonwaonBitmap-16px.ttf"), 32, glm::vec2(200.0f));
}

}  // namespace pyc::sunny_land