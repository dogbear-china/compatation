#include "csr_matrix.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using spmv_course::CsrMatrix;
using spmv_course::Index;

namespace {

constexpr Index kRows = 16;
constexpr Index kCols = 16;
constexpr std::size_t kPartitionCount = 4;

struct MatrixProfile {
    std::vector<Index> row_nnz;
    Index empty_rows = 0;
    Index minimum_row_nnz = 0;
    Index maximum_row_nnz = 0;
    double average_row_nnz = 0.0;
    std::array<Index, kPartitionCount> partition_nnz{};
    double imbalance_ratio = 0.0;
};

CsrMatrix make_matrix(const std::vector<Index>& row_lengths) {
    CsrMatrix matrix;
    matrix.rows = static_cast<Index>(row_lengths.size());
    matrix.cols = kCols;
    matrix.row_ptr.reserve(row_lengths.size() + 1U);
    matrix.row_ptr.push_back(0);

    for (Index row = 0; row < matrix.rows; ++row) {
        const Index row_length = row_lengths[static_cast<std::size_t>(row)];
        if (row_length < 0 || row_length > matrix.cols) {
            throw std::invalid_argument("row length is outside the matrix");
        }
        for (Index slot = 0; slot < row_length; ++slot) {
            matrix.col_idx.push_back((row + slot) % matrix.cols);
            matrix.values.push_back(1.0F);
        }
        matrix.row_ptr.push_back(static_cast<Index>(matrix.values.size()));
    }

    matrix.nnz = static_cast<Index>(matrix.values.size());
    spmv_course::validate_csr(matrix);
    return matrix;
}

CsrMatrix make_uniform_matrix() {
    return make_matrix(std::vector<Index>(kRows, 4));
}

CsrMatrix make_skewed_matrix() {
    return make_matrix({
        0, 0, 0, 0,
        1, 1, 1, 1,
        3, 3, 3, 3,
        12, 12, 12, 12,
    });
}

MatrixProfile analyze_matrix(const CsrMatrix& matrix) {
    MatrixProfile profile;
    profile.row_nnz.reserve(static_cast<std::size_t>(matrix.rows));

    for (Index row = 0; row < matrix.rows; ++row) {
        // REPORT-Q06：一行的工作量可由相邻两个 row_ptr 的差得到。
        const Index row_nnz =
            matrix.row_ptr[static_cast<std::size_t>(row) + 1U] -
            matrix.row_ptr[static_cast<std::size_t>(row)];
        profile.row_nnz.push_back(row_nnz);
        profile.empty_rows += row_nnz == 0 ? 1 : 0;
    }

    if (!profile.row_nnz.empty()) {
        const auto limits = std::minmax_element(
            profile.row_nnz.begin(), profile.row_nnz.end());
        profile.minimum_row_nnz = *limits.first;
        profile.maximum_row_nnz = *limits.second;
        profile.average_row_nnz =
            static_cast<double>(matrix.nnz) / static_cast<double>(matrix.rows);
    }

    for (std::size_t part = 0; part < kPartitionCount; ++part) {
        const Index begin = static_cast<Index>(
            static_cast<std::size_t>(matrix.rows) * part / kPartitionCount);
        const Index end = static_cast<Index>(
            static_cast<std::size_t>(matrix.rows) * (part + 1U) /
            kPartitionCount);
        profile.partition_nnz[part] =
            matrix.row_ptr[static_cast<std::size_t>(end)] -
            matrix.row_ptr[static_cast<std::size_t>(begin)];
    }

    const Index maximum_partition_nnz = *std::max_element(
        profile.partition_nnz.begin(), profile.partition_nnz.end());
    const double average_partition_nnz =
        static_cast<double>(matrix.nnz) /
        static_cast<double>(kPartitionCount);
    if (average_partition_nnz > 0.0) {
        profile.imbalance_ratio =
            static_cast<double>(maximum_partition_nnz) /
            average_partition_nnz;
    }
    return profile;
}

void print_profile(
    const std::string& pattern,
    const CsrMatrix& matrix,
    const MatrixProfile& profile) {
    std::cout << "===== " << pattern << " =====\n";
    std::cout << "rows=" << matrix.rows
              << " cols=" << matrix.cols
              << " nnz=" << matrix.nnz << '\n';
    std::cout << "row_nnz=";
    for (std::size_t row = 0; row < profile.row_nnz.size(); ++row) {
        std::cout << (row == 0 ? "" : ",") << profile.row_nnz[row];
    }
    std::cout << '\n';
    std::cout << "empty_rows=" << profile.empty_rows
              << " min_row_nnz=" << profile.minimum_row_nnz
              << " max_row_nnz=" << profile.maximum_row_nnz
              << " average_row_nnz=" << std::fixed << std::setprecision(2)
              << profile.average_row_nnz << '\n';
    std::cout << "partition,row_range,local_nnz\n";
    for (std::size_t part = 0; part < kPartitionCount; ++part) {
        const Index begin = static_cast<Index>(
            static_cast<std::size_t>(matrix.rows) * part / kPartitionCount);
        const Index end = static_cast<Index>(
            static_cast<std::size_t>(matrix.rows) * (part + 1U) /
            kPartitionCount);
        std::cout << part << ",[" << begin << ',' << end << "),"
                  << profile.partition_nnz[part] << '\n';
    }
    std::cout << "imbalance_ratio=" << profile.imbalance_ratio << "\n\n";
}

bool check_profiles(
    const CsrMatrix& uniform,
    const MatrixProfile& uniform_profile,
    const CsrMatrix& skewed,
    const MatrixProfile& skewed_profile) {
    const bool same_scale =
        uniform.rows == skewed.rows &&
        uniform.cols == skewed.cols &&
        uniform.nnz == skewed.nnz;
    const bool uniform_ok =
        uniform_profile.empty_rows == 0 &&
        uniform_profile.minimum_row_nnz == 4 &&
        uniform_profile.maximum_row_nnz == 4 &&
        uniform_profile.partition_nnz ==
            std::array<Index, kPartitionCount>{16, 16, 16, 16} &&
        uniform_profile.imbalance_ratio == 1.0;
    const bool skewed_ok =
        skewed_profile.empty_rows == 4 &&
        skewed_profile.minimum_row_nnz == 0 &&
        skewed_profile.maximum_row_nnz == 12 &&
        skewed_profile.partition_nnz ==
            std::array<Index, kPartitionCount>{0, 4, 12, 48} &&
        skewed_profile.imbalance_ratio == 3.0;

    if (!same_scale || !uniform_ok || !skewed_ok) {
        std::cerr << "[失败] 矩阵规模或行工作量统计与预期不符。\n";
        return false;
    }
    std::cout << "[通过] 两个矩阵规模相同，行工作量与分区统计正确。\n";
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    const CsrMatrix uniform = make_uniform_matrix();
    const CsrMatrix skewed = make_skewed_matrix();
    const MatrixProfile uniform_profile = analyze_matrix(uniform);
    const MatrixProfile skewed_profile = analyze_matrix(skewed);

    if (argc > 1 && std::strcmp(argv[1], "--check") == 0) {
        return check_profiles(
                   uniform, uniform_profile, skewed, skewed_profile)
                   ? 0
                   : 1;
    }

    std::cout << "以下分区由串行程序依次统计，不会创建并行执行单元。\n\n";
    print_profile("uniform", uniform, uniform_profile);
    print_profile("skewed", skewed, skewed_profile);
    return 0;
}
