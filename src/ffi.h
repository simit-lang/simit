#ifndef SIMIT_FFI_H
#define SIMIT_FFI_H

#include <tuple>


/// Converts a Simit blocked matrix into a CSR matrix.
template <typename Float>
void convertToCSR(Float* bufferA,
                  int* row_start, int* col_idx,
                  int rows, int columns, int bs_x, int bs_y,
                  int** csrRowStart, int** csrColIdx, Float** csrVals) {
  int nnz = row_start[rows/bs_x];

  // create tuples for each matrix entry
  std::vector<std::tuple<int,int,Float>> entries;
  entries.reserve(nnz*bs_x*bs_y);
  for (int i=0; i<rows/bs_x; i++) {
    for (int j=row_start[i]; j<row_start[i+1]; j++) {
      for (int bi=0; bi<bs_x; bi++) {
        for (int bj=0; bj<bs_y; bj++) {
          Float val = bufferA[j*bs_x*bs_y+bs_x*bi+bj];
          entries.push_back(std::tuple<int,int,Float>(i*bs_x+bi,
                                                            col_idx[j]*bs_y+bj,
                                                            val));
        }
      }
    }
  }

  // sort the tuples
  std::sort(entries.begin(), entries.end());
  
  // build the matrix
  *csrRowStart = (int*)malloc((rows+1) * sizeof(int));
  *csrColIdx = (int*)malloc(nnz*bs_x*bs_y * sizeof(int));
  *csrVals = (Float*)malloc(nnz*bs_x*bs_y * sizeof(Float));
 
  // determine row lengths
  for (int i=0; i<=rows; i++)
    (*csrRowStart)[i] = 0;
  for (int i=0; i<nnz*bs_x*bs_y; i++)
    (*csrRowStart)[std::get<0>(entries[i])+1]++;
  for (int i=0; i<rows; i++)
    (*csrRowStart)[i+1] += (*csrRowStart)[i];
 
  // fill in col_idx and vals
  for (int l=0; l<nnz*bs_x*bs_y; l++) {
    int i = (*csrRowStart)[std::get<0>(entries[l])];
    (*csrVals)[i] = std::get<2>(entries[l]);
    (*csrColIdx)[i] = std::get<1>(entries[l]);
    (*csrRowStart)[std::get<0>(entries[l])]++;
  }
  
  // shift back row_start
  for (int i=rows; i>0; i--) (*csrRowStart)[i] = (*csrRowStart)[i-1];
  
  (*csrRowStart)[0] = 0;
}
#endif
