#ifndef APPS_MULTIGRID_MGPARAMETERMANAGER_H_
#define APPS_MULTIGRID_MGPARAMETERMANAGER_H_

#include "../thermal/ThermalParameterManager.h"

class MGParameterManager:public ThermalParameterManager {
public:
  static const Parameter<int>    smoother_iter;
  static const Parameter<std::string> CGNSFileName_0;	// Relative path to the CGNS File
  static const Parameter<std::string> CGNSFileName_1;	// Relative path to the CGNS File

  MGParameterManager():ThermalParameterManager()	{
    initializeMG();
    updateDependentParameters();
  };
  bool readParameters(std::string paramFileName);
private:
  void initializeMG(void);
};

#endif /* APPS_MULTIGRID_MGPARAMETERMANAGER_H_ */

