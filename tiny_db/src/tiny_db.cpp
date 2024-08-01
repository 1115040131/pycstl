#include <fmt/base.h>

#include "tiny_db/machine.h"

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        fmt::println("Must supply a database filename.");
        exit(EXIT_FAILURE);
    }

    tiny_db::Machine machine(argv[1]);
    machine.Start();
    return 0;
}
