#pragma once

#include <entt/entt.hpp>

struct position {
    float x;
    float y;
};

struct velocity {
    float dx;
    float dy;
};

// 增加一个新组件, 用于标记静态物体
struct immovable_tag {};

// 增加一个组件来存储实体的 "标签"
struct tag {
    // entt::hashed_string value;  // 只保留字符串的指针, 不保证字符串是否被销毁, 将来使用 value.data() 可能为空

    // 确保字符串不被销毁的正确做法: value 用于储存, id 用于比较/查询
    entt::id_type id;
    std::string value;
};