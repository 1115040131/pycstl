#include <fmt/base.h>

import monster_war.example.entt_demo;

int main() {
    fmt::println("=== entt_base ===");
    entt_base();

    fmt::println("");
    fmt::println("=== entt_delegate ===");
    entt_delegate();

    fmt::println("");
    fmt::println("=== entt_signal ===");
    entt_signal();

    fmt::println("");
    fmt::println("=== entt_dispatcher ===");
    entt_dispatcher();
}
