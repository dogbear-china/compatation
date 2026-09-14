#include "benchmark_utils.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace spmv_course {
namespace {

volatile float benchmark_sink = 0.0F;

double median_of(std::vector<double> values) {
    if (values.empty()) {
        return 0.0;
    }
    std::sort(values.begin(), values.end());
    const std::size_t middle = values.size() / 2U;
    if ((values.size() & 1U) != 0U) {
        return values[middle];
    }
    return 0.5 * (values[middle - 1U] + values[middle]);
}

}  // namespace

void spmv_reference(const CsrMatrix& matrix, const float* x, float* y) {
    for (Index row = 0; row < matrix.rows; ++row) {
        double sum = 0.0;
        const Index begin = matrix.row_ptr[static_cast<std::size_t>(row)];
        const Index end = matrix.row_ptr[static_cast<std::size_t>(row) + 1U];
        for (Index index = begin; index < end; ++index) {
            const auto position = static_cast<std::size_t>(index);
            sum += static_cast<double>(matrix.values[position]) *
                   static_cast<double>(x[matrix.col_idx[position]]);
        }
        y[row] = static_cast<float>(sum);
    }
}

VerificationResult verify_result(
    const std::vector<float>& expected,
    const std::vector<float>& actual,
    double absolute_tolerance,
    double relative_tolerance) {
    if (expected.size() != actual.size()) {
        throw std::invalid_argument("verification vectors have different sizes");
    }

    VerificationResult result;
    bool recorded_first_failure = false;
    for (std::size_t index = 0; index < expected.size(); ++index) {
        const double reference = static_cast<double>(expected[index]);
        const double observed = static_cast<double>(actual[index]);
        const double absolute_error = std::abs(reference - observed);
        const double relative_error =
            absolute_error / std::max(std::abs(reference), 1.0e-30);
        result.max_abs_error = std::max(result.max_abs_error, absolute_error);
        result.max_rel_error = std::max(result.max_rel_error, relative_error);

        const bool finite = std::isfinite(observed);
        const bool within_tolerance =
            absolute_error <= absolute_tolerance + relative_tolerance * std::abs(reference);
        if ((!finite || !within_tolerance) && !recorded_first_failure) {
            result.ok = false;
            result.first_bad_index = index;
            result.expected_at_first_bad = expected[index];
            result.actual_at_first_bad = actual[index];
            recorded_first_failure = true;
        }
    }
    return result;
}

BenchmarkResult benchmark_kernel(
    SpmvFunction kernel,
    const CsrMatrix& matrix,
    const std::vector<std::vector<float>>& inputs,
    const BenchmarkConfig& config) {
    if (kernel == nullptr) {
        throw std::invalid_argument("benchmark kernel is null");
    }
    if (inputs.empty()) {
        throw std::invalid_argument("benchmark needs at least one input vector");
    }
    if (config.warmup_calls < 0 || config.batches <= 0 ||
        config.minimum_batch_seconds <= 0.0 || config.maximum_iterations == 0) {
        throw std::invalid_argument("invalid benchmark configuration");
    }
    for (const auto& input : inputs) {
        if (input.size() != static_cast<std::size_t>(matrix.cols)) {
            throw std::invalid_argument("benchmark input vector has the wrong size");
        }
    }

    std::vector<float> output(static_cast<std::size_t>(matrix.rows), 0.0F);
    for (int call = 0; call < config.warmup_calls; ++call) {
        const auto& input = inputs[static_cast<std::size_t>(call) % inputs.size()];
        kernel(matrix, input.data(), output.data());
    }

    const auto calibration_start = std::chrono::steady_clock::now();
    kernel(matrix, inputs.front().data(), output.data());
    const auto calibration_end = std::chrono::steady_clock::now();
    const double calibration_seconds = std::max(
        std::chrono::duration<double>(calibration_end - calibration_start).count(),
        1.0e-9);
    const double requested_iterations =
        std::ceil(config.minimum_batch_seconds / calibration_seconds);
    const std::size_t iterations = std::clamp<std::size_t>(
        static_cast<std::size_t>(requested_iterations),
        1U,
        config.maximum_iterations);

    BenchmarkResult result;
    result.iterations_per_batch = iterations;
    result.seconds_per_call.reserve(static_cast<std::size_t>(config.batches));

    std::size_t input_index = 0;
    for (int batch = 0; batch < config.batches; ++batch) {
        const auto start = std::chrono::steady_clock::now();
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            const auto& input = inputs[input_index % inputs.size()];
            kernel(matrix, input.data(), output.data());
            ++input_index;
        }
        const auto end = std::chrono::steady_clock::now();
        const double seconds =
            std::chrono::duration<double>(end - start).count() /
            static_cast<double>(iterations);
        result.seconds_per_call.push_back(seconds);
        if (!output.empty()) {
            benchmark_sink = benchmark_sink + output[input_index % output.size()];
        }
    }

    result.mean_seconds = std::accumulate(
        result.seconds_per_call.begin(),
        result.seconds_per_call.end(),
        0.0) / static_cast<double>(result.seconds_per_call.size());
    result.median_seconds = median_of(result.seconds_per_call);
    const auto limits = std::minmax_element(
        result.seconds_per_call.begin(), result.seconds_per_call.end());
    result.minimum_seconds = *limits.first;
    result.maximum_seconds = *limits.second;
    if (result.median_seconds > 0.0) {
        result.gnnz_per_second =
            static_cast<double>(matrix.nnz) / result.median_seconds / 1.0e9;
    }
    return result;
}

std::string format_bytes(std::size_t bytes) {
    static const char* units[] = {"B", "KiB", "MiB", "GiB"};
    double value = static_cast<double>(bytes);
    std::size_t unit = 0;
    while (value >= 1024.0 && unit + 1U < std::size(units)) {
        value /= 1024.0;
        ++unit;
    }
    std::ostringstream output;
    output << std::fixed << std::setprecision(unit == 0 ? 0 : 2)
           << value << ' ' << units[unit];
    return output.str();
}

}  // namespace spmv_course
