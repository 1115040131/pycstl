#include "monster_war/game/system/place_unit_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/audio/audio_player.h"
#include "monster_war/engine/component/name_component.h"
#include "monster_war/engine/component/render_component.h"
#include "monster_war/engine/component/sprite_component.h"
#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/core/context.h"
#include "monster_war/engine/input/input_manager.h"
#include "monster_war/engine/render/camera.h"
#include "monster_war/game/component/place_occupied_component.h"
#include "monster_war/game/component/unit_prep_component.h"
#include "monster_war/game/data/game_stats.h"
#include "monster_war/game/data/session_data.h"
#include "monster_war/game/def/events.h"
#include "monster_war/game/def/tag.h"
#include "monster_war/game/factory/entity_factory.h"

namespace pyc::monster_war {

using namespace entt::literals;

PlaceUnitSystem::PlaceUnitSystem(entt::registry& registry, EntityFactory& entity_factory, Context& context)
    : registry_(registry), entity_factory_(entity_factory), context_(context) {
    // 注册按键
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("mouse_right"_hs).connect<&PlaceUnitSystem::onCancelPrepUnit>(this);
    input_manager.onAction("mouse_left"_hs).connect<&PlaceUnitSystem::onPlaceUnit>(this);
    // 注册事件
    auto& dispatcher = context_.getDispatcher();
    dispatcher.sink<PrepUnitEvent>().connect<&PlaceUnitSystem::onPrepUnitEvent>(this);
    dispatcher.sink<RemovePlayerUnitEvent>().connect<&PlaceUnitSystem::onRemoveUnitEvent>(this);
}

PlaceUnitSystem::~PlaceUnitSystem() {
    // 断开按键
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("mouse_right"_hs).disconnect<&PlaceUnitSystem::onCancelPrepUnit>(this);
    input_manager.onAction("mouse_left"_hs).disconnect<&PlaceUnitSystem::onPlaceUnit>(this);
    // 断开事件
    context_.getDispatcher().disconnect(this);
}

void PlaceUnitSystem::update(std::chrono::duration<float>) {
    // 目标放置位置先置为null，只有找到了有效位置才会被赋值
    target_place_entity_ = entt::null;

    auto view = registry_.view<UnitPrepComponent, TransformComponent, RenderComponent>();
    // 虽然是循环，但拥有UnitPrepComponent的实体最多只有一个
    for (auto [entity, unit_prep, transform, render] : view.each()) {
        // 位置同步到到鼠标
        transform.position_ =
            context_.getCamera().screenToWorld(context_.getInputManager().getLogicalMousePosition());

        // 检查放置位置是否有效
        checkTargetPlace(transform.position_, unit_prep.type_);

        // 根据是否有效设置颜色
        target_place_entity_ != entt::null ? render.color_ = FColor::green() : render.color_ = FColor::red();
    }
}

void PlaceUnitSystem::checkTargetPlace(const glm::vec2& position, PlayerType player_type) {
    if (player_type == PlayerType::MELEE) {  // 检查是否处在近战可放置区域（拥有MeleePlaceTag的地点）
        auto melee_place_view = registry_.view<MeleePlaceTag, TransformComponent, SpriteComponent>(
            entt::exclude<PlaceOccupiedComponent>);
        for (auto [entity, transform, sprite] : melee_place_view.each()) {
            // 检测与放置区域中心的距离（Tiled中的参照点是左上角）
            auto center_position = transform.position_ + sprite.size_ * transform.scale_ / 2.0f;
            if (distanceSquared(position, center_position) < PLACE_RADIUS * PLACE_RADIUS) {
                target_place_entity_ = entity;
                return;
            }
        }
    } else if (player_type == PlayerType::RANGED) {  // 检查是否处在远程可放置区域（拥有RangedPlaceTag的地点）
        auto ranged_place_view = registry_.view<RangedPlaceTag, TransformComponent, SpriteComponent>(
            entt::exclude<PlaceOccupiedComponent>);
        for (auto [entity, transform, sprite] : ranged_place_view.each()) {
            auto center_position = transform.position_ + sprite.size_ * transform.scale_ / 2.0f;
            if (distanceSquared(position, center_position) < PLACE_RADIUS * PLACE_RADIUS) {
                target_place_entity_ = entity;
                return;
            }
        }
    }
}

void PlaceUnitSystem::onPrepUnitEvent(const PrepUnitEvent& event) {
    // 如果cost资源不够，直接返回
    auto& game_stats = registry_.ctx().get<GameStats&>();
    if (game_stats.cost_ < event.cost_) {
        return;
    }

    // 先移除其他单位准备类型实体
    onCancelPrepUnit();

    // 在鼠标所在的位置创建单位准备类型实体
    auto position = context_.getCamera().screenToWorld(context_.getInputManager().getLogicalMousePosition());
    entity_factory_.createUnitPrep(event.name_id_, event.class_id_, event.cost_, position);
    spdlog::info("创建单位准备类型实体: {}, pos: {}, {}", event.name_id_, position.x, position.y);
}

void PlaceUnitSystem::onRemoveUnitEvent(const RemovePlayerUnitEvent& event) {
    // 标记该单位为死亡
    registry_.emplace_or_replace<DeadTag>(event.entity_);
    // 检查所有被占用的地点，如果占用者是移除事件中的单位，则移除占用组件
    auto view = registry_.view<PlaceOccupiedComponent>();
    for (auto [entity, place_occupied] : view.each()) {
        if (place_occupied.entity_ == event.entity_) {
            registry_.remove<PlaceOccupiedComponent>(entity);
            spdlog::info("移除地点 {} 的占用组件", entt::to_integral(entity));
            break;
        }
    }
}

bool PlaceUnitSystem::onPlaceUnit() {
    // 目标放置位置有效才继续
    if (target_place_entity_ == entt::null) {
        return false;
    }

    // 获取目标位置坐标
    const auto& transform = registry_.get<TransformComponent>(target_place_entity_);
    const auto& sprite = registry_.get<SpriteComponent>(target_place_entity_);
    auto position = transform.position_ + sprite.size_ * transform.scale_ / 2.0f;
    // 获取单位信息
    auto unit_map = registry_.ctx().get<std::shared_ptr<SessionData>>()->getUnitMap();
    auto& game_stats = registry_.ctx().get<GameStats&>();
    auto view_prep = registry_.view<UnitPrepComponent>();
    // 循环只会进行一次，因为拥有UnitPrepComponent的实体最多只有一个
    for (auto [entity, unit_prep] : view_prep.each()) {
        auto& unit_data = unit_map[unit_prep.name_id_];
        // 创建单位
        auto unit_entity =
            entity_factory_.createPlayerUnit(unit_data.class_id_, position, unit_data.level_, unit_data.rarity_);
        registry_.emplace<NameComponent>(unit_entity, unit_data.name_id_, unit_data.name_);
        // 地点实体添加占用组件
        registry_.emplace<PlaceOccupiedComponent>(target_place_entity_, unit_entity);
        // 扣除费用
        game_stats.cost_ -= unit_prep.cost_;
        // 移除单位准备类型实体
        registry_.emplace_or_replace<DeadTag>(entity);

        // 通知UI移除对应肖像
        context_.getDispatcher().enqueue(RemoveUIPortraitEvent{unit_data.name_id_});

        // --- 渲染图层修正：确保玩家所在图层大于放置点图标的图层 ---
        const auto& render_place = registry_.get<RenderComponent>(target_place_entity_);
        // 正常情况下render_place.layer应该不会超过主图层（10），那么不做处理
        // 如果超过了，就让玩家所在图层 = 放置点图层 + 1
        if (render_place.layer > RenderComponent::kMainLayer) {
            auto& render_player = registry_.get<RenderComponent>(unit_entity);
            render_player.layer = render_place.layer + 1;
        }
    }
    // 播放放置音效
    context_.getAudioPlayer().playSound("unit_placed"_hs);
    return true;
}

bool PlaceUnitSystem::onCancelPrepUnit() {
    // 移除所有单位准备类型实体
    auto view = registry_.view<UnitPrepComponent>();
    for (auto entity : view) {
        registry_.emplace_or_replace<DeadTag>(entity);
        spdlog::info("移除单位准备类型实体: {}", entt::to_integral(entity));
    }
    return false;  // 让鼠标右键可以穿透
}

}  // namespace pyc::monster_war