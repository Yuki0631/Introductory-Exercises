#pragma once
#include "puzzle.hpp"

namespace puzzle8 {

inline int misplaced_heuristic(const Puzzle& p) {
    int m = 0; // ミスプレースされたタイルの数
    for (int i = 0; i < 9; ++i) {
        if (p.tiles[i] != i + 1 && p.tiles[i] != 0) {
            ++m;
        }
    }
    return m;
}

inline int manhattan_heuristic(const Puzzle& p) {
    int d = 0; // マンハッタン距離
    for (int i = 0; i < 9; ++i) {
        if (p.tiles[i] != 0)  {
            int target_row = (p.tiles[i] - 1) / 3;
            int target_col = (p.tiles[i] - 1) % 3;
            int current_row = i / 3;
            int current_col = i % 3;
            d += abs(target_row - current_row) + abs(target_col - current_col);
        }
    }
    return d;
}

inline int const_heuristic(const Puzzle& p) {
    return 0;
}

} // namespace puzzle8