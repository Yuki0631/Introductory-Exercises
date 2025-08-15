#include <random>
#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include "../puzzle15.hpp"
#include "korf15.hpp"
#include "../solver15.hpp"

int main() {
    puzzle15::init_manhattan_table(); // マンハッタン距離のテーブルを初期化

    // 100個の盤面を生成してA* Searchを実行
    std::size_t generated_total = 0; // すべてのテストの生成ノード数の合計
    long long elapsed_total = 0; // すべてのテストの経過時間の合計
    std::size_t path_length_total = 0; // すべてのテストの経路長の合計
    int successful_tests = 0; // 成功したテストの数
    std::vector<long long> elapsed_list; // 成功したテストの経過時間リスト

    auto goal = puzzle15::Puzzle::goal(); // 目標状態

    auto problems = korf15::load_korf_problems("15-puzzle-states.txt");

    // デバッグ用に最初の1個の盤面を表示
    std::cout << "Initial problem (1):\n";
    for (int i = 0; i < 16; ++i) {
        std::cout << static_cast<int>(problems[0].get(i)) << " ";
        if ((i + 1) % 4 == 0) std::cout << "\n";
    }

    for (int i = 0; i < problems.size(); ++i) {
        auto result = solver15::A_star_path(problems[i], goal);
        if (result.path) {
            generated_total += result.generated;
            elapsed_total += result.elapsed_ms;
            path_length_total += result.path->size();
            successful_tests++;
            elapsed_list.push_back(result.elapsed_ms);
            std::cout << "Problem " << (i + 1) << ": "
                      << "Path length = " << result.path->size() << ", "
                      << "Elapsed time = " << result.elapsed_ms << " ms, "
                      << "Generated nodes = " << result.generated << "\n";
        }
        if ((i+1) % 20 == 0 && (i+1) < problems.size()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 200ミリ秒待機
        }
    }

    // 平均値の出力
    std::cout << "A* Search Results (100 problems):\n";
    std::cout << "Average generated nodes: " << (generated_total / successful_tests) << "\n";
    std::cout << "Average elapsed time: " << (elapsed_total / successful_tests) << " ms\n";
    std::cout << "Average path length: " << (path_length_total / successful_tests) << "\n";

    // 中央値の出力
    std::sort(elapsed_list.begin(), elapsed_list.end());
    long long median = elapsed_list[successful_tests / 2];
    std::cout << "Median elapsed time: " << median << " ms\n";

    // 1秒間に生成されたノード数の計算
    double gen_nodes_per_sec = static_cast<double>(generated_total) / (elapsed_total / 1000.0);
    std::cout << "Generated nodes per second: " << gen_nodes_per_sec << "\n";

    return 0;

}