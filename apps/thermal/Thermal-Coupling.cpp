#include "graph.h"
#include "program.h"
#include "mesh.h"
#include <cmath>

#include "cgnslib.h"

#include "../thermal/ParameterManager.h"
#include "../thermal/ParameterManagerMacros.h"
#include "../thermal/ThermalParameterManager.h"

using namespace simit;

#define TPM ThermalParameterManager

// Thermal-viz
void DumpToVisit(int iter, double time, Set *quadsPan, Set *pointsPan);
// Thermal model
int Thermal(std::string paramFile, std::string zoneName, simit::Tensor<double,2> *dt, simit::Tensor<double,2> *cflPan,
		    Function *solve_thermal_Pan, Function *compute_dt_Pan,
		    Set *quadsPan, Set *pointsPan, Set *facesPan, Set *bcleftPan, Set *bcrightPan, Set *bcupPan, Set *bcbottomPan );

int main(int argc, char **argv)
{

	//1- Analyse command line arguments
	if (argc != 2) {
		std::cerr << "Usage: thermic <path to .therm file>" << std::endl;
		return -1;
	}
	std::string thermfile = argv[1];


	//2- Construct the parameter manager
	TPM PM;
	PM.readParameters(thermfile);

	//3 - Read the Pan part
	Set pointsPan;
	Set quadsPan(pointsPan, pointsPan, pointsPan, pointsPan);
	Set facesPan(quadsPan,quadsPan);
	Set bcleftPan(quadsPan);
	Set bcrightPan(quadsPan);
	Set bcupPan(quadsPan);
	Set bcbottomPan(quadsPan);
	Function solve_thermal_Pan, compute_dt_Pan;
	simit::Tensor<double,2> dtPan,cflPan;
	Thermal(PM.get(TPM::PanFileName),"Pan",&dtPan,&cflPan,&solve_thermal_Pan,&compute_dt_Pan,
		    &quadsPan,&pointsPan,&facesPan,&bcleftPan,&bcrightPan,&bcupPan,&bcbottomPan);

	// Time Loop
	double time=0.0;
	compute_dt_Pan.runSafe();

	int iter=0;
	if (PM.get(TPM::dumpVisit))
		DumpToVisit(iter, time, &quadsPan, &pointsPan);

	while ((time < PM.get(TPM::timeMax)) && (iter<PM.get(TPM::iterMax))) {
		iter=iter+1;
		std::cout << "----Iteration " << iter << " ----" << std::endl;

		// To arrive just on time
		if (time+dtPan(0) > PM.get(TPM::timeMax))
			dtPan(0) = PM.get(TPM::timeMax) - time;

		// Iterate on thermal solving
		solve_thermal_Pan.runSafe();
		time+=dtPan(0);

		if ((PM.get(TPM::dumpVisit)) && (iter%(PM.get(TPM::dumpFrequency))==0))
			DumpToVisit(iter, time, &quadsPan, &pointsPan);

		// Compute the next timestep value
		compute_dt_Pan.runSafe();
	}
	if (PM.get(TPM::dumpVisit))
		DumpToVisit(iter, time, &quadsPan, &pointsPan);
}
