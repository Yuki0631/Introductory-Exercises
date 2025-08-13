#pragma once
#include <random>
#include <utility>
#include <vector>
#include "puzzle15.hpp"

namespace puzzle15 {
inline Puzzle generator(
    int n, // スライド操作の回数
    std::mt19937& rng, // メルセンヌツイスター (32bit) 乱数生成器
    bool avoid_backtrack = true // 直前の手を除外するか (デフォルト値は true)
) {
    Puzzle cur = Puzzle::goal(); // 現在の手
    if (n <= 0) return cur;

    std::optional<Puzzle::Move> last = std::nullopt; // 直前の手

    for (int step = 0; step < n; ++step) {
        // 合法手を列挙する
        std::vector<std::pair<Puzzle, Puzzle::Move>> neigh = cur.neighbors();

        // 直前手の反転を除外 (avoid_backtrack が true の場合)
        if (avoid_backtrack && last.has_value()) {
            const Puzzle::Move ban = cur.inverse(*last);
            std::vector<std::pair<Puzzle, Puzzle::Move>> filtered; // 直前手の反転を除外した手
            filtered.reserve(neigh.size());
            for (auto& pr : neigh) {
                if (pr.second != ban) filtered.push_back(std::move(pr));
            }
            if (!filtered.empty()) neigh.swap(filtered); // 全部が反転手ならそのまま
        }

        // ランダムに 1 手選ぶ
        std::uniform_int_distribution<std::size_t> dist(0, neigh.size() - 1); // 一様分布
        auto& pick = neigh[dist(rng)];

        last = pick.second;
        cur  = pick.first;
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

} // namespace puzzle15