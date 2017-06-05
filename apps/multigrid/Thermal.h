
#ifndef APPS_THERMAL_THERMAL_H_
#define APPS_THERMAL_THERMAL_H_

#include "graph.h"
#include "program.h"
#include "mesh.h"
#include <cmath>

#include "cgnslib.h"

#include "ParameterManager.h"
#include "ParameterManagerMacros.h"
#include "ThermalParameterManager.h"

using namespace simit;

#define TPM ThermalParameterManager

class Thermal {
public:
  Thermal(std::string paramFile, std::string CGNSFileName_L0,
          std::string CGNSFileName_L1,std::string zoneName, int index_zone);
  virtual ~Thermal();

  // Parameter Manager
  TPM PM;
  int Xsize[2],Ysize[2],Zsize[2];

  // Mesh graphs
  std::array<simit::Set*,2> points_MG;
  std::array<simit::Set*,2> quads_MG;	//(points, points, points, points);
  std::array<simit::Set*,2> faces_MG;	//(quads, quads);
  std::array<simit::Set*,2> bcleft_MG;	//(quads);
  std::array<simit::Set*,2> bcright_MG;	//(quads);
  std::array<simit::Set*,2> bcup_MG;	//(quads);
  std::array<simit::Set*,2> bcbottom_MG;//(quads);
  std::array<std::vector<ElementRef>,2> quadsRefs_MG;

  Set *links;

  // input/ouput parameters of the model
  simit::Tensor<double,2> dt;
  simit::Tensor<double,2> cfl;
  simit::Tensor<int,2> 	coupling_direction;
  simit::Tensor<int,2> 	solver_type;
  simit::Tensor<int,2> 	solver_itermax;
  simit::Tensor<double,2> solver_tolerance;
  simit::Tensor<int,2> 	smoother_iter;
  simit::Tensor<int,4> 	bc_types;

  // Simit functions
  Function solve_thermal;
  Function compute_dt;
  Function flux_interface;
  Function temperature_interface;
  void bindSimitFunc(Function *simFunc);

  // Set boundary conditions for coupling applications
  void setBC_qw(Set *bcIn, int i);
  void setBC_Tw(Set *bcIn, int i);
  bool compareTw(Set *bcIn, double tolerance, int i);

};

#endif /* APPS_THERMAL_THERMAL_H_ */
