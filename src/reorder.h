#ifndef SIMIT_REORDER_H
#define SIMIT_REORDER_H

#include <graph.h>
#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <fstream>

namespace simit {
  
  void reorder(Set& edgeSet, Set& vertexSet, std::vector<int>& vertexOrdering);
 
  template<typename T, int... dimensions>
  void populateEdgeSetSpatialField(Set& edgeSet, Set& vertexSet,FieldRef<T, dimensions...>& edgeSpatialField, FieldRef<T, dimensions...>& spatialField, std::string& name) {
    //FieldRef<T, dimensions...> edgeSpatialField = edgeSet.addSpatialField<T, dimensions...>(name);
    auto& fields = vertexSet.getFields();
    int fieldIndex = vertexSet.getFieldIndex("x");
    T* spatialData = static_cast<T*>(fields[fieldIndex]->data);
    
    std::ofstream output;
    output.open("/data/scratch/ptew/simit/test/input/program/fem/dragon-hilbert-edge-reorderd.ele");
    output << edgeSet.getSize() << " 3 0 0\n"; 

    int dim = 3;
    int cardinality = edgeSet.getCardinality(); 
    std::vector<float> sum(3,0);
    for (auto& element : edgeSet) {
      for (auto& endpoint : edgeSet.getEndpoints(element)) {
        for (int z=0; z < dim; ++z) {
          sum[z] += spatialData[endpoint.getIdent()*dim + z]/edgeSet.getCardinality();
        }
        //if ( element.getIdent() < 5 ) {
        //  std::cout << spatialData[endpoint.getIdent()*dim + 0] << ", ";
        //  std::cout << spatialData[endpoint.getIdent()*dim + 1] << ", ";
        //  std::cout << spatialData[endpoint.getIdent()*dim + 2];
        //  std::cout << " + " << std::endl;
        //}
      }
      edgeSpatialField.set(element, sum);
      //if ( element.getIdent() < 5 ) {
      //  std::cout << "==========================" << std::endl;
      //  std::cout << sum[0] << ", ";
      //  std::cout << sum[1] << ", ";
      //  std::cout << sum[2];
      //  std::cout << "\n" << std::endl;
      //}
      std::fill(sum.begin(), sum.end(), 0);
    }
    output.close();
    //float* data = static_cast<float *>(edgeSet.getFieldData(name));
    //std::cout << "0: " << data[0] << std::endl;
    //std::cout << "1: " << data[1] << std::endl;
    //std::cout << "2: " << data[2] << std::endl;
  }
  
  template<typename T>
  void reorderFieldData(T* data, const std::vector<int>& vertexOrdering, const int capacity, const int typeSize) {
    T* newData = static_cast<T*>(malloc(capacity * typeSize));
    int dim = typeSize/sizeof(T);
    assert(dim > 0);

    for ( int i=0; i<vertexOrdering.size(); ++i) {
      for ( int x=0; x<dim; ++x) {
        assert(vertexOrdering[i]*dim + x < vertexOrdering.size() * dim);
        newData[vertexOrdering[i]*dim+x] = data[i*dim+x];
      }
    }

    memcpy(data, newData, capacity * typeSize); 
    delete newData;
  }
} // namespace simit 
#endif
