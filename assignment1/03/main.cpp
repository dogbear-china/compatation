#include "csr_matrix.hpp"

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>

using spmv_course::CsrMatrix;
using spmv_course::Index;

int main() {
    constexpr Index rows = 4096;
    constexpr Index nnz_per_row = 16;
    const CsrMatrix matrix =
        spmv_course::make_banded_matrix(rows, rows, nnz_per_row);

    const std::size_t row_iterations = static_cast<std::size_t>(matrix.rows);
    const std::size_t nonzero_iterations = static_cast<std::size_t>(matrix.nnz);
    const std::size_t floating_point_operations = 2U * nonzero_iterations;

    const std::size_t values_bytes = nonzero_iterations * sizeof(float);
    const std::size_t col_idx_bytes = nonzero_iterations * sizeof(Index);
    const std::size_t x_bytes_if_not_cached = nonzero_iterations * sizeof(float);
    const std::size_t row_ptr_bytes = matrix.row_ptr.size() * sizeof(Index);
    const std::size_t y_bytes = row_iterations * sizeof(float);
    const std::size_t estimated_bytes = values_bytes + col_idx_bytes +
                                        x_bytes_if_not_cached + row_ptr_bytes + y_bytes;
    const double arithmetic_intensity =
        static_cast<double>(floating_point_operations) /
        static_cast<double>(estimated_bytes);

    // REPORT-Q03：这里统计的是算法的逻辑工作量，而不是某次运行的硬件计数器。
    // 请说明缓存命中后，哪些字节不一定每次都来自主存。
    std::cout << "rows                 = " << matrix.rows << '\n';
    std::cout << "nnz                  = " << matrix.nnz << '\n';
    std::cout << "row loop iterations  = " << row_iterations << '\n';
    std::cout << "inner loop iterations= " << nonzero_iterations << '\n';
    std::cout << "floating-point ops    = " << floating_point_operations << '\n';
    std::cout << "values bytes          = " << values_bytes << '\n';
    std::cout << "col_idx bytes         = " << col_idx_bytes << '\n';
    std::cout << "x bytes (uncached)    = " << x_bytes_if_not_cached << '\n';
    std::cout << "row_ptr bytes         = " << row_ptr_bytes << '\n';
    std::cout << "y bytes               = " << y_bytes << '\n';
    std::cout << "estimated total bytes = " << estimated_bytes << '\n';
    std::cout << std::fixed << std::setprecision(4)
              << "estimated FLOP/byte    = " << arithmetic_intensity << '\n';
    return 0;
}
