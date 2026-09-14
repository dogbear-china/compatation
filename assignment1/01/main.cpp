#include "csr_matrix.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

using spmv_course::CsrMatrix;
using spmv_course::Index;

int main() {
    // 稀疏矩阵：
    // [0 2 0 0]
    // [1 0 3 0]
    // [0 0 4 5]
    // [0 0 0 6]
    const std::vector<float> dense = {
        0, 2, 0, 0,
        1, 0, 3, 0,
        0, 0, 4, 5,
        0, 0, 0, 6};
    const std::vector<float> x = {1.0F, 2.0F, 3.0F, 4.0F};

    CsrMatrix matrix;
    matrix.rows = 4;
    matrix.cols = 4;
    matrix.nnz = 6;

    // TODO(01)：根据上面的矩阵补全下面三个 CSR 数组。
    matrix.row_ptr = {0, 0, 0, 0, 0};
    matrix.col_idx = {0, 0, 0, 0, 0, 0};
    matrix.values = {0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F};

    bool csr_is_valid =
        matrix.row_ptr.size() == static_cast<std::size_t>(matrix.rows) + 1U &&
        matrix.col_idx.size() == static_cast<std::size_t>(matrix.nnz) &&
        matrix.values.size() == static_cast<std::size_t>(matrix.nnz) &&
        matrix.row_ptr.front() == 0 && matrix.row_ptr.back() == matrix.nnz;
    std::vector<float> reconstructed(dense.size(), 0.0F);
    if (csr_is_valid) {
        for (Index row = 0; row < matrix.rows && csr_is_valid; ++row) {
            const Index begin = matrix.row_ptr[static_cast<std::size_t>(row)];
            const Index end = matrix.row_ptr[static_cast<std::size_t>(row) + 1U];
            Index previous_col = -1;
            if (begin < 0 || begin > end || end > matrix.nnz) {
                csr_is_valid = false;
                break;
            }
            for (Index k = begin; k < end; ++k) {
                const auto position = static_cast<std::size_t>(k);
                const Index col = matrix.col_idx[position];
                if (col <= previous_col || col < 0 || col >= matrix.cols) {
                    csr_is_valid = false;
                    break;
                }
                reconstructed[
                    static_cast<std::size_t>(row) *
                        static_cast<std::size_t>(matrix.cols) +
                    static_cast<std::size_t>(col)] = matrix.values[position];
                previous_col = col;
            }
        }
    }

    if (!csr_is_valid || reconstructed != dense) {
        std::cerr << "[未通过] CSR 数组还不正确。请完成 TODO(01) 后重新运行。\n";
        return 1;
    }

    std::vector<float> y(static_cast<std::size_t>(matrix.rows), 0.0F);
    for (Index row = 0; row < matrix.rows; ++row) {
        // REPORT-Q01：结合这里的行起止位置，解释 row_ptr[row]
        // 和 row_ptr[row + 1] 分别表示什么，并逐步跟踪 row 2。
        const Index begin = matrix.row_ptr[static_cast<std::size_t>(row)];
        const Index end = matrix.row_ptr[static_cast<std::size_t>(row) + 1U];

        std::cout << "row " << row << ": k in [" << begin << ", " << end << ")\n";
        for (Index k = begin; k < end; ++k) {
            const auto position = static_cast<std::size_t>(k);
            const Index col = matrix.col_idx[position];
            const float product = matrix.values[position] * x[static_cast<std::size_t>(col)];
            std::cout << "  values[" << k << "] * x[col_idx[" << k << "]] = "
                      << matrix.values[position] << " * x[" << col << "] = "
                      << product << '\n';
            y[static_cast<std::size_t>(row)] += product;
        }
    }

    std::cout << "y = [";
    for (std::size_t index = 0; index < y.size(); ++index) {
        std::cout << std::fixed << std::setprecision(1) << y[index]
                  << (index + 1U == y.size() ? "" : ", ");
    }
    std::cout << "]\n[通过] CSR 数组与 SpMV 结果均正确。\n";
    return 0;
}
