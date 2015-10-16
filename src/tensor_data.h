#ifndef SIMIT_TENSOR_DATA_H
#define SIMIT_TENSOR_DATA_H

namespace simit {

// TODO: For now only a way of wrapping sparse tensor data
class TensorData {
public:
  enum Kind {Sparse};
  // rowLen indicates how many int values may be read from rowPtr,
  // dataLen indicates how many int / datatype values may be read
  // from colInd and data respectively.
  TensorData(const int* rowPtr, const int* colInd, void *data,
             int rowLen, int dataLen) :
      kind(Sparse), rowPtr(rowPtr), colInd(colInd), data(data),
      rowLen(rowLen), dataLen(dataLen) {}

  TensorData(TensorData& td) :
      kind(td.kind), rowPtr(td.rowPtr), colInd(td.colInd), data(td.data),
      rowLen(td.rowLen), dataLen(td.dataLen) {}

  Kind getKind() { return kind; }

  void *getData() { return data; }

  const int *getRowPtr() {
    iassert(kind == Sparse);
    return rowPtr;
  }

  const int *getColInd() {
    iassert(kind == Sparse);
    return colInd;
  }

  int getRowLen() { return rowLen; }
  int getDataLen() { return dataLen; }

private:
  Kind kind;

  // Sparse tensor data
  const int* rowPtr;
  const int* colInd;
  void* data;
  int rowLen;
  int dataLen;

};

} // namespace simit

#endif // SIMIT_TENSOR_DATA_H
