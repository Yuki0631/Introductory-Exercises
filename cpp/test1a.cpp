#include <random>
#include <iostream>
#include <vector>
#include <algorithm>
#include "puzzle15.hpp"
#include "generator15.hpp"
#include "solver15.hpp"

int main() {
    std::mt19937 rng(std::random_device{}()); // 乱数生成器
    puzzle15::init_manhattan_table(); // マンハッタン距離のテーブルを初期化

    // 1000個の盤面を生成してA* Searchを実行
    int num_tests = 100;
    std::size_t generated_total = 0;
    long long elapsed_total = 0;
    std::size_t path_length_total = 0;

    std::vector<long long> elapsed_list;

    auto goal = puzzle15::Puzzle::goal(); // 目標状態

    for (int i = 0; i < num_tests; ++i) {
        int steps = std::uniform_int_distribution<int>(10, 40)(rng); // 10から40のランダムな手数
        puzzle15::Puzzle p = puzzle15::generate_random_puzzle(steps, std::nullopt);
        auto result = solver15::A_star_path(p, goal);
        if (result.path) {
            generated_total += result.generated;
            elapsed_total += result.elapsed_ms;
            path_length_total += result.path->size();

            elapsed_list.push_back(result.elapsed_ms);
        }
    }

    std::cout << "Average generated nodes: " << (generated_total / num_tests) << "\n";
    std::cout << "Average elapsed time: " << (elapsed_total / num_tests) << " ms\n";
    std::cout << "Average path length: " << (path_length_total / num_tests) << "\n";

    std::sort(elapsed_list.begin(), elapsed_list.end());
    long long median = elapsed_list[num_tests / 2];
    std::cout << "Median elapsed time: " << median << " ms\n";

    return 0;
}