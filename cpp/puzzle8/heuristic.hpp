#pragma once
#include "puzzle.hpp"

namespace puzzle8 {

inline int misplaced_heuristic(const Puzzle& p) {
    int m = 0;
    for (int i = 0; i < 9; ++i) {
        uint8_t v = get_nibble(p.board, i);
        if (v != 0 && v != i + 1) {
            ++m;
        }
    }
    return m;
}

inline int manhattan_heuristic(const Puzzle& p) {
    return static_cast<int>(p.hman); // 差分管理している値をそのまま返す
}

inline int const_heuristic(const Puzzle& p) {
    return 0;
}

} // namespace puzzle8