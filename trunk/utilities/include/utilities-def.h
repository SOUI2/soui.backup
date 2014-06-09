#ifdef UTILITIES_EXPORTS
#define UTILITIES_API __declspec(dllexport)
#else
#define UTILITIES_API __declspec(dllimport)
#endif

#include <assert.h>

#define DUIASSERT(x) assert(x)
#define DUIASSERT_NE(a,b) DUIASSERT(a!=b)
