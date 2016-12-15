/* These classes are inspired by a work of Qinghai Zhang
 * C++ Design Patterns for Managing Parameters in Scientific Computing
 *
 * see http://www.math.utah.edu/~tsinghai/papers/parameterPatternsEL.pdf
*/

#ifndef APPS_THERMAL_PARAMETERMANAGER_H_
#define APPS_THERMAL_PARAMETERMANAGER_H_

#include<map>
#include<iostream>
#include "../thermal/Parameter.h"
#include "../thermal/ParameterManagerMacros.h"

template<class T>
class ParameterManager {
	// Template class for a Parameter Manager for ONE type T
public:
	typedef std::map<Parameter<T>, T>        MAP;
	virtual ~ParameterManager(){}
	const T get(const Parameter<T>& p) const
	{
		typename MAP::const_iterator mit = map_.find(p);
		if (mit==map_.end()){
			std::cerr << "WARNING: Parameter " << p.key()
				<< " not set yet! Return default value...\n";
			return p.defaultVal();
		}
		return mit->second;
	}
	const T set(const Parameter<T>& p, const T& v)
	{
		if (!p.isConstant())
			map_[p]=v;
		// initialize a constant p, check value
		else if (map_.find(p) == map_.end() && v==p.defaultVal())
			map_[p] = p.defaultVal();
		else
			std::cerr << "WARNING: " << p.key() << " = "
			<< p.defaultVal() << " is already set!\n ";
		return map_[p];
	}
	void print(std::ostream& os) const
	{
		for (typename MAP::const_iterator mit = map_.begin();
				mit != map_.end(); mit++)
			os << mit->first.key() << " = "
			<< mit->second << std::endl;
	}
protected :
	MAP map_;
};

template<class T1, class T2, class T3, class T4>
// Template class for a Parameter Manager for FOUR types T
class ParameterManager4 : private ParameterManager<T1>,
private ParameterManager<T2>,
private ParameterManager<T3>,
private ParameterManager<T4>
{
public:
	PMM_GET(1)
	PMM_GET(2)
	PMM_GET(3)
	PMM_GET(4)

	PMM_SET(1)
	PMM_SET(2)
	PMM_SET(3)
	PMM_SET(4)
friend std::ostream& operator<<
(std::ostream& os, const ParameterManager4<T1, T2, T3, T4>& pmm) {
		pmm.ParameterManager<T1>::print(os);
		pmm.ParameterManager<T2>::print(os);
		pmm.ParameterManager<T3>::print(os);
		pmm.ParameterManager<T4>::print(os);
		return os;
	}
public:
	virtual void updateDependentParameters(void) = 0;
	virtual bool readParameters(std::string inputFile) = 0;
protected:
	virtual void initialize(void)           = 0;
};

#endif /* APPS_THERMAL_PARAMETERMANAGER_H_ */
