#include "monster_war/game/system/selection_system.h"

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/core/context.h"
#include "monster_war/engine/input/input_manager.h"
#include "monster_war/engine/utils/math.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/component/player_component.h"
#include "monster_war/game/def/constants.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

using namespace entt::literals;

SelectionSystem::SelectionSystem(entt::registry& registry, Context& context)
    : registry_(registry), context_(context) {
    context_.getInputManager().onAction("mouse_left"_hs).connect<&SelectionSystem::onMouseLeftClick>(this);
    context_.getInputManager().onAction("mouse_right"_hs).connect<&SelectionSystem::onMouseRightClick>(this);
}

SelectionSystem::~SelectionSystem() {
    context_.getInputManager().onAction("mouse_left"_hs).disconnect<&SelectionSystem::onMouseLeftClick>(this);
    context_.getInputManager().onAction("mouse_right"_hs).disconnect<&SelectionSystem::onMouseRightClick>(this);
}

void SelectionSystem::update() {
    auto mouse_pos = context_.getInputManager().getLogicalMousePosition();
    // 优先判断玩家单位
    auto view_player = registry_.view<PlayerComponent, TransformComponent>();
    for (auto [entity, player, transform] : view_player.each()) {
        if (distanceSquared(transform.position_, mouse_pos) <= HOVER_RADIUS * HOVER_RADIUS) {
            registry_.ctx().get<entt::entity&>("hovered_unit"_hs) = entity;
            return;  // 找到悬浮单位，直接返回
        }
    }
    // 如果玩家单位没有被选中，再判断敌方单位
    auto view_enemy = registry_.view<EnemyComponent, TransformComponent>();
    for (auto [entity, enemy, transform] : view_enemy.each()) {
        if (distanceSquared(transform.position_, mouse_pos) <= HOVER_RADIUS * HOVER_RADIUS) {
            registry_.ctx().get<entt::entity&>("hovered_unit"_hs) = entity;
            return;
        }
    }
    // 如果都没有被悬浮，则不悬浮任何单位
    registry_.ctx().get<entt::entity&>("hovered_unit"_hs) = entt::null;
}

void SelectionSystem::clearCurrentSelection() {
    auto selected_unit = registry_.ctx().get<entt::entity&>("selected_unit"_hs);
    if (selected_unit != entt::null && registry_.valid(selected_unit)) {
        registry_.remove<ShowRangeTag>(selected_unit);
    }
    registry_.ctx().get<entt::entity&>("selected_unit"_hs) = entt::null;
}

bool SelectionSystem::onMouseLeftClick() {
    auto hovered_unit = registry_.ctx().get<entt::entity&>("hovered_unit"_hs);
    if (hovered_unit == entt::null || !registry_.valid(hovered_unit)) {
        return false;
    }
    // 如果鼠标悬浮单位是玩家，则选中单位，并清除之前选中的单位
    if (auto player = registry_.try_get<PlayerComponent>(hovered_unit); player) {
        clearCurrentSelection();
        registry_.ctx().get<entt::entity&>("selected_unit"_hs) = hovered_unit;
        // 添加范围显示标签
        registry_.emplace_or_replace<ShowRangeTag>(hovered_unit);
        return true;
    }
    return false;
}

bool SelectionSystem::onMouseRightClick() {
    clearCurrentSelection();
    return false;  // 让鼠标右键可以穿透
}

}  // namespace pyc::monster_war