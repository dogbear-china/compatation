#include "benchmark_utils.hpp"
#include "csr_matrix.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using spmv_course::CsrMatrix;
using spmv_course::Index;

namespace {

void spmv_student(const CsrMatrix& matrix, const float* x, float* y) {
    (void)x;
    for (Index row = 0; row < matrix.rows; ++row) {
        float sum = 0.0F;

        // TODO(02)：使用 row_ptr 找到本行范围，并累加 values[k] * x[col_idx[k]]。
        // REPORT-Q02：完成后，结合测试输出解释空行与矩形矩阵的处理方式。

        y[row] = sum;
    }
}

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

bool run_case(const TestCase& test_case) {
    std::vector<float> expected(static_cast<std::size_t>(test_case.matrix.rows), 0.0F);
    std::vector<float> actual(
        static_cast<std::size_t>(test_case.matrix.rows),
        std::numeric_limits<float>::quiet_NaN());
    spmv_course::spmv_reference(
        test_case.matrix, test_case.x.data(), expected.data());
    spmv_student(test_case.matrix, test_case.x.data(), actual.data());
    const auto verification = spmv_course::verify_result(expected, actual);
    if (!verification.ok) {
        std::cout << "[失败] " << test_case.name
                  << "：y[" << verification.first_bad_index << "] 期望 "
                  << verification.expected_at_first_bad << "，实际 "
                  << verification.actual_at_first_bad << '\n';
        return false;
    }
    std::cout << "[通过] " << test_case.name << '\n';
    return true;
}

}  // namespace

int main() {
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

    std::vector<float> long_row(128, 1.0F);
    std::vector<float> long_x(128, 0.5F);
    cases.push_back(make_case(
        "单一长行",
        1,
        128,
        std::move(long_row),
        std::move(long_x)));
    cases.push_back(make_case(
        "正负数抵消",
        1,
        4,
        {4, -4, 2, -2},
        {1, 1, 1, 1}));

    int passed = 0;
    for (const TestCase& test_case : cases) {
        passed += run_case(test_case) ? 1 : 0;
    }
    std::cout << "\n公开测试：" << passed << '/' << cases.size() << " 通过。\n";
    if (passed != static_cast<int>(cases.size())) {
        std::cout << "请完成 TODO(02)，并从第一个失败用例开始排查。\n";
        return 1;
    }
    return 0;
}
