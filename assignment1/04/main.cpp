#include "benchmark_utils.hpp"
#include "csr_matrix.hpp"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

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

double median(std::vector<double> values) {
    std::sort(values.begin(), values.end());
    const std::size_t middle = values.size() / 2U;
    return values[middle];
}

}  // namespace

int main() {
    constexpr Index rows = 65536;
    constexpr Index nnz_per_row = 16;

    const auto bad_start = std::chrono::steady_clock::now();
    CsrMatrix bad_matrix =
        spmv_course::make_random_matrix(rows, rows, nnz_per_row, 2026U);
    std::vector<float> bad_x = spmv_course::make_input_vector(rows, 2026U);
    std::vector<float> bad_y(static_cast<std::size_t>(rows), 0.0F);
    std::vector<float> bad_expected(static_cast<std::size_t>(rows), 0.0F);
    spmv_scalar(bad_matrix, bad_x.data(), bad_y.data());
    spmv_course::spmv_reference(
        bad_matrix, bad_x.data(), bad_expected.data());
    const auto bad_verification =
        spmv_course::verify_result(bad_expected, bad_y);
    const auto bad_end = std::chrono::steady_clock::now();
    const double bad_seconds =
        std::chrono::duration<double>(bad_end - bad_start).count();

    const CsrMatrix matrix =
        spmv_course::make_random_matrix(rows, rows, nnz_per_row, 2026U);
    const auto inputs = spmv_course::make_input_vectors(rows, 8, 3000U);
    const auto result = spmv_course::benchmark_kernel(
        spmv_scalar, matrix, inputs);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "错误示例（生成+分配+一次内核+验证）："
              << bad_seconds * 1000.0 << " ms，结果 "
              << (bad_verification.ok ? "正确" : "错误") << "\n\n";

    std::cout << "正确示例只测内核，每批 "
              << result.iterations_per_batch << " 次：\n";
    for (std::size_t index = 0; index < result.seconds_per_call.size(); ++index) {
        std::cout << "  batch " << index + 1U << ": "
                  << result.seconds_per_call[index] * 1000.0 << " ms/call\n";
    }
    std::cout << "  mean   = " << result.mean_seconds * 1000.0 << " ms\n";
    std::cout << "  median = " << result.median_seconds * 1000.0 << " ms\n";
    std::cout << "  range  = [" << result.minimum_seconds * 1000.0 << ", "
              << result.maximum_seconds * 1000.0 << "] ms\n";

    std::vector<double> simulated = result.seconds_per_call;
    simulated.back() *= 5.0;
    const double simulated_mean = std::accumulate(
        simulated.begin(), simulated.end(), 0.0) /
        static_cast<double>(simulated.size());
    std::cout << "\n模拟最后一批受到系统干扰、耗时放大 5 倍：\n";
    std::cout << "  mean   = " << simulated_mean * 1000.0 << " ms\n";
    std::cout << "  median = " << median(simulated) * 1000.0 << " ms\n";

    // REPORT-Q04：比较两种计时边界，并根据真实样本和模拟异常值解释
    // 预热、重复测量、中位数和绑核的作用。
    return 0;
}
