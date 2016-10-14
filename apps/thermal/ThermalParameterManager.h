#ifndef APPS_THERMAL_THERMALPARAMETERMANAGER_H_
#define APPS_THERMAL_THERMALPARAMETERMANAGER_H_

#include "../thermal/ParameterManager.h"

class ThermalParameterManager:public ParameterManager4<int,double,bool,std::string> {
public:
	static const Parameter<int>    			iterMax;		// Maximum iteration for the solver
	static const Parameter<int>    			iterMax_coupling;		// Maximum iteration for the solver
	static const Parameter<int>    			dumpFrequency;		// Maximum iteration for the solver
	static const Parameter<double> 			timeMax;		// Maximum simulation time
	static const Parameter<double> 			cfl;			// CFL
	static const Parameter<double> 			tolerance_coupling;
	static const Parameter<double> 			T_init;
	static const Parameter<double> 			K;				// kappa
	static const Parameter<double> 			rho;			// rho material
	static const Parameter<double> 			cv;				// cv material
	static const Parameter<double> 			qwl;			// boundary neuman condition on the left
	static const Parameter<double> 			qwr;			// boundary neuman condition on the right
	static const Parameter<double> 			qwu;			// boundary neuman condition on the up
	static const Parameter<double> 			qwb;			// boundary neuman condition on the bottom
	static const Parameter<bool>    		dumpVisit;
	static const Parameter<std::string>    	CGNSFileName;	// Relative path to the CGNS File

	ThermalParameterManager()	{
		initialize();
		updateDependentParameters();
	};
	void updateDependentParameters(void);
	bool readParameters(std::string paramFileName);
private:
	void initialize(void);
};

#endif /* APPS_THERMAL_THERMALPARAMETERMANAGER_H_ */

