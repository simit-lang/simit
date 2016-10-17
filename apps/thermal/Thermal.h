
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
	Thermal(std::string paramFile, std::string zoneName, int index_zone);
	virtual ~Thermal();
	TPM PM;

	Set *points;
	Set *quads;		//(points, points, points, points);
	Set *faces;		//(quads, quads);
	Set *bcleft;	//(quads);
	Set *bcright;	//(quads);
	Set *bcup;		//(quads);
	Set *bcbottom;	//(quads);

	simit::Tensor<double,2> dt;
	simit::Tensor<double,2> cfl;
	Function solve_thermal;
	Function compute_dt;
};

#endif /* APPS_THERMAL_THERMAL_H_ */
