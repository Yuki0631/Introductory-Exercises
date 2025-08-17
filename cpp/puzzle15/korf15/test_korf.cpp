#include <random>
#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <cstdlib>
#include <chrono>
#include "../puzzle15.hpp"
#include "korf15.hpp"
#include "../solver15.hpp"
#include "../generator15.hpp"

int main(int argc, char* argv[]) {
    puzzle15::init_manhattan_table(); // マンハッタン距離のテーブルを初期化
    std::mt19937 rng(std::random_device{}()); // 乱数生成器

    // 100個の盤面を生成してA*(IDA*) Searchを実行
    std::size_t generated_total = 0; // すべてのテストの生成ノード数の合計
    long long elapsed_total = 0; // すべてのテストの経過時間の合計
    std::size_t path_length_total = 0; // すべてのテストの経路長の合計
    int successful_tests = 0; // 成功したテストの数


    auto problems = korf15::load_korf_problems("15-puzzle-states.txt");

    auto goal = problems[100];

    int num = 0;
    std::string slv = "ida"; // デフォルトのソルバーはIDA*
    if (argc >= 2) {
        num = std::atoi(argv[1]) - 1; // argv[1]をintに変換（1-based指定 → 0-basedに変換）
    }

    if (argc >= 3) {
        slv = argv[2];
    }

    if (num < 0 || num >= static_cast<int>(problems.size())) {
        std::cerr << "Invalid problem number. Please specify between 1 and " 
                  << problems.size() << ".\n";
        return 1;
    }

    // 解く問題を表示
    std::cout << "Problem " << num + 1 << ":\n";
    for (int i = 0; i < 16; ++i) {
        std::cout << static_cast<int>(problems[num].get(i)) << " ";
        if ((i + 1) % 4 == 0) std::cout << "\n";
    }

    // ゴール状態を表示
    std::cout << "Goal state:\n";
    for (int i = 0; i < 16; ++i) {
        std::cout << static_cast<int>(goal.get(i)) << " ";
        if ((i + 1) % 4 == 0) std::cout << "\n";
    }


    if (slv == "ida") {
        auto result = solver15::IDA_star_path(problems[num], goal);
        if (result.path) {
            generated_total += result.generated;
            elapsed_total += result.elapsed_ms;
            path_length_total += result.path->size();
            successful_tests++;
        }
    }

    if (slv == "a") {
        auto result = solver15::A_star_path(problems[num], goal);
        if (result.path) {
            generated_total += result.generated;
            elapsed_total += result.elapsed_ms;
            path_length_total += result.path->size();
            successful_tests++;
        }
    }

    // 平均値の出力
    std::cout << slv << " Search Results:\n";
    std::cout << "Generated nodes: " << (generated_total / successful_tests) << "\n";
    std::cout << "Elapsed time: " << (elapsed_total / successful_tests) << " ms\n";
    std::cout << "Path length: " << (path_length_total / successful_tests) << "\n";


    // 1秒間に生成されたノード数の計算
    double gen_nodes_per_sec = static_cast<double>(generated_total) / (elapsed_total / 1000.0);
    std::cout << "Generated nodes per second: " << gen_nodes_per_sec << "\n";

    return 0;
}