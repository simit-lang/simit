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
void DumpToVisit(std::string ZoneName,int iter, double tim, int Xsize, int Ysize, Set *quadsPan, Set *pointsPan);

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
	Thermal Pan = Thermal(PM.get(TPM::PanFileName),PM.get(TPM::CGNSFileName),"Pan",1);

	//4 - Read the Steak part
	Thermal Steak = Thermal(PM.get(TPM::SteakFileName),PM.get(TPM::CGNSFileName),"Steak",2);

	// Compute first time step
	double time=0.0;
	double dt=0.0;
	Pan.compute_dt.runSafe();
	Steak.compute_dt.runSafe();
	dt=min(Steak.dt(0),Pan.dt(0));
	Steak.dt(0)=dt;
	Pan.dt(0)=dt;
	int iterMax_coupling=PM.get(TPM::iterMax_coupling);

	// Interface boundary condition : Dirichlet for the Pan - Neuman for the steak
	Pan.bc_types(Pan.coupling_direction(0))=1;
	// Initialize interface
	Steak.temperature_interface.runSafe();

	int iter=0;
	if (PM.get(TPM::dumpVisit)) {
		DumpToVisit("Pan",iter, time, Pan.Xsize, Pan.Ysize, Pan.quads, Pan.points);
		DumpToVisit("Steak",iter, time, Pan.Xsize, Pan.Ysize, Steak.quads, Steak.points);
	}
	// Time loop
	while ((time < PM.get(TPM::timeMax)) && (iter<PM.get(TPM::iterMax))) {
		iter=iter+1;
		std::cout << "---- Iteration " << iter << " ----" << std::endl;
		std::cout << "  -- Time " << time << " -- dt " << dt << " --" << std::endl;

		int iter_coupling=0;
		bool converged = false;
		// Strong coupling
		while ((iter_coupling < iterMax_coupling) && !converged) {
			// Extrapolate Temperature on Steak and set Pan
			Pan.setBC_Tw(Steak.bcbottom);
			// Thermal solving for the Pan
			Pan.solve_thermal.runSafe();
			// Extrapolate Temperature on Pan and set Steak
			Pan.temperature_interface.runSafe();
			Steak.setBC_qw(Pan.bcup);
			// Thermal solving for the Steak
			Steak.solve_thermal.runSafe();
			Steak.temperature_interface.runSafe();
			converged = Pan.compareTw(Steak.bcbottom,PM.get(TPM::tolerance_coupling));
			iter_coupling++;
		}
		if (converged)
			std::cout << " >>> Strong coupling converged in " << iter_coupling << " iterations" << std::endl;
		time+=dt;

		if ((PM.get(TPM::dumpVisit)) && (iter%(PM.get(TPM::dumpFrequency))==0)) {
			DumpToVisit("Pan",iter, time, Pan.Xsize, Pan.Ysize, Pan.quads, Pan.points);
			DumpToVisit("Steak",iter, time, Pan.Xsize, Pan.Ysize, Steak.quads, Steak.points);
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
		DumpToVisit("Pan",iter, time, Pan.Xsize, Pan.Ysize, Pan.quads, Pan.points);
		DumpToVisit("Steak",iter, time, Pan.Xsize, Pan.Ysize, Steak.quads, Steak.points);
	}
}
