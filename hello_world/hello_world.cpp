#include <array>
#include <iostream>
#include <utility>

template <std::size_t M, std::size_t N>
using Tetromino = std::array<std::array<int, N>, M>;

// 辅助模板，用于在编译期获取数组的旋转
template <std::size_t M, std::size_t N, std::size_t... I>
constexpr Tetromino<N, M> RightRotateImpl(const Tetromino<M, N>& arr, std::index_sequence<I...>) {
    // 通过嵌套的初始化列表构造旋转后的矩阵
    return {{arr[M - 1 - I % M][I / M]...}};
}

/// @brief 右旋(顺时针旋转) 90 度
template <std::size_t M, std::size_t N>
constexpr Tetromino<N, M> RightRotate(const Tetromino<M, N>& arr) {
    // 利用递归模板展开计算旋转矩阵
    return RightRotateImpl<M, N>(arr, std::make_index_sequence<M * N>{});
}

// 辅助模板，用于在编译期获取数组的旋转
template <std::size_t M, std::size_t N, std::size_t... I>
constexpr Tetromino<N, M> LeftRotateImpl(const Tetromino<M, N>& arr, std::index_sequence<I...>) {
    // 通过嵌套的初始化列表构造旋转后的矩阵
    return {{arr[I % M][N - 1 - I / M]...}};
}

/// @brief 作旋(逆时针旋转) 90 度
template <std::size_t M, std::size_t N>
constexpr Tetromino<N, M> LeftRotate(const Tetromino<M, N>& arr) {
    // 利用递归模板展开计算旋转矩阵
    return LeftRotateImpl<M, N>(arr, std::make_index_sequence<M * N>{});
}

int main() {
    constexpr Tetromino<2, 3> original{{
        {1, 2, 3},
        {4, 5, 6},
    }};

    {
        constexpr auto rotated = RightRotate(original);

        // rotated 是一个 3x2 的数组
        static_assert(rotated[0][0] == 4);
        static_assert(rotated[0][1] == 1);
        static_assert(rotated[1][0] == 5);
        static_assert(rotated[1][1] == 2);
        static_assert(rotated[2][0] == 6);
        static_assert(rotated[2][1] == 3);
    }

    {
        constexpr auto rotated = LeftRotate(original);

        // rotated 是一个 3x2 的数组
        static_assert(rotated[0][0] == 3);
        static_assert(rotated[0][1] == 6);
        static_assert(rotated[1][0] == 2);
        static_assert(rotated[1][1] == 5);
        static_assert(rotated[2][0] == 1);
        static_assert(rotated[2][1] == 4);
    }

    return 0;
}