#include <random>
#include <iostream>
#include "puzzle.hpp"
#include "generator.hpp"
#include "solver.hpp"

int main() {
    // 盤面の生成と表示
    std::mt19937 rng(std::random_device{}()); // 乱数生成器
    puzzle8::Puzzle p = puzzle8::generate_random_puzzle(20, std::nullopt); // 20 手ランダム
    std::cout << p.to_string() << "\n";

    // A* Search の実行
    auto goal = puzzle8::Puzzle::goal(); // 目標状態
    auto result = solver::A_star_path(p, goal, puzzle8::manhattan_heuristic);
    if (result.path) {
        std::cout << "Found solution with A* Search:\n";
        for (const auto& move : *result.path) {
            std::cout << " - " << solver::move_to_string(move) << "\n";
        }
        std::cout << "Total moves: " << result.path->size() << "\n";
        std::cout << "Generated nodes: " << result.generated << "\n";
        std::cout << "Elapsed time: " << result.elapsed_ms << " ms\n";
    } else {
        std::cout << "No solution found with A* Search.\n";
    }
}