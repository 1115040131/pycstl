// entt_base.cpp / entt_signal.cpp 原本靠 entt_example.cpp 里的 void entt_base(); 等
// 声明跨 TU 链接，模块化后必须由一个接口单元导出这四个入口。
export module monster_war.example.entt_demo;

export void entt_base();
export void entt_delegate();
export void entt_signal();
export void entt_dispatcher();
