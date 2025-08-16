#pragma once
#include "puzzle15.hpp"

namespace puzzle15 {

inline int misplaced_heuristic(const Puzzle& p) {
    int m = 0; // ミスプレースされたタイルの数
    for (int i = 0; i < 16; ++i) {
        if (p.get(i) != i + 1 && p.get(i) != 0) {
            ++m;
        }
    }
    return m;
}

inline int MDIST[16][16];

inline void init_manhattan_table() {
    for (int tile = 1; tile < 16; ++tile) {
        int tr = tile / 4; // タイルの目標行
        int tc = tile % 4; // タイルの目標列
        for (int pos = 0; pos < 16; ++pos) {
            int r = pos / 4; // 現在の行
            int c = pos % 4; // 現在の列
            MDIST[tile][pos] = std::abs(tr - r) + std::abs(tc - c);
        }
    }
}

inline int manhattan_heuristic(const Puzzle& p) {
    int d = 0; // マンハッタン距離
    for (int i = 0; i < 16; ++i) {
        if (p.get(i) != 0)  {
            int target_row = (p.get(i) - 1) / 4;
            int target_col = (p.get(i) - 1) % 4;
            int current_row = i / 4;
            int current_col = i % 4;
            d += abs(target_row - current_row) + abs(target_col - current_col);
        }
    }
    return d;
}

inline int manhattan_heuristic_fast(const Puzzle& p) {
    int d = 0;
    for (int pos = 0; pos < 16; ++pos) {
        uint8_t t = p.get(pos);
        if (t) d += MDIST[t][pos];
    }
    return d;
}

inline int manhattan_delta_for_move(int h, uint8_t t, int oldPos, int newPos) {
    return h - MDIST[t][oldPos] + MDIST[t][newPos];
}

inline int const_heuristic(const Puzzle& p) {
    return 0;
}

} // namespace puzzle15