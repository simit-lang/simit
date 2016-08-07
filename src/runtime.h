#ifndef SIMIT_RUNTIME_H
#define SIMIT_RUNTIME_H

#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>

template<typename Float, int Major=Eigen::RowMajor>
Eigen::SparseMatrix<Float,Major>
csr2eigen(int n, int m, int* rowPtr, int* colIdx, int nn, int mm, Float* vals) {
  int nnz = rowPtr[n/nn];

  std::vector<Eigen::Triplet<Float>> tripletList;
  tripletList.reserve(nnz*nn*mm);
  for (int i=0; i<n/(nn); ++i) {
    for (int ij=rowPtr[i]; ij<rowPtr[i+1]; ++ij) {
      int j = colIdx[ij];
      for (int bi=0; bi<nn; bi++) {
        for (int bj=0; bj<mm; bj++) {
          tripletList.push_back(Eigen::Triplet<Float>(i*nn+bi, j*mm+bj,
                                                      vals[ij*nn*mm+bi*nn+bj]));
        }
      }
    }
  }
  
  Eigen::SparseMatrix<Float> mat(n, m);
  mat.setFromTriplets(tripletList.begin(), tripletList.end());
  mat.makeCompressed();
  return mat;
}

template<typename Float> Eigen::Matrix<Float,Eigen::Dynamic,1>
dense2eigen(int n, Float* vals) {
  auto result = Eigen::Matrix<Float,Eigen::Dynamic,1>(n);
  for (int i=0; i<n; ++i) {
    result(i) = vals[i];
  }
  return result;
}
#endif

#endif
