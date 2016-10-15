#include "ThermalParameterManager.h"

#include<fstream>
#include "../thermal/ParameterManagerMacros.h"

#define PM_CLASSNAME ThermalParameterManager

// Defining default values
PM_DEF ( int, iterMax, 100);
PM_DEF ( int, iterMax_coupling, 30);
PM_DEF ( int, dumpFrequency, 10);
PM_DEF ( double, timeMax, 10.0);
PM_DEF ( double, cfl, 1.0e3);
PM_DEF ( double, tolerance_coupling, 1.0e-5);
PM_DEFC ( double, K, 100.0);
PM_DEF ( double, T_init, 500.0);
PM_DEF ( double, rho, 100.0);
PM_DEF ( double, cv, 100.0);
PM_DEF ( double, qwl, 0.0);
PM_DEF ( double, qwr, 0.0);
PM_DEF ( double, qwu, 0.0);
PM_DEF ( double, qwb, 5000.0);
PM_DEF ( bool, dumpVisit, false);
PM_DEF ( std::string, CGNSFileName, "../rectangular.cgns");
PM_DEF ( std::string, SimitFileName, "../thermal.sim");
PM_DEF ( std::string, PanFileName, "../pan.therm");

void ThermalParameterManager::initialize(void)
{
	// Initialize with default values
	PM_INIT(iterMax);
	PM_INIT(iterMax_coupling);
	PM_INIT(dumpFrequency);
	PM_INIT(timeMax);
	PM_INIT(cfl);
	PM_INIT(tolerance_coupling);
	PM_INIT(K);
	PM_INIT(T_init);
	PM_INIT(rho);
	PM_INIT(cv);
	PM_INIT(qwl);
	PM_INIT(qwr);
	PM_INIT(qwu);
	PM_INIT(qwb);
	PM_INIT(dumpVisit);
	PM_INIT(CGNSFileName);
	PM_INIT(SimitFileName);
	PM_INIT(PanFileName);
}

void ThermalParameterManager::updateDependentParameters(void)
{
	// Set parameters dependent of others parameters
}

// Small function to be able to make a switch on Strings
// Be careful not fully unique but enough for our case
constexpr unsigned int str2int(const char* str, int h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

bool ThermalParameterManager::readParameters(std::string paramFileName)
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
			PM_READ_BOOL("dumpVisit",paramValue)
			PM_READ_STRING("CGNSFileName",paramValue)
			PM_READ_STRING("SimitFileName",paramValue)
			PM_READ_STRING("PanFileName",paramValue)
			default :
				std::cerr << "WARNING parameter " << paramName << " is not defined in the thermic model" << std::endl;
			}
		}
		paramFile.close();
		return true;}
	else {
		std::cerr << "WARNING Can not open the file " << paramFileName << std::endl;
		return false;
	}
}

