#include <fmt/base.h>

void entt_base();
void entt_delegate();
void entt_signal();
void entt_dispatcher();

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