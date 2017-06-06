#include "MGParameterManager.h"

#include<fstream>
#include "../thermal/ParameterManagerMacros.h"

#define PM_CLASSNAME MGParameterManager

// Defining default values
PM_DEF ( int, smoother_iter, 10);
PM_DEF ( int, solver_assembly, 1);
PM_DEF ( std::string, CGNSFileName_0, "../rectangular.cgns");
PM_DEF ( std::string, CGNSFileName_1, "../rectangular.cgns");

void MGParameterManager::initializeMG(void)
{
  // Initialize with default values
  PM_INIT(smoother_iter);
  PM_INIT(solver_assembly);
  PM_INIT(CGNSFileName_0);
  PM_INIT(CGNSFileName_1);
}

// Small function to be able to make a switch on Strings
// Be careful not fully unique but enough for our case
constexpr unsigned int str2int(const char* str, int h = 0)
{
  return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

bool MGParameterManager::readParameters(std::string paramFileName)
{
  // Reading parameters set by the user in a file
  // Syntax : VarName VarValue
  std::ifstream paramFile(paramFileName);
  if (paramFile.is_open())	{
    std::string paramName, paramValue;
    while (paramFile >> paramName >> paramValue)
    {
      switch (str2int(paramName.c_str())) {
        PM_READ_INT("iterMax",paramValue)
        PM_READ_INT("iterMax_coupling",paramValue)
        PM_READ_INT("dumpFrequency",paramValue)
        PM_READ_INT("coupling_direction",paramValue)
        PM_READ_INT("solver_type",paramValue)
        PM_READ_INT("solver_itermax",paramValue)
        PM_READ_INT("smoother_iter",paramValue)
        PM_READ_INT("solver_assembly",paramValue)
        PM_READ_DOUBLE("timeMax",paramValue)
        PM_READ_DOUBLE("cfl",paramValue)
        PM_READ_DOUBLE("tolerance_coupling",paramValue)
        PM_READ_DOUBLE("K",paramValue)
        PM_READ_DOUBLE("T_init",paramValue)
        PM_READ_DOUBLE("rho",paramValue)
        PM_READ_DOUBLE("cv",paramValue)
        PM_READ_DOUBLE("qwl",paramValue)
        PM_READ_DOUBLE("qwr",paramValue)
        PM_READ_DOUBLE("qwu",paramValue)
        PM_READ_DOUBLE("qwb",paramValue)
        PM_READ_DOUBLE("solver_tolerance",paramValue)
        PM_READ_BOOL("dumpVisit",paramValue)
        PM_READ_STRING("CGNSFileName_0",paramValue)
        PM_READ_STRING("CGNSFileName_1",paramValue)
        PM_READ_STRING("SimitFileName",paramValue)
        PM_READ_STRING("PanFileName",paramValue)
        PM_READ_STRING("SteakFileName",paramValue)
        default :
          std::cerr << "WARNING parameter " << paramName
                    << " is not defined in the thermic model" << std::endl;
      }
    }
    paramFile.close();
    return true;}
  else {
    std::cerr << "WARNING Can not open the file " << paramFileName << std::endl;
    return false;
  }
}


