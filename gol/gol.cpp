#include <cassert>
#include <chrono>
#include <fstream>
#include <thread>
#include <vector>

#include "tetris/terminal.h"

using namespace pyc::tetris;

std::vector<std::vector<int>> Load() {
    std::ifstream fs("hello_world/resource/base.map");
    assert(fs.is_open());

    std::vector<std::vector<int>> result;

    std::string line;
    while (std::getline(fs, line)) {
        int cols = line.size();
        static int s_cols = cols;
        assert(cols == s_cols);

        result.emplace_back();

        for (auto cell : line) {
            if (cell == '0') {
                result.back().push_back(0);
            } else {
                result.back().push_back(1);
            }
        }
    }

    return result;
}

void Update(std::vector<std::vector<int>>& field) {
    static constexpr int neighbours[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1},
    };

    const int kMaxRow = field.size();
    const int kMaxCol = field[0].size();

    auto new_field = field;

    for (int i = 0; i < kMaxRow; i++) {
        for (int j = 0; j < kMaxCol; j++) {
            int survival_heighbours = 0;
            for (auto [dx, dy] : neighbours) {
                int x = i + dx;
                int y = j + dy;
                if (x >= 0 && x < kMaxRow && y >= 0 && y < kMaxCol) {
                    survival_heighbours += field[x][y];
                }

                if (survival_heighbours < 2) {
                    new_field[i][j] = 0;
                } else if (survival_heighbours == 2) {
                    new_field[i][j] = field[i][j];
                } else if (survival_heighbours == 3) {
                    new_field[i][j] = 1;
                } else if (survival_heighbours > 3) {
                    new_field[i][j] = 0;
                }
            }
        }
    }

    field = std::move(new_field);
}

void Print(const std::vector<std::vector<int>>& field) {
    Terminal::GetInstance().Clear().SetBackgroundColor(ColorId::kWhite);
    for (unsigned int i = 0; i < field.size(); ++i) {
        for (unsigned int j = 0; j < field[i].size(); ++j) {
            if (field[i][j] == 1) {
                Terminal::GetInstance().MoveTo(i, 2 * j - 1).Output("  ");
            }
        }
    }
    Terminal::GetInstance().Reset().Output("\n").Flush();
}

int main() {
    std::vector<std::vector<int>> field = Load();

    constexpr size_t kFps = 5;
    constexpr auto kInterval =
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(1.0 / kFps));


    auto prev_time = std::chrono::steady_clock::now();
    while (true) {
        auto end_time = std::chrono::steady_clock::now();
        auto delta = end_time - prev_time;
        prev_time = end_time;

        Print(field);
        // for (auto& row : field) {
        //     for (auto& cell : row) {
        //         std::cout << cell << ' ';
        //     }
        //     std::cout << '\n';
        // }
        // std::cout << '\n';

        Update(field);

        while (delta > kInterval) {
            delta -= kInterval;
        }

        std::this_thread::sleep_for(kInterval - delta);
    }

    return 0;
}