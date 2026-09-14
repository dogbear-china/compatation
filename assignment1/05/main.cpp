#include "benchmark_utils.hpp"
#include "csr_matrix.hpp"

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using spmv_course::BenchmarkConfig;
using spmv_course::CsrMatrix;
using spmv_course::Index;

namespace {

void spmv_scalar(const CsrMatrix& matrix, const float* x, float* y) {
    for (Index row = 0; row < matrix.rows; ++row) {
        float sum = 0.0F;
        for (Index k = matrix.row_ptr[static_cast<std::size_t>(row)];
             k < matrix.row_ptr[static_cast<std::size_t>(row) + 1U];
             ++k) {
            const auto position = static_cast<std::size_t>(k);
            sum += matrix.values[position] * x[matrix.col_idx[position]];
        }
        y[row] = sum;
    }
}

void run_case(
    const std::string& name,
    const CsrMatrix& matrix,
    const std::vector<std::vector<float>>& inputs,
    const BenchmarkConfig& config) {
    const auto result = spmv_course::benchmark_kernel(
        spmv_scalar, matrix, inputs, config);
    const std::size_t working_set =
        spmv_course::csr_storage_bytes(matrix) +
        static_cast<std::size_t>(matrix.cols) * sizeof(float) +
        static_cast<std::size_t>(matrix.rows) * sizeof(float);
    std::cout << name << ',' << matrix.rows << ',' << matrix.nnz << ','
              << spmv_course::format_bytes(working_set) << ','
              << result.median_seconds * 1000.0 << ','
              << result.gnnz_per_second << '\n';
}

}  // namespace

int main() {
    const std::vector<Index> sizes = {4096, 65536, 1048576};
    constexpr Index nnz_per_row = 16;
    BenchmarkConfig config;
    config.warmup_calls = 3;
    config.batches = 5;
    config.minimum_batch_seconds = 0.10;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "pattern,rows,nnz,working_set,median_ms,gnnz_per_second\n";
    for (Index rows : sizes) {
        const auto inputs = spmv_course::make_input_vectors(rows, 8, 2026U);
        const CsrMatrix banded =
            spmv_course::make_banded_matrix(rows, rows, nnz_per_row);
        run_case("banded", banded, inputs, config);

        const CsrMatrix random =
            spmv_course::make_random_matrix(rows, rows, nnz_per_row, 2026U);
        run_case("random", random, inputs, config);
    }

    // REPORT-Q05：上面两种矩阵仅改变 col_idx 的分布。结合工作集大小和
    // make info 给出的 Cache 容量，解释相同 nnz 为什么会得到不同吞吐率。
    return 0;
}
