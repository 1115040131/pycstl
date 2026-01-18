#include "monster_war/game/system/debug_ui_system.h"

#include <SDL3/SDL.h>
#include <entt/entity/registry.hpp>
#include <imgui/imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/name_component.h"
#include "monster_war/engine/core/context.h"
#include "monster_war/engine/core/game_state.h"
#include "monster_war/engine/render/renderer.h"
#include "monster_war/game/component/blocker_component.h"
#include "monster_war/game/component/class_name_component.h"
#include "monster_war/game/component/stats_component.h"

namespace pyc::monster_war {

using namespace entt::literals;

DebugUISystem::DebugUISystem(entt::registry& registry, Context& context)
    : registry_(registry), context_(context) {}

void DebugUISystem::update() {
    beginFrame();
    renderHoveredUnit();
    renderSelectedUnit();
    endFrame();
}

void DebugUISystem::beginFrame() {
    // 开始新帧
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // 关闭逻辑分辨率 (ImGui目前对于SDL逻辑分辨率支持不好，所以使用时先关闭)
    if (!context_.getGameState().disableLogicalPresentation()) {
        spdlog::error("关闭逻辑分辨率失败");
    }
}

void DebugUISystem::endFrame() {
    // ImGui: 渲染
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), context_.getRenderer().getSDLRenderer());

    // 渲染完成后，打开(恢复)逻辑分辨率
    if (!context_.getGameState().enableLogicalPresentation()) {
        spdlog::error("启用逻辑分辨率失败");
    }
}

void DebugUISystem::renderHoveredUnit() {
    // 确定鼠标悬浮的单位存在
    auto& entity = registry_.ctx().get<entt::entity&>("hovered_unit"_hs);
    if (entity == entt::null || !registry_.valid(entity)) {
        return;
    }

    // Tooltip 是悬浮在鼠标上的小窗口，可以显示单位信息
    if (!ImGui::BeginTooltip()) {
        ImGui::EndTooltip();
        spdlog::error("鼠标悬浮单位窗口打开失败");
        return;
    }
    // 获取必要信息并显示
    const auto& stats = registry_.get<StatsComponent>(entity);
    const auto& class_name = registry_.get<ClassNameComponent>(entity);
    // 只有玩家单位才有姓名，所以需要尝试获取
    if (auto name = registry_.try_get<NameComponent>(entity)) {
        ImGui::Text("%s  ", name->name_.c_str());
        ImGui::SameLine();
    }
    ImGui::Text("%s", class_name.class_name_.c_str());
    ImGui::Text("等级: %d", stats.level_);
    ImGui::SameLine();
    ImGui::Text("稀有度: %d", stats.rarity_);
    ImGui::Text("生命值: %d/%d", static_cast<int>(std::round(stats.hp_)),
                static_cast<int>(std::round(stats.max_hp_)));
    ImGui::Text("攻击力: %d", static_cast<int>(std::round(stats.atk_)));
    ImGui::Text("防御力: %d", static_cast<int>(std::round(stats.def_)));
    ImGui::Text("攻击范围: %d", static_cast<int>(std::round(stats.range_)));
    ImGui::Text("攻击间隔: %.2f", stats.atk_interval_.count());
    ImGui::EndTooltip();
}

void DebugUISystem::renderSelectedUnit() {
    // 确定选中的单位存在
    auto& entity = registry_.ctx().get<entt::entity&>("selected_unit"_hs);
    if (entity == entt::null || !registry_.valid(entity)) {
        return;
    }

    // 设置窗口位置在左上角
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);

    if (!ImGui::Begin("角色状态", nullptr, ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        spdlog::error("角色状态窗口打开失败");
        return;
    }
    // 获取必要信息并显示
    const auto& stats = registry_.get<StatsComponent>(entity);
    const auto& class_name = registry_.get<ClassNameComponent>(entity);
    const auto& blocker = registry_.try_get<BlockerComponent>(entity);
    if (auto name = registry_.try_get<NameComponent>(entity); name) {
        ImGui::Text("%s  ", name->name_.c_str());
        ImGui::SameLine();
    }
    ImGui::Text("%s", class_name.class_name_.c_str());
    ImGui::Text("等级: %d", stats.level_);
    ImGui::SameLine();
    ImGui::Text("稀有度: %d", stats.rarity_);
    ImGui::Text("生命值: %d/%d", static_cast<int>(std::round(stats.hp_)),
                static_cast<int>(std::round(stats.max_hp_)));
    ImGui::Text("攻击力: %d", static_cast<int>(std::round(stats.atk_)));
    ImGui::SameLine();
    ImGui::Text("防御力: %d", static_cast<int>(std::round(stats.def_)));
    ImGui::Text("攻击范围: %d", static_cast<int>(std::round(stats.range_)));
    ImGui::SameLine();
    ImGui::Text("攻击间隔: %.2f", stats.atk_interval_.count());
    if (blocker) {
        ImGui::Text("阻挡数量: %d/%d", blocker->current_count_, blocker->max_count_);
    }
    // TODO: 技能相关按钮与信息
    ImGui::End();
}

}  // namespace pyc::monster_war