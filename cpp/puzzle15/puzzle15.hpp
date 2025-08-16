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
    uint64_t packed = 0;
    uint8_t zero_pos = 0; // 空白の位置

    enum class Move : uint8_t { Up, Down, Left, Right };

    static inline uint8_t nibble(uint64_t x, int idx) noexcept {
        return static_cast<uint8_t>((x >> (idx * 4)) & 0xFULL); // 4ビットを抽出
    }

    static inline void set_nibble(uint64_t &x, int idx, uint8_t v) noexcept {
        const uint64_t mask = 0xFULL << (idx * 4);
        x = (x & ~mask) | (static_cast<uint64_t>(v & 0xFULL) << (idx * 4));
    }
    inline uint8_t get(int idx) const noexcept { return nibble(packed, idx); }
    inline void    set(int idx, uint8_t v) noexcept { set_nibble(packed, idx, v); }

    // goalの生成
    static inline Puzzle goal() noexcept {
        Puzzle g;
        for (int i = 0; i < 15; ++i) set_nibble(g.packed, i, static_cast<uint8_t>(i+1));
        set_nibble(g.packed, 15, 0);
        g.zero_pos = 15;
        return g;
    }

    // 文字列化は get(i) を使って実装
    std::string to_string() const { return ""; } // 不要なメンバの実装は省略

    // 位置 → (row,col)
    static inline int row(int idx) noexcept { return idx / 4; }
    static inline int col(int idx) noexcept { return idx % 4; }

    static inline bool can_move(int zero, Move m) noexcept {
        const int r = row(zero), c = col(zero);
        switch (m) {
            case Move::Up:    return r > 0;
            case Move::Down:  return r < 3;
            case Move::Left:  return c > 0;
            case Move::Right: return c < 3;
        }
        return false;
    }

    // 空白を動かした盤面を返す（ニブル swap）
    std::optional<Puzzle> moved(Move m) const noexcept {
        if (!can_move(zero_pos, m)) return std::nullopt;
        Puzzle q = *this;
        int zr = row(zero_pos), zc = col(zero_pos);
        int tr = zr + (m == Move::Down) - (m == Move::Up);
        int tc = zc + (m == Move::Right) - (m == Move::Left);
        int to = tr * 4 + tc;              // 入れ替える相手位置
        uint8_t t = nibble(q.packed, to);  // タイル値

        // swap(t, 0)
        set_nibble(q.packed, to, 0);
        set_nibble(q.packed, zero_pos, t);
        q.zero_pos = static_cast<uint8_t>(to);
        return q;
    }

    inline std::vector<std::pair<Puzzle, Puzzle::Move>> neighbors() const {
    using M = Puzzle::Move;
    std::vector<std::pair<Puzzle, M>> out;
    out.reserve(4);

    const int zr = zero_pos / 4;
    const int zc = zero_pos % 4;

    auto gen = [&](M m, int dr, int dc) {
        const int tr = zr + dr, tc = zc + dc;
        const int to = tr * 4 + tc;

        Puzzle q = *this;

        // 空白(0) と相手タイルをニブル交換
        const uint8_t t = this->get(to); // 元盤面から取得
        q.set(to, 0);
        q.set(this->zero_pos, t);
        q.zero_pos = static_cast<uint8_t>(to);

        out.emplace_back(std::move(q), m);
    };

    if (zr > 0) gen(M::Up,    -1,  0);
    if (zr < 3) gen(M::Down,   1,  0);
    if (zc > 0) gen(M::Left,   0, -1);
    if (zc < 3) gen(M::Right,  0,  1);

    return out;
    }

    // 固定長buffer版の neighbors 関数
    inline int neighbors_into(std::array<std::pair<Puzzle, Puzzle::Move>, 4>& buf) const noexcept {
        using M = Puzzle::Move;
        int n = 0;

        const int zr = zero_pos / 4;
        const int zc = zero_pos % 4;

        auto push = [&](M m, int dr, int dc) { // 外部変数をキャプチャする (コピーを避ける)
            const int tr = zr + dr, tc = zc + dc;
            const int to = tr * 4 + tc;

            Puzzle q = *this;

            const uint8_t t = this->get(to); // 元盤面から取得
            set_nibble(q.packed, to, 0);
            set_nibble(q.packed, zero_pos, t);
            q.zero_pos = static_cast<uint8_t>(to);

            buf[n++] = {std::move(q), m};
        };

        if (zr > 0) push(M::Up,    -1,  0);
        if (zr < 3) push(M::Down,   1,  0);
        if (zc > 0) push(M::Left,   0, -1);
        if (zc < 3) push(M::Right,  0,  1);

        return n;
    }

    // 方向の反転
    inline Puzzle::Move inverse(Puzzle::Move m) noexcept {
    using M = Puzzle::Move;
    switch (m) {
        case M::Up:    return M::Down;
        case M::Down:  return M::Up;
        case M::Left:  return M::Right;
        case M::Right: return M::Left;
    }
    // 到達不能だが、警告抑止用に返す
    return M::Up;
    }

    inline bool apply_move_inplace(Move m, uint8_t& moved_tile, uint8_t& old_zero) noexcept {
        const int zr = row(zero_pos), zc = col(zero_pos);
        int dr = 0, dc = 0;
        switch (m) {
            case Move::Up:    dr = -1; dc = 0; break;
            case Move::Down:  dr =  1; dc = 0; break;
            case Move::Left:  dr = 0; dc = -1; break;
            case Move::Right: dr = 0; dc =  1; break;
        }
        const int tr = zr + dr, tc = zc + dc;
        if (tr < 0 || tr > 3 || tc < 0 || tc > 3) return false; // 移動できない場合
        const int to = tr * 4 + tc; // 移動先のインデックス

        old_zero = zero_pos; // 元のゼロ位置を保存
        moved_tile = nibble(packed, to); // 移動するタイルの値を保存

        set_nibble(packed, to, 0);
        set_nibble(packed, old_zero, moved_tile);
        zero_pos = static_cast<uint8_t>(to);
        return true;
    }

    inline void undo_move_inplace(uint8_t moved_tile, uint8_t old_zero) noexcept {
        const int to = zero_pos; // 現在のゼロ位置
        set_nibble(packed, old_zero, 0); // 元のゼロ位置にタイルをセット
        set_nibble(packed, to, moved_tile); // 現在のゼロ位置にタイルを戻す
        zero_pos = old_zero; // ゼロ位置を更新
    }

};

} // namespace puzzle15