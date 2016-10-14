// Set of MACROS to generate template class


/// define a non-constant, independent parameter with default value.
#define PM_DEF(TYPE, NAME, VALUE)                             \
		const Parameter<TYPE> PM_CLASSNAME::NAME(#NAME, VALUE)
/// define a constant parameter
#define PM_DEFC(TYPE, NAME, VALUE)                            \
		const Parameter<TYPE> PM_CLASSNAME::NAME(#NAME, VALUE, true)
/// define a dependent parameter
#define PM_DEFD(TYPE, NAME, VALUE, C)                         \
		const Parameter<TYPE> PM_CLASSNAME::NAME(#NAME, VALUE, C, true)
/// initialize a parameter to its default value.
#define PM_INIT(P) \
		set(P, P.defaultVal())
#define PMM_GET(N)                                     \
		const T##N get(const Parameter<T##N>& p) const {     \
	return ParameterManager<T##N>::get(p); }
#define PMM_SET(N)                                     \
		const T##N set(const Parameter<T##N>& p, const T##N& v) {  \
	return ParameterManager<T##N>::set(p,v); }
#define PM_READ_DOUBLE(NAME,VALUE) \
		case str2int(NAME) : \
		set(Parameter<double>(NAME),atof(VALUE.c_str())); \
		break;
#define PM_READ_INT(NAME,VALUE) \
		case str2int(NAME) : \
		set(Parameter<int>(NAME),atoi(VALUE.c_str())); \
		break;
#define PM_READ_BOOL(NAME,VALUE) \
		case str2int(NAME) : \
		set(Parameter<bool>(NAME),atoi(VALUE.c_str())); \
		break;
#define PM_READ_STRING(NAME,VALUE) \
		case str2int(NAME) : \
		set(Parameter<std::string>(NAME),VALUE); \
		break;

