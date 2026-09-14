#include "benchmark_utils.hpp"
#include "csr_matrix.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using spmv_course::BenchmarkConfig;
using spmv_course::CsrMatrix;
using spmv_course::Index;

void spmv_student(const CsrMatrix& matrix, const float* x, float* y);

namespace {

struct TestCase {
    std::string name;
    CsrMatrix matrix;
    std::vector<float> x;
};

TestCase make_case(
    std::string name,
    Index rows,
    Index cols,
    std::vector<float> dense,
    std::vector<float> x) {
    return {
        std::move(name),
        spmv_course::make_csr_from_dense(rows, cols, dense),
        std::move(x)};
}

bool same_matrix(const CsrMatrix& left, const CsrMatrix& right) {
    return left.rows == right.rows && left.cols == right.cols &&
           left.nnz == right.nnz && left.row_ptr == right.row_ptr &&
           left.col_idx == right.col_idx && left.values == right.values;
}

bool run_case(const TestCase& test_case, bool verbose) {
    const CsrMatrix original_matrix = test_case.matrix;
    const std::vector<float> original_x = test_case.x;
    std::vector<float> expected(static_cast<std::size_t>(test_case.matrix.rows), 0.0F);
    spmv_course::spmv_reference(
        test_case.matrix, test_case.x.data(), expected.data());

    constexpr float guard = 1234567.0F;
    std::vector<float> guarded_output(
        static_cast<std::size_t>(test_case.matrix.rows) + 2U,
        std::numeric_limits<float>::quiet_NaN());
    guarded_output.front() = guard;
    guarded_output.back() = guard;
    float* output = guarded_output.data() + 1;
    spmv_student(test_case.matrix, test_case.x.data(), output);

    std::vector<float> actual(
        output,
        output + static_cast<std::size_t>(test_case.matrix.rows));
    const auto verification = spmv_course::verify_result(expected, actual);
    const bool guards_ok =
        guarded_output.front() == guard && guarded_output.back() == guard;
    const bool inputs_ok =
        same_matrix(test_case.matrix, original_matrix) && test_case.x == original_x;
    const bool ok = verification.ok && guards_ok && inputs_ok;

    if (verbose) {
        if (ok) {
            std::cout << "[通过] " << test_case.name << '\n';
        } else if (!guards_ok) {
            std::cout << "[失败] " << test_case.name << "：检测到 y 越界写入\n";
        } else if (!inputs_ok) {
            std::cout << "[失败] " << test_case.name << "：输入矩阵或 x 被修改\n";
        } else {
            std::cout << "[失败] " << test_case.name << "：y["
                      << verification.first_bad_index << "] 期望 "
                      << verification.expected_at_first_bad << "，实际 "
                      << verification.actual_at_first_bad << '\n';
        }
    }
    return ok;
}

std::vector<TestCase> public_cases() {
    std::vector<TestCase> cases;
    cases.push_back(make_case(
        "普通 4x4",
        4,
        4,
        {0, 2, 0, 0, 1, 0, 3, 0, 0, 0, 4, 5, 0, 0, 0, 6},
        {1, 2, 3, 4}));
    cases.push_back(make_case(
        "中间含空行",
        3,
        3,
        {1, 0, 2, 0, 0, 0, 3, 0, 4},
        {1, 2, 3}));
    cases.push_back(make_case(
        "全空行矩阵",
        3,
        5,
        std::vector<float>(15, 0.0F),
        {1, 2, 3, 4, 5}));
    cases.push_back(make_case(
        "首行和末行为空",
        4,
        3,
        {0, 0, 0, 1, 2, 0, 0, 3, 4, 0, 0, 0},
        {1, 2, 3}));
    cases.push_back(make_case(
        "宽矩阵 2x4",
        2,
        4,
        {1, 0, 2, 0, 0, 3, 0, 4},
        {1, 2, 3, 4}));
    cases.push_back(make_case(
        "高矩阵 4x2",
        4,
        2,
        {1, 0, 0, 2, 3, 4, 0, 5},
        {2, -1}));
    cases.push_back(make_case("0x0", 0, 0, {}, {}));
    cases.push_back(make_case("1x1", 1, 1, {7}, {3}));

    std::vector<float> long_row(1024, 1.0F);
    std::vector<float> long_x(1024, 0.5F);
    cases.push_back(make_case(
        "单一长行",
        1,
        1024,
        std::move(long_row),
        std::move(long_x)));
    cases.push_back(make_case(
        "正负数抵消",
        1,
        4,
        {4, -4, 2, -2},
        {1, 1, 1, 1}));

    CsrMatrix random = spmv_course::make_random_matrix(127, 131, 7, 2026U);
    cases.push_back({
        "固定种子随机矩阵",
        std::move(random),
        spmv_course::make_input_vector(131, 2027U)});
    return cases;
}

bool run_repeated_call_test(bool verbose) {
    const CsrMatrix matrix = spmv_course::make_banded_matrix(64, 64, 5);
    const auto first_x = spmv_course::make_input_vector(64, 1U);
    const auto second_x = spmv_course::make_input_vector(64, 2U);
    std::vector<float> expected(static_cast<std::size_t>(matrix.rows), 0.0F);
    std::vector<float> actual(
        static_cast<std::size_t>(matrix.rows),
        std::numeric_limits<float>::quiet_NaN());

    spmv_course::spmv_reference(matrix, first_x.data(), expected.data());
    spmv_student(matrix, first_x.data(), actual.data());
    const bool first_ok = spmv_course::verify_result(expected, actual).ok;

    std::fill(actual.begin(), actual.end(), std::numeric_limits<float>::quiet_NaN());
    spmv_course::spmv_reference(matrix, second_x.data(), expected.data());
    spmv_student(matrix, second_x.data(), actual.data());
    const bool second_ok = spmv_course::verify_result(expected, actual).ok;
    const bool ok = first_ok && second_ok;
    if (verbose) {
        std::cout << (ok ? "[通过] " : "[失败] ")
                  << "同一矩阵更换 x 后重复调用\n";
    }
    return ok;
}

bool run_public_tests(bool verbose) {
    const auto cases = public_cases();
    int passed = 0;
    for (const TestCase& test_case : cases) {
        passed += run_case(test_case, verbose) ? 1 : 0;
    }

    passed += run_repeated_call_test(verbose) ? 1 : 0;
    const int total = static_cast<int>(cases.size()) + 1;

    if (verbose) {
        std::cout << "\n公开测试：" << passed << '/' << total << " 通过。\n";
    }
    return passed == total;
}

struct CsvWriter {
    explicit CsvWriter(const std::string& path) : file(path) {
        if (!file) {
            throw std::runtime_error("无法创建结果文件：" + path);
        }
        file << "matrix,rows,cols,nnz,setup_ms,kernel_ms,end_to_end_ms,"
                "gnnz_per_second,max_abs_error,max_rel_error,iterations_per_batch\n";
    }

