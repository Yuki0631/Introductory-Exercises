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

    int num_problems = 1;

    // 問題の生成
    std::vector<puzzle15::Puzzle> puzzle_list = std::vector<puzzle15::Puzzle>(); // 100個の問題
    puzzle_list.reserve(num_problems);

    for (int i = 0; i < num_problems; ++i) {
        int steps = std::uniform_int_distribution<int>(70, 71)(rng); 
        puzzle15::Puzzle p = puzzle15::generate_random_puzzle(steps, std::nullopt);
        puzzle_list.push_back(p);
    }

    auto goal = puzzle15::Puzzle::goal(); // 目標状態

    // 100個の盤面を生成してA* Searchを実行

    std::size_t generated_total = 0;
    long long elapsed_total = 0;
    std::size_t path_length_total = 0;
    int success_count = 0;

    std::vector<long long> elapsed_list;

    for (int i = 0; i < puzzle_list.size(); ++i) {
        auto result = solver15::IDA_star_path(puzzle_list[i], goal);
        if (result.path) {
            generated_total += result.generated;
            elapsed_total += result.elapsed_ms;
            path_length_total += result.path->size();
            elapsed_list.push_back(result.elapsed_ms);
            success_count++;
        }
    }

    // 同様に IDA* を実行
    int generated_total_ida = 0;
    int elapsed_total_ida = 0;
    std::size_t path_length_total_ida = 0;
    int success_count_ida = 0;

    std::vector<long long> elapsed_list_ida;

    for (int i = 0; i < puzzle_list.size(); ++i) {
        auto result = solver15::IDA_star_path(puzzle_list[i], goal);
        if (result.path) {
            generated_total_ida += result.generated;
            elapsed_total_ida += result.elapsed_ms;
            path_length_total_ida += result.path->size();
            elapsed_list_ida.push_back(result.elapsed_ms);
            success_count_ida++;
        }
    }

    std::cout << "A* Search Results ( " << num_problems << " problems):\n";
    std::cout << "Average generated nodes: " << (generated_total / success_count) << "\n";
    std::cout << "Average elapsed time: " << (elapsed_total / success_count) << " ms\n";
    std::cout << "Average path length: " << (path_length_total / success_count) << "\n";

    std::sort(elapsed_list.begin(), elapsed_list.end());
    long long median = elapsed_list[success_count / 2];
    std::cout << "Median elapsed time: " << median << " ms\n";

    double gen_nodes_per_sec = static_cast<double>(generated_total) / (elapsed_total / 1000.0);
    std::cout << "Generated nodes per second: " << gen_nodes_per_sec << "\n";

    std::cout << "\n";

    // IDA* の結果を表示
    std::cout << "IDA* Search Results ( " << num_problems << " problems):\n";
    std::cout << "Average generated nodes: " << (generated_total_ida / success_count_ida) << "\n";
    std::cout << "Average elapsed time: " << (elapsed_total_ida / success_count_ida) << " ms\n";
    std::cout << "Average path length: " << (path_length_total_ida / success_count_ida) << "\n";

    std::sort(elapsed_list_ida.begin(), elapsed_list_ida.end());
    long long median_ida = elapsed_list_ida[success_count_ida / 2];
    std::cout << "Median elapsed time: " << median_ida << " ms\n";

    double gen_nodes_per_sec_ida = static_cast<double>(generated_total_ida) / (elapsed_total_ida / 1000.0);
    std::cout << "Generated nodes per second: " << gen_nodes_per_sec_ida << "\n";

    return 0;
}