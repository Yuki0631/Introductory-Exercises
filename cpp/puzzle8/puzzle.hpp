#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <sstream>
#include <iomanip>
#include <utility>
#include <functional>
#include <cstdlib>

namespace puzzle8 {

// 3x3 の座標と一次元indexの相互変換関数
static inline int row(int idx) {
    return idx / 3;
}
static inline int col(int idx) {
    return idx % 3;
}
static inline int idx(int r, int c) {
    return r * 3 + c;
}

// 4bitづつインデックスの値を書き込む (先頭4bitは0番目のタイルの値などのようにする)
static inline uint8_t get_nibble(uint64_t x, int i) {
    return static_cast<uint8_t>((x >> (i * 4)) & 0xF);
}
static inline void set_nibble(uint64_t &x, int i, uint8_t v) {
    const uint64_t mask = ~(0xFULL << (i * 4));
    x = (x & mask) | (static_cast<uint64_t>(v & 0xF) << (i * 4));
}

struct Puzzle {
    uint64_t board = 0; // 4bit×9=36bit
    uint8_t  zero_pos = 0; // 空白の位置
    uint8_t  hman = 0; // マンハッタン距離

    enum class Move : uint8_t { Up=0, Down=1, Left=2, Right=3 };

    // 目標状態
    static Puzzle goal() {
        Puzzle p;
        for (int i = 0; i < 8; ++i) {
            set_nibble(p.board, i, static_cast<uint8_t>(i+1));
        }
        set_nibble(p.board, 8, 0);
        p.zero_pos = 8;
        p.recompute_manhattan();
        return p;
    }

    // コンストラクタ 
    explicit Puzzle(const std::array<uint8_t,9>& t) {
        for (int i = 0; i < 9; ++i) {
            set_nibble(board, i, t[i]);
            if (t[i] == 0) {
                zero_pos = static_cast<uint8_t>(i);
            }
        }
        recompute_manhattan();
    }
    Puzzle() = default;

    bool operator==(const Puzzle& other) const noexcept {
        return board == other.board;
    }
    bool operator!=(const Puzzle& other) const noexcept {
        return !(*this == other);
    }
    bool operator<(const Puzzle& other) const noexcept  {
        return board < other.board;
    }

    // デバッグ表示
    std::string to_string() const {
        std::ostringstream oss;
        for (int i = 0; i < 9; ++i) {
            if (i && i % 3 == 0) oss << '\n';
            uint8_t v = get_nibble(board, i);
            if (v == 0) oss << " _";
            else oss << ' ' << int(v);
        }
        return oss.str();
    }

    // 初期化、検証用にもう一度マンハッタン距離を計算する関数
    inline void recompute_manhattan() {
        uint8_t sum = 0;
        for (int i = 0; i < 9; ++i) {
            uint8_t t = get_nibble(board, i);
            if (t == 0) {
                zero_pos = static_cast<uint8_t>(i);
                continue;
            }
            int tr = (t - 1) / 3, tc = (t - 1) % 3;
            sum += static_cast<uint8_t>(std::abs(tr - row(i)) + std::abs(tc - col(i)));
        }
        hman = sum;
    }

    // 空白を動かせるか判定する関数
    static inline bool can_move(int zero, Move m) {
        int r = row(zero), c = col(zero);
        switch (m) {
            case Move::Up: return r > 0;
            case Move::Down: return r < 2;
            case Move::Left: return c > 0;
            case Move::Right: return c < 2;
        }
        return false;
    }

    // インプレース移動と差分マンハッタンを計算する関数
    inline void move_inplace(Move m) {
        int zr = row(zero_pos), zc = col(zero_pos);
        int nr = zr, nc = zc;
        switch (m) {
            case Move::Up: --nr; break;
            case Move::Down: ++nr; break;
            case Move::Left: --nc; break;
            case Move::Right: ++nc; break;
        }
        int to = idx(nr, nc);
        uint8_t tile = get_nibble(board, to);
        // 差分更新
        if (tile != 0) {
            int old_r = row(to), old_c = col(to);
            int new_r = row(zero_pos), new_c = col(zero_pos);
            int tr = (tile - 1) / 3, tc = (tile - 1) % 3;
            uint8_t oldd = static_cast<uint8_t>(std::abs(tr - old_r) + std::abs(tc - old_c));
            uint8_t newd = static_cast<uint8_t>(std::abs(tr - new_r) + std::abs(tc - new_c));
            hman = static_cast<uint8_t>(hman - oldd + newd);
        }
        // tile と 0 をスワップする
        set_nibble(board, to, 0);
        set_nibble(board, zero_pos, tile);
        zero_pos = static_cast<uint8_t>(to);
    }

    // 新しい盤面を返す関数
    std::optional<Puzzle> moved(Move m) const {
        if (!can_move(zero_pos, m)) {
            return std::nullopt;
        }
        Puzzle nxt = *this;
        nxt.move_inplace(m);
        return nxt;
    }

    // 合法手をすべて列挙する関数
    std::vector<std::pair<Puzzle, Move>> neighbors() const {
        std::vector<std::pair<Puzzle, Move>> res;
        res.reserve(4);
        for (Move m : {Move::Up, Move::Down, Move::Left, Move::Right}) {
            if (auto next = moved(m)) {
                res.emplace_back(*next, m);
            }
        }
        return res;
    }

    // 逆操作
    static inline Move inverse(Move m) noexcept {
        switch (m) {
            case Move::Up: return Move::Down;
            case Move::Down: return Move::Up;
            case Move::Left: return Move::Right;
            case Move::Right: return Move::Left;
        }
        return Move::Up;
    }

    // 0..8 がちょうど一回ずつ現れるか確認する関数
    inline bool has_valid_tiles() const {
        std::array<int, 9> cnt{};
        cnt.fill(0);
        for (int i = 0; i < 9; ++i) {
            uint8_t v = get_nibble(board, i);
            if (v > 8) {
                return false;
            }
            cnt[v] += 1;
        }
        for (int v : cnt) {
            if (v != 1) {
                return false;
            }
        }
        return true;
    }

    // zero_pos と hman が実データと一致しているか判定する関数
    inline bool validate_invariants(bool check_h=true) const {
        // zero_pos
        int z = -1;
        for (int i = 0; i < 9; ++i) {
            if (get_nibble(board, i) == 0) {
                z = i;
                break;
            }
        }
        if (z < 0 || static_cast<int>(zero_pos) != z) {
            return false;
        }
        if (!check_h) {
            return true;
        }
        // hman
        uint8_t sum = 0;
        for (int i = 0; i < 9; ++i) {
            uint8_t t = get_nibble(board, i);
            if (t == 0) {
                continue;
            }
            int r = row(i), c = col(i);
            int tr = (t - 1) / 3, tc = (t - 1) % 3;
            sum += static_cast<uint8_t>(std::abs(tr - r) + std::abs(tc - c));
        }
        return sum == hman;
    }
};

// unordered_* で使う用のハッシュ
struct PuzzleHash {
    std::size_t operator()(const Puzzle& p) const noexcept {
        return std::hash<uint64_t>{}(p.board);
    }
};

} // namespace puzzle8
