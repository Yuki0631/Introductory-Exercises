#pragma once
#include <queue>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <optional>
#include <algorithm>
#include <chrono>
#include <sstream>
#include "puzzle15.hpp"
#include "heuristic15.hpp"
#include "bucket_pq.hpp"

namespace solver15 {

struct SearchResult { // 探索結果用の構造体
    std::optional<std::vector<puzzle15::Puzzle::Move>> path;
    std::size_t generated = 0;
    long long elapsed_ms = 0;
};

using Heuristic = std::function<int(const puzzle15::Puzzle&)>; // ヒューリスティック関数の型

// A* Search 
inline SearchResult
A_star_path(const puzzle15::Puzzle& start,
            const puzzle15::Puzzle& goal
            ) {
    using puzzle15::Puzzle;
    Heuristic h = puzzle15::manhattan_heuristic_fast;

    auto t0 = std::chrono::steady_clock::now();
    std::size_t generated = 0;

    struct Node {
        int f; // プライオリティ値
        int g; // 実コスト（手数）
        int h; // ヒューリスティック値
        Puzzle s; // 現在の状態
    };

    struct Cmp { // 比較関数
        bool operator()(const Node& a, const Node& b) const noexcept {
            if (a.f != b.f) return a.f > b.f; // fが異なるならfが小さい方が優先
            return a.h > b.h; // 同点ならhが小さい方を優先
        }
    };

    if (start.tiles == goal.tiles) { // もし開始状態が目標状態なら
        auto t1 = std::chrono::steady_clock::now();
        return SearchResult{
            std::make_optional(std::vector<Puzzle::Move>{}),
            generated,
            std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
        };
    }

    BucketPriorityQueue<Node> open(0, 82, 0, 80); // オープンリストのデータ構造 // max値の設定は最重要
    std::unordered_set<Puzzle, puzzle15::PuzzleHash> closed; // クローズドリストのデータ構造
    std::unordered_map<Puzzle, int, puzzle15::PuzzleHash> gscore; // g値のマップ
    std::unordered_map<Puzzle, int, puzzle15::PuzzleHash> hscore; // h値のマップ
    std::unordered_map<Puzzle, std::pair<Puzzle, Puzzle::Move>, puzzle15::PuzzleHash> parent; // <子状態, (親状態, 打った手)>

    int hstart = h(start);
    open.push(Node{hstart, 0, hstart, start}, hstart, hstart);
    gscore[start] = 0;
    hscore[start] = hstart;

    constexpr Puzzle::Move MOVES[4] = {
        Puzzle::Move::Up, Puzzle::Move::Down, Puzzle::Move::Left, Puzzle::Move::Right
    };

    while (!open.empty()) {
        Node cur = open.top(); // オープンリストから最小のノードを取得
        open.pop(); // オープンリストからノードを削除

        auto itg = gscore.find(cur.s);
        if (itg != gscore.end() && cur.g > itg->second) continue;

        // ゴール条件を満たした場合
        if (cur.s.tiles == goal.tiles) {
            std::vector<Puzzle::Move> path;
            Puzzle x = cur.s;
            while (!(x.tiles == start.tiles)) {
                auto itp = parent.find(x);
                if (itp == parent.end()) break; // ありえないが念のため
                path.push_back(itp->second.second);
                x = itp->second.first;
            }
            std::reverse(path.begin(), path.end()); // スタートからゴールへの経路にする
            auto t1 = std::chrono::steady_clock::now();
            return SearchResult{
                std::make_optional(std::move(path)),
                generated,
                std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
            };
        }

        // クローズドリストへの追加
        // 遅延重複検出
        if (!closed.insert(cur.s).second) continue; // falseならすでに訪問済みなのでスキップする

        

        // ノードの拡張 (Expand)
        for (auto m : MOVES) {
            int old_zero_pos = cur.s.zero_pos;
            if (!Puzzle::can_move(old_zero_pos, m)) continue; // 移動できない場合はスキップ
            auto nxt_opt = cur.s.moved(m);
            if (!nxt_opt) continue; // 移動後の状態が無効の場合はスキップ
            int new_zero_pos = nxt_opt->zero_pos;
            int t = nxt_opt->tiles[old_zero_pos];
            Puzzle nxt = *nxt_opt;

            int tentative_g = cur.g + 1; // 暫定的な g 値

            // 即時重複検出
            auto it = gscore.find(nxt); // gscore に存在するか確認
            if (it != gscore.end() && tentative_g >= it->second) continue; // 既存の経路よりも悪い場合はスキップ

            int h_value = puzzle15::manhattan_delta_for_move(hscore[cur.s], t, new_zero_pos, old_zero_pos);

            parent[nxt] = std::make_pair(cur.s, m); // 親と手を更新
            gscore[nxt] = tentative_g; // g 値を更新
            hscore[nxt] = h_value; // h 値を更新
            int f_value = tentative_g + h_value;
            generated++; // 新たに生成したノード数をカウント
            open.push(Node{f_value, tentative_g, h_value, nxt}, f_value, h_value);
        }
    }

    return SearchResult{
        std::nullopt,
        generated,
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count()
    };
}

inline std::string move_to_string(puzzle15::Puzzle::Move m) {
    switch (m) {
        case puzzle15::Puzzle::Move::Up:    return "Up";
        case puzzle15::Puzzle::Move::Down:  return "Down";
        case puzzle15::Puzzle::Move::Left:  return "Left";
        case puzzle15::Puzzle::Move::Right: return "Right";
    }
    return "Unknown"; // 理論的には呼ばれないが、デフォルトの戻り値を設定しておく
}

inline std::string path_to_string(const std::vector<puzzle15::Puzzle::Move>& path) {
    std::ostringstream oss;
    for (const auto& move : path) {
        oss << move_to_string(move) << " ";
    }
    return oss.str();
}

} // namespace solver15