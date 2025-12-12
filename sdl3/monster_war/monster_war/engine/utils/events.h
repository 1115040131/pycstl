#pragma once

#include <memory>

namespace pyc::monster_war {

class Scene;

struct QuitEvent {};  // 退出事件

struct PopSceneEvent {};  // 弹出场景事件
struct PushSceneEvent {   // 压入场景事件
    std::unique_ptr<Scene> scene;
};
struct ReplaceSceneEvent {  // 替换场景事件
    std::unique_ptr<Scene> scene;
};

}  // namespace pyc::monster_war