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
    static inline puzzle15::Puzzle::Move inverse_move(puzzle15::Puzzle::Move m) noexcept {
    using M = puzzle15::Puzzle::Move;
    switch (m) {
        case M::Up:    return M::Down;
        case M::Down:  return M::Up;
        case M::Left:  return M::Right;
        case M::Right: return M::Left;
    }
    return M::Up; // 到達不能
}

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

    if (start.packed == goal.packed) { // もし開始状態が目標状態なら
        auto t1 = std::chrono::steady_clock::now();
        return SearchResult{
            std::make_optional(std::vector<Puzzle::Move>{}),
            generated,
            std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
        };
    }

    using Key = uint64_t;

    BucketPriorityQueue<Node> open(0, 82, 0, 80); // オープンリストのデータ構造 // max値の設定は最重要
    struct Meta { int g; int h; bool closed; };
    std::unordered_map<Key, Meta> meta; // g,h,closed を集約
    struct Parent { Key prev; puzzle15::Puzzle::Move move; };
    std::unordered_map<Key, Parent> parent; // <子状態, (親状態, 打った手)>

    int hstart = h(start);
    open.push(Node{hstart, 0, hstart, start}, hstart, hstart);
    meta[start.packed] = {0, hstart, false};

    constexpr Puzzle::Move MOVES[4] = {
        Puzzle::Move::Up, Puzzle::Move::Down, Puzzle::Move::Left, Puzzle::Move::Right
    };

    while (!open.empty()) {
        Node cur = open.top(); // オープンリストから最小のノードを取得
        open.pop(); // オープンリストからノードを削除

        auto itg = meta.find(cur.s.packed);
        if (itg != meta.end() && cur.g > itg->second.g) continue;

        // ゴール条件を満たした場合
        if (cur.s.packed == goal.packed) {
            std::vector<Puzzle::Move> path;
            Puzzle x = cur.s;
            while (!(x.packed == start.packed)) {
                auto itp = parent.find(x.packed);
                if (itp == parent.end()) break; // ありえないが念のため
                path.push_back(itp->second.move);

                puzzle15::Puzzle prev;
                prev.packed = itp->second.prev;

                for (int i = 0; i < 16; ++i) {
                    if (((prev.packed >> (i * 4)) & 0xF) == 0) {
                        prev.zero_pos = static_cast<uint8_t>(i);
                        break;
                    }
                }
                x = prev;
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
        {
            Meta &m = meta[cur.s.packed];
            if (m.closed) continue; // すでにクローズドリストにあるならスキップ
            m.closed = true; // クローズドリストに追加
        }

        

        // ノードの拡張 (Expand)
        for (auto m : MOVES) {
            int old_zero_pos = cur.s.zero_pos;
            if (!Puzzle::can_move(old_zero_pos, m)) continue; // 移動できない場合はスキップ
            auto nxt_opt = cur.s.moved(m);
            if (!nxt_opt) continue; // 移動後の状態が無効の場合はスキップ
            int new_zero_pos = nxt_opt->zero_pos;
            int t = nxt_opt->get(old_zero_pos);
            Puzzle nxt = *nxt_opt;

            int tentative_g = cur.g + 1; // 暫定的な g 値

            // 即時重複検出
            auto it = meta.find(nxt.packed);
            if (it != meta.end() && tentative_g >= it->second.g) continue;

            int h_value = puzzle15::manhattan_delta_for_move(meta[cur.s.packed].h, t, new_zero_pos, old_zero_pos);

            parent[nxt.packed] = {cur.s.packed, m}; // 親と手を更新
            meta[nxt.packed] = {tentative_g, h_value, false}; // メタ情報を更新
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

inline SearchResult
IDA_star_path(const puzzle15::Puzzle& start,
              const puzzle15::Puzzle& goal) {
    using puzzle15::Puzzle;

    SearchResult out;
    auto t0 = std::chrono::steady_clock::now();

    // 与えられたスタート状態がゴール状態なら
    if (start.packed == goal.packed) {
        out.path = std::vector<Puzzle::Move>{}; // 空経路
        out.generated = 1;
        auto t1 = std::chrono::steady_clock::now();
        out.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        return out;
    }

    // IDA* 用ワーキング領域
    std::array<Puzzle::Move, 81> path; // 探索経路
    std::array<uint64_t, 81> onpath; // ループ防止用、最大の深さは 80 なので 81 で十分
    int depth = 0;

    const int h0 = puzzle15::manhattan_heuristic_fast(start);
    int bound = h0; // 初期の閾値

    std::cout << "Initial bound: " << bound << "\n";

    // 再帰DFS: 見つかったら負値（-1）を返す。見つからなければ次の閾値候補（最小の f 超過値）
    struct Dfs {
        const Puzzle& goal;
        SearchResult& out;
        std::array<uint64_t, 81>& onpath;
        std::array<Puzzle::Move, 81>& path;
        int& depth;

        int operator()(const Puzzle& s, int g, int bound, int h, std::optional<Puzzle::Move> prev_move) {
            const int f = g + h;
            if (f > bound) return f;                    // 閾値超過 → 次のbound候補
            if (s.packed == goal.packed) return -1;     // 発見

            // 展開（子の f 超過の最小値を覚える）
            int min_next = std::numeric_limits<int>::max();

            // 直前手の逆手はスキップして枝刈り
            std::array<std::pair<Puzzle, Puzzle::Move>, 4> buf;
            int n = s.neighbors_into(buf);
            out.generated += n; // 生成ノード数カウント（「子を列挙した数」で近似）

            for (int i = 0; i < n; ++i) {
                const auto& t = buf[i].first;
                const auto mv = buf[i].second;

                if (prev_move.has_value() && mv == inverse_move(*prev_move)) {
                    continue; // 即時バックトラック防止
                }
                // 経路上再訪の禁止（IDA*なのでクローズ表は持たない）
                bool seen = false;
                for (int k = 0; k < depth; ++k) {
                    if (onpath[k] == t.packed) {
                        seen = true;
                        break;
                    }
                }
                if (seen) continue;

            // マンハッタンヒューリスティックの差分更新
            const int old_zero = s.zero_pos;
            const int new_zero = t.zero_pos;
            const uint8_t moved_tile = t.get(old_zero);

            const int h_child = puzzle15::manhattan_delta_for_move(h, moved_tile, new_zero, old_zero);
            const int f_child = (g + 1) + h_child;

            if (f_child > bound) {
                if (f_child < min_next) min_next = f_child; // 最小の f 超過値を更新
                continue; // 次の子ノードへ
            }

            path[depth] = mv; // 現在の手を記録
            onpath[depth] = t.packed; // 現在の状態を記録
            ++depth; // 深さを増やす

            int r = (*this)(t, g + 1, bound, h_child, mv);

            if (r == -1) { // found
                return -1;
            }
            if (r < min_next) min_next = r;

            --depth; // 深さを戻す
        }
        return min_next;
    }
};



    // 実際のIDA*探索
    for (;;) {
        depth = 0;
        onpath[0] = start.packed;
        Dfs dfs{goal, out, onpath, path, depth};

        int r = dfs(start, 0, bound, h0, std::nullopt);
        if (r == -1) {
            out.path = std::vector<Puzzle::Move>(path.begin(), path.begin() + depth);
            auto t1 = std::chrono::steady_clock::now();
            out.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
            return out;
        }
        if (r == std::numeric_limits<int>::max()) { // すべての子が閾値超過なら終了
            out.path = std::nullopt;
            auto t1 = std::chrono::steady_clock::now();
            out.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
            return out;
        }
        bound = r;
    }
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