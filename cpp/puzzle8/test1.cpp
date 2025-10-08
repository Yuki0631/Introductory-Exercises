#include <random>
#include <iostream>
#include "puzzle.hpp"
#include "generator.hpp"
#include "solver.hpp"

int main() {
    std::mt19937 rng(std::random_device{}()); // 乱数生成器

    // 1000個の盤面を生成してA* Searchを実行
    int num_tests = 1000;
    int min_len = 10;
    int max_len = 80;


    std::size_t generated_total = 0;
    long long elapsed_total = 0;
    std::size_t path_length_total = 0;

    auto goal = puzzle8::Puzzle::goal(); // 目標状態

    for (int i = 0; i < num_tests; ++i) {
        int steps = std::uniform_int_distribution<int>(min_len, max_len)(rng); // 10から40のランダムな手数
        puzzle8::Puzzle p = puzzle8::generate_random_puzzle(steps, std::nullopt);
        auto result = solver::A_star_path(p, goal, puzzle8::manhattan_heuristic);
        if (result.path) {

            // 解の経路が正しいか判定する
            if (!solver::validate_path(p, goal, *result.path, /*check_invariants_each_step=*/(i % 10 == 0))) {
                std::cerr << "[ERROR] path validation failed at i=" << i << "\n";
                return 1;
            }
            generated_total += result.generated;
            elapsed_total += result.elapsed_ms;
            path_length_total += result.path->size();
        }
    }

    std::cout << "The number of tests: " << num_tests << "\n";
    std::cout << "The range of moves (moves taken to generate the initial board state): " << "from " << min_len << " to " << max_len << "\n";
    std::cout << "Average generated nodes: " << (generated_total / num_tests) << "\n";
    std::cout << "Average elapsed time: " << (elapsed_total / num_tests) << " ms\n";
    std::cout << "Average path length: " << (path_length_total / num_tests) << "\n";

    return 0;
}