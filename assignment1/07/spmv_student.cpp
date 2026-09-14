#include "csr_matrix.hpp"

using spmv_course::CsrMatrix;
using spmv_course::Index;

void spmv_student(const CsrMatrix& matrix, const float* x, float* y) {
    (void)x;
    for (Index row = 0; row < matrix.rows; ++row) {
        // TODO(07)：先完成正确的 CSR SpMV，再基于 01～06 的实验结论优化。
        // REPORT-Q07：只允许修改本文件；正式排行只计本函数的执行时间。
        y[row] = 0.0F;
    }
}
