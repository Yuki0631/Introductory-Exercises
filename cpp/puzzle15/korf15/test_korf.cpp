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

    auto goal = puzzle15::Puzzle::goal(); // 目標状態

    auto problems = korf15::load_korf_problems("15-puzzle-states-debug.txt");

    int num = 2;

    // 解く問題を表示
    std::cout << "Problem " << num << ":\n";
    for (int i = 0; i < 16; ++i) {
        std::cout << static_cast<int>(problems[num].get(i)) << " ";
        if ((i + 1) % 4 == 0) std::cout << "\n";
    }

    
    auto result = solver15::IDA_star_path(problems[num], goal);
    if (result.path) {
        generated_total += result.generated;
        elapsed_total += result.elapsed_ms;
        path_length_total += result.path->size();
        successful_tests++;
    }
        
        

    // 平均値の出力
    std::cout << "IDA* Search Results:\n";
    std::cout << "Generated nodes: " << (generated_total / successful_tests) << "\n";
    std::cout << "Elapsed time: " << (elapsed_total / successful_tests) << " ms\n";
    std::cout << "Path length: " << (path_length_total / successful_tests) << "\n";


    // 1秒間に生成されたノード数の計算
    double gen_nodes_per_sec = static_cast<double>(generated_total) / (elapsed_total / 1000.0);
    std::cout << "Generated nodes per second: " << gen_nodes_per_sec << "\n";

    return 0;
}