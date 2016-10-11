#ifndef APPS_THERMAL_PARAMETER_H_
#define APPS_THERMAL_PARAMETER_H_
#include<string>
#include <cstring>
#include<iostream>

template<class T>
class Parameter {
// Definition of a template class of Parameters

public:
	Parameter(const std::string& key, const T& defaultValue = T(),
			  bool constant   = false, bool dependent  = false) :
				key_(key),
				defaultValue_(defaultValue),
				constant_(constant),
				dependent_(dependent){}

	bool isDependent(void) const { return dependent_; }
	bool isConstant(void) const  { return constant_;  }
	char const*const key(void) const { return key_.c_str(); }
	const T defaultVal(void) const  { return defaultValue_; }

	// Comparison operator for maps
	friend bool operator<(const Parameter<T>& p1, const Parameter<T>& p2) {
		return (strcmp(p1.key(),p2.key()) <0) ;
	}

private:
	std::string key_;
	T           defaultValue_;
	bool        constant_;
	bool        dependent_;
};

#endif /* APPS_THERMAL_PARAMETER_H_ */
