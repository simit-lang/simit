#include "graph.h"
#include "program.h"
#include "mesh.h"
#include <cmath>
#include <time.h>
#include <sys/time.h>

#include "Thermal.h"

#include "../thermal/ParameterManager.h"
#include "../thermal/ParameterManagerMacros.h"
#include "MGParameterManager.h"

using namespace simit;

#define TPM MGParameterManager

// Thermal-viz
void DumpToVisit(std::string ZoneName,int iter, double tim,
                 int Xsize, int Ysize, Set *quadsPan, Set *pointsPan);

int main(int argc, char **argv)
{

  //1- Analyse command line arguments
  if (argc != 2) {
    std::cerr << "Usage: mg <path to .therm file>" << std::endl;
    return -1;
  }
  std::string thermfile = argv[1];

  //2- Construct the parameter manager
  TPM PM;
  PM.readParameters(thermfile);

  //3 - Read the Pan part
  Thermal Pan = Thermal(PM.get(TPM::PanFileName),PM.get(TPM::CGNSFileName_0),
                        PM.get(TPM::CGNSFileName_1),"Pan",1);


  // Compute first time step
  double time=0.0;
  double dt=0.0;
  Pan.compute_dt.runSafe();
  dt=Pan.dt(0);

  // Interface boundary condition : Dirichlet for the Pan
  Pan.bc_types(Pan.coupling_direction(0))=1;
  //	Pan_L0.bc_types(Pan_L0.coupling_direction(0))=1;

  int iter=0;
  if (PM.get(TPM::dumpVisit)) {
    DumpToVisit("Pan_L1",iter, time, Pan.Xsize[1], Pan.Ysize[1],
                Pan.quads_MG[1], Pan.points_MG[1]);
  }
  // Time loop
  timeval start;
  gettimeofday(&start, NULL) ;
  while ((time < PM.get(TPM::timeMax)) && (iter<PM.get(TPM::iterMax))) {
    iter=iter+1;
    std::cout << "---- Iteration " << iter << " ----" << std::endl;
    std::cout << "  -- Time " << time << " -- dt " << dt << " --" << std::endl;

    Pan.solve_thermal.runSafe();

    time+=dt;

    if ((PM.get(TPM::dumpVisit)) && (iter%(PM.get(TPM::dumpFrequency))==0)) {
      DumpToVisit("Pan_L1",iter, time, Pan.Xsize[1], Pan.Ysize[1],
                  Pan.quads_MG[1], Pan.points_MG[1]);
    }
    // Compute the next timestep value
    Pan.compute_dt.runSafe();
    dt=Pan.dt(0);
    // To arrive just on time
    if (time+dt > PM.get(TPM::timeMax))
      dt = PM.get(TPM::timeMax) - time;
    Pan.dt(0)=dt;
  }
  timeval end;
  gettimeofday(&end, NULL) ;
  double elapsed_time = (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec))/1000000 ;
  elapsed_time = elapsed_time*1000;
  printf("\nElapsed time         = %10.2f (ms)\n", elapsed_time);
  if (PM.get(TPM::dumpVisit)) {
    DumpToVisit("Pan_L1",iter, time, Pan.Xsize[1], Pan.Ysize[1],
                Pan.quads_MG[1], Pan.points_MG[1]);
  }
}
