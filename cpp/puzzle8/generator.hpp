#pragma once
#include <random>
#include <utility>
#include <vector>
#include "puzzle.hpp"

namespace puzzle8 {

inline Puzzle generator(
    int n,
    std::mt19937& rng,
    bool avoid_backtrack = true
) {
    Puzzle cur = Puzzle::goal();
    if (n <= 0) return cur;

    // 直前の逆戻りを避ける optional
    std::optional<Puzzle::Move> last = std::nullopt;

    for (int i = 0; i < n; ++i) {
        std::vector<Puzzle::Move> moves;
        moves.reserve(4);
        for (Puzzle::Move m : {Puzzle::Move::Up, Puzzle::Move::Down, Puzzle::Move::Left, Puzzle::Move::Right}) {
            if (!Puzzle::can_move(cur.zero_pos, m)) {
                continue;
            }
            if (avoid_backtrack && last && m == Puzzle::inverse(*last)) {
                continue;
            }
            moves.push_back(m);
        }
        if (moves.empty()) {
            // 逆戻り抑止で詰んだ場合は許可する
            for (Puzzle::Move m : {Puzzle::Move::Up, Puzzle::Move::Down, Puzzle::Move::Left, Puzzle::Move::Right}) {
                if (Puzzle::can_move(cur.zero_pos, m)) {
                    moves.push_back(m);
                }
            }
        }
        std::uniform_int_distribution<int> dist(0, (int)moves.size()-1);
        auto m = moves[dist(rng)];
        cur.move_inplace(m);
        last = m;
    }
    return cur;
}

inline Puzzle generate_random_puzzle(
    int n,
    std::optional<std::uint32_t> seed, // 乱数生成のためのシード
    bool avoid_backtrack = true
) {
    if (seed.has_value()) {
        std::mt19937 rng(*seed);
        return generator(n, rng, avoid_backtrack);
    } else {
        std::random_device rd; // 非決定的な乱数生成器
        std::mt19937 rng(rd());
        return generator(n, rng, avoid_backtrack);
    }
}

} // namespace puzzle8