    std::ofstream file;
};

void benchmark_case(
    const std::string& name,
    CsrMatrix matrix,
    double setup_seconds,
    CsvWriter& csv) {
    auto inputs = spmv_course::make_input_vectors(matrix.cols, 8, 4000U);
    for (auto& input : inputs) {
        for (float& value : input) {
            // Keep the public performance data numerically well-conditioned.
            // Signed cancellation is tested separately by the correctness suite.
            value = 0.25F + std::abs(value);
        }
    }
    std::vector<float> expected(static_cast<std::size_t>(matrix.rows), 0.0F);
    std::vector<float> actual(static_cast<std::size_t>(matrix.rows), 0.0F);
    double max_abs_error = 0.0;
    double max_rel_error = 0.0;
    for (const auto& input : inputs) {
        spmv_course::spmv_reference(matrix, input.data(), expected.data());
        std::fill(actual.begin(), actual.end(), std::numeric_limits<float>::quiet_NaN());
        spmv_student(matrix, input.data(), actual.data());
        const auto verification = spmv_course::verify_result(expected, actual);
        if (!verification.ok) {
            throw std::runtime_error(
                "基准矩阵 " + name + " 在 y[" +
                std::to_string(verification.first_bad_index) +
                "] 正确性检查失败：期望 " +
                std::to_string(verification.expected_at_first_bad) +
                "，实际 " +
                std::to_string(verification.actual_at_first_bad));
        }
        max_abs_error = std::max(max_abs_error, verification.max_abs_error);
        max_rel_error = std::max(max_rel_error, verification.max_rel_error);
    }

    BenchmarkConfig config;
    config.warmup_calls = 3;
    config.batches = 7;
    config.minimum_batch_seconds = 0.25;
    const auto result = spmv_course::benchmark_kernel(
        spmv_student, matrix, inputs, config);
    const double setup_ms = setup_seconds * 1000.0;
    const double kernel_ms = result.median_seconds * 1000.0;
    csv.file << std::fixed << std::setprecision(8)
             << name << ',' << matrix.rows << ',' << matrix.cols << ',' << matrix.nnz << ','
             << setup_ms << ',' << kernel_ms << ',' << setup_ms + kernel_ms << ','
             << result.gnnz_per_second << ',' << max_abs_error << ','
             << max_rel_error << ',' << result.iterations_per_batch << '\n';
    csv.file.flush();
    std::cout << std::left << std::setw(18) << name
              << " kernel=" << std::right << std::setw(10)
              << std::fixed << std::setprecision(4) << kernel_ms << " ms"
              << "  GNNZ/s=" << std::setprecision(4)
              << result.gnnz_per_second << '\n';
}

template <typename Generator>
void benchmark_generated(
    const std::string& name,
    Generator generator,
    CsvWriter& csv) {
    const auto start = std::chrono::steady_clock::now();
    CsrMatrix matrix = generator();
    const auto end = std::chrono::steady_clock::now();
    benchmark_case(
        name,
        std::move(matrix),
        std::chrono::duration<double>(end - start).count(),
        csv);
}

int run_benchmark(const std::string& output_path) {
    if (!run_public_tests(false)) {
        std::cerr << "公开正确性测试未全部通过，拒绝开始性能测试。\n";
        return 1;
    }

    CsvWriter csv(output_path);
    constexpr Index rows = 262144;
    benchmark_generated(
        "banded_public",
        [] { return spmv_course::make_banded_matrix(rows, rows, 16); },
        csv);
    benchmark_generated(
        "random_public",
        [] { return spmv_course::make_random_matrix(rows, rows, 16, 2026U); },
        csv);
    benchmark_generated(
        "skewed_public",
        [] { return spmv_course::make_skewed_matrix(rows, rows, 2026U); },
        csv);

    const char* data_root_env = std::getenv("SPMV_DATA_ROOT");
    const std::string data_root =
        data_root_env != nullptr && data_root_env[0] != '\0'
            ? data_root_env
            : "../data";
    const std::string path = data_root + "/cage11.mtx";
    const auto start = std::chrono::steady_clock::now();
    CsrMatrix matrix = spmv_course::load_matrix_market(path);
    const auto end = std::chrono::steady_clock::now();
    benchmark_case(
        "cage11",
        std::move(matrix),
        std::chrono::duration<double>(end - start).count(),
        csv);

    std::cout << "结果已写入 " << output_path << '\n';
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const std::string command = argc > 1 ? argv[1] : "test";
        if (command == "test") {
            return run_public_tests(true) ? 0 : 1;
        }
        if (command == "benchmark") {
            const std::string output = argc > 2 ? argv[2] : "results/latest.csv";
            return run_benchmark(output);
        }
        std::cerr << "用法：" << argv[0] << " [test | benchmark [csv路径]]\n";
        return 2;
    } catch (const std::exception& error) {
        std::cerr << "错误：" << error.what() << '\n';
        return 2;
    }
}
