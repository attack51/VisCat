#ifdef VISCAT_LIB_EXPORTS
#define VISCAT_LIB_API __declspec(dllexport)
#else
#define VISCAT_LIB_API __declspec(dllimport)
#endif 