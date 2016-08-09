#ifndef SIMIT_RUNTIME_H
#define SIMIT_RUNTIME_H

#include "ffi.h"
#include <iostream>

#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>

template<typename Float, int Major=Eigen::RowMajor>
Eigen::SparseMatrix<Float,Major>
csr2eigen(int n, int m, int* rowptr, int* colidx, int nn, int mm, Float* vals) {
  int nnz = rowptr[n/nn];

  std::vector<Eigen::Triplet<Float>> tripletList;
  tripletList.reserve(nnz*nn*mm);
  for (int i=0; i<n/(nn); ++i) {
    for (int ij=rowptr[i]; ij<rowptr[i+1]; ++ij) {
      int j = colidx[ij];
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

template<typename Float,int Major>
void eigen2csr(Eigen::SparseMatrix<Float,Major> mat,
               int n, int m, int** rowptr, int** colidx,
               int nn, int mm, Float** vals) {
  mat.makeCompressed();

  auto nnz = mat.nonZeros();
  *rowptr =   static_cast<int*>(simit::ffi::simit_malloc((n+1) * sizeof(int)));
  *colidx =   static_cast<int*>(simit::ffi::simit_malloc(nnz * sizeof(int)));
  *vals = static_cast<Float*>(simit::ffi::simit_malloc(nnz * sizeof(Float)));

  // copy rowptr
  auto matrowptr = mat.outerIndexPtr();
  for (int i=0; i<n+1; ++i) {
    (*rowptr)[i] = matrowptr[i];
  }

  // copy data and colidx
  auto data = mat.data();
  auto matcolidx = mat.innerIndexPtr();
  for (int i=0; i<nnz; ++i) {
    (*colidx)[i] = matcolidx[i];
    (*vals)[i] = data.value(i);
  }
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
