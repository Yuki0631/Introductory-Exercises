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

namespace puzzle15 {

struct Puzzle {
    std::array<uint8_t, 16> tiles{}; // タイルの配置
    uint8_t zero_pos = 0; // 空白の位置のインデックス

    static Puzzle goal() {
        Puzzle p;
        p.tiles = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0};
        p.zero_pos = 15;
        return p;
    }

    // コンストラクタ 
    explicit Puzzle(const std::array<uint8_t,16>& t) : tiles(t) {
        for (uint8_t i = 0; i < 16; ++i) if (tiles[i] == 0) { zero_pos = i; break; }
    }
    Puzzle() = default; // デフォルトコンストラクタ

    bool operator==(const Puzzle& other) const noexcept { return tiles == other.tiles; } // 等価比較の演算オペレータ
    bool operator!=(const Puzzle& other) const noexcept { return !(*this == other); } // 非等価比較の演算オペレータ

    // デバッグ用の、盤面を表示する関数
    std::string to_string() const {
        std::ostringstream oss;
        for (int i = 0; i < 16; ++i) {
            if (i && i % 4 == 0) oss << '\n'; // iが0以外の4の倍数の時
            if (tiles[i] == 0) oss << " _";
            else {
                oss << ' ' << int(tiles[i]);
            }
        }
        return oss.str();
    }

    // 1次元配列と2次元配列のインデックス交換を行うインライン関数群
    static inline int row(int idx) { return idx / 4; } 
    static inline int col(int idx) { return idx % 4; }
    static inline int idx(int r, int c) { return r*4 + c; }

    // 空白を上下左右へ動かす際に必要な列挙型と関数
    enum class Move : uint8_t { Up=0, Down=1, Left=2, Right=3 };

    // 空白を動かせるか判定する関数
    static inline bool can_move(int zero, Move m) { // zero: 空白の位置
        int r = row(zero), c = col(zero);
        switch (m) {
            case Move::Up:    return r > 0;
            case Move::Down:  return r < 3;
            case Move::Left:  return c > 0;
            case Move::Right: return c < 3;
        }
        return false;
    }

    // 空白を動かして新しい盤面を返す関数
    std::optional<Puzzle> moved(Move m) const {
        if (!can_move(zero_pos, m)) return std::nullopt;
        int zr = row(zero_pos), zc = col(zero_pos);
        int nr = zr, nc = zc;
        switch (m) {
            case Move::Up:    --nr; break;
            case Move::Down:  ++nr; break;
            case Move::Left:  --nc; break;
            case Move::Right: ++nc; break;
        }
        int ni = idx(nr,nc); // 入れ替え先
        Puzzle nxt = *this;
        std::swap(nxt.tiles[zero_pos], nxt.tiles[ni]);
        nxt.zero_pos = static_cast<uint8_t>(ni);
        return nxt;
    }

    // 合法手をすべて列挙する関数
    std::vector<std::pair<Puzzle, Move>> neighbors() const {
        std::vector<std::pair<Puzzle, Move>> res;
        res.reserve(4);
        for (Move m : {Move::Up, Move::Down, Move::Left, Move::Right}) {
            if (auto next = moved(m)) res.emplace_back(*next, m);
        }
        return res;
    }

    // スライド操作の逆操作を返す関数
    inline Puzzle::Move inverse(Puzzle::Move m) noexcept {
    switch (m) {
        case Puzzle::Move::Up:    return Puzzle::Move::Down;
        case Puzzle::Move::Down:  return Puzzle::Move::Up;
        case Puzzle::Move::Left:  return Puzzle::Move::Right;
        case Puzzle::Move::Right: return Puzzle::Move::Left;
    }
    // 理論的には呼ばれないが、デフォルトの戻り値を設定しておく
    return Puzzle::Move::Up;
    }
    
};

// ハッシュ（unordered_* で使う用）
struct PuzzleHash {
    std::size_t operator()(const Puzzle& p) const noexcept {
        // シンプルかつ高速なローリングハッシュ
        std::size_t h = 1469598103934665603ull; // FNV1-a の初期値
        for (auto v : p.tiles) {
            h ^= static_cast<std::size_t>(v + 1); // xor演算
            h *= 1099511628211ull; // FNVの素数を掛け合わせる
        }
        return h;
    }
};

} // namespace puzzle15