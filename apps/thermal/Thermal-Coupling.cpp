#include "graph.h"
#include "program.h"
#include "mesh.h"
#include <cmath>

#include "Thermal.h"

#include "ParameterManager.h"
#include "ParameterManagerMacros.h"
#include "ThermalParameterManager.h"

using namespace simit;

#define TPM ThermalParameterManager

// Thermal-viz
void DumpToVisit(std::string ZoneName,int iter, double time, Set *quadsPan, Set *pointsPan);

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
	Thermal Pan = Thermal(PM.get(TPM::PanFileName),"Pan",1);

	//4 - Read the Steak part
	Thermal Steak = Thermal(PM.get(TPM::SteakFileName),"Steak",2);

	// Time Loop
	double time=0.0;
	double dt=0.0;
	Pan.compute_dt.runSafe();
	Steak.compute_dt.runSafe();
	dt=min(Steak.dt(0),Pan.dt(0));
	Steak.dt(0)=dt;
	Pan.dt(0)=dt;

	int iter=0;
	if (PM.get(TPM::dumpVisit)) {
		DumpToVisit("Pan",iter, time, Pan.quads, Pan.points);
		DumpToVisit("Steak",iter, time, Steak.quads, Steak.points);
	}
	while ((time < PM.get(TPM::timeMax)) && (iter<PM.get(TPM::iterMax))) {
		iter=iter+1;
		std::cout << "----Iteration " << iter << " ----" << std::endl;

		// Iterate on thermal solving
		Pan.solve_thermal.runSafe();
		Steak.solve_thermal.runSafe();
		time+=dt;

		if ((PM.get(TPM::dumpVisit)) && (iter%(PM.get(TPM::dumpFrequency))==0)) {
			DumpToVisit("Pan",iter, time, Pan.quads, Pan.points);
			DumpToVisit("Steak",iter, time, Steak.quads, Steak.points);
		}
		// Compute the next timestep value
		Pan.compute_dt.runSafe();
		Steak.compute_dt.runSafe();
		dt=min(Steak.dt(0),Pan.dt(0));
		// To arrive just on time
		if (time+dt > PM.get(TPM::timeMax))
			dt = PM.get(TPM::timeMax) - time;
		Steak.dt(0)=dt;
		Pan.dt(0)=dt;
	}
	if (PM.get(TPM::dumpVisit)) {
		DumpToVisit("Pan",iter, time, Pan.quads, Pan.points);
		DumpToVisit("Steak",iter, time, Steak.quads, Steak.points);
	}
}
