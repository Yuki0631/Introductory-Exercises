#include <random>
#include <iostream>
#include "puzzle.hpp"
#include "generator.hpp"
#include "solver.hpp"

int main() {
    std::mt19937 rng(std::random_device{}()); // 乱数生成器

    // 1000個の盤面を生成してA* Searchを実行
    int num_tests = 1000;
    std::size_t generated_total = 0;
    long long elapsed_total = 0;
    std::size_t path_length_total = 0;

    auto goal = puzzle8::Puzzle::goal(); // 目標状態

    for (int i = 0; i < num_tests; ++i) {
        int steps = std::uniform_int_distribution<int>(10, 40)(rng); // 10から40のランダムな手数
        puzzle8::Puzzle p = puzzle8::generate_random_puzzle(steps, std::nullopt);
        auto result = solver::A_star_path(p, goal, puzzle8::manhattan_heuristic);
        if (result.path) {
            generated_total += result.generated;
            elapsed_total += result.elapsed_ms;
            path_length_total += result.path->size();
        }
    }

    std::cout << "Average generated nodes: " << (generated_total / num_tests) << "\n";
    std::cout << "Average elapsed time: " << (elapsed_total / num_tests) << " ms\n";
    std::cout << "Average path length: " << (path_length_total / num_tests) << "\n";

    return 0;
}