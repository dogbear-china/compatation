#pragma once

#include "csr_matrix.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace spmv_course {

struct VerificationResult {
    bool ok = true;
    std::size_t first_bad_index = 0;
    float expected_at_first_bad = 0.0F;
    float actual_at_first_bad = 0.0F;
    double max_abs_error = 0.0;
    double max_rel_error = 0.0;
};

struct BenchmarkConfig {
    int warmup_calls = 3;
    int batches = 7;
    double minimum_batch_seconds = 0.25;
    std::size_t maximum_iterations = 1U << 20;
};

struct BenchmarkResult {
    std::vector<double> seconds_per_call;
    std::size_t iterations_per_batch = 0;
    double mean_seconds = 0.0;
    double median_seconds = 0.0;
    double minimum_seconds = 0.0;
    double maximum_seconds = 0.0;
    double gnnz_per_second = 0.0;
};

void spmv_reference(const CsrMatrix& matrix, const float* x, float* y);

VerificationResult verify_result(
    const std::vector<float>& expected,
    const std::vector<float>& actual,
    double absolute_tolerance = 1.0e-5,
    double relative_tolerance = 1.0e-4);

BenchmarkResult benchmark_kernel(
    SpmvFunction kernel,
    const CsrMatrix& matrix,
    const std::vector<std::vector<float>>& inputs,
    const BenchmarkConfig& config = {});

std::string format_bytes(std::size_t bytes);

}  // namespace spmv_course
