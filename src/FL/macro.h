#pragma once

#include <string.h>
#include <assert.h>
#include "util.h"
#include "logmanager.h"

#if defined __GNUC__ || defined __llvm__
#	define FL_LICKLY(x)		__builtin_expect(!!(x),1)
#	define FL_UNLICKLY(x)	__builtin_expect(!!(x),0)
#else
#	define FL_LICKLY(x)		(x)
#	define FL_UNLICKLY(x)	(x)
#endif


#define FL_ASSERT(exp) \
	if(FL_UNLICKLY(!(exp))) {\
			FL_LOG_ERROR(FL_LOG_ROOT()) << "ASSERTION: " << #exp\
				<< "\nbacktrace\n" \
				<< FL::UT::BacktraceToString(100,2,"	");\
				assert(exp);\
			}

#define FL_ASSERT_2Arg(exp,info) \
	if(FL_UNLICKLY(!(exp))) {\
			FL_LOG_ERROR(FL_LOG_ROOT()) << "ASSERTION: " << #exp\
										<<"\n" << info\
			                            << "\nbacktrace\n" \
			                            << FL::UT::BacktraceToString(100, 2, "	");\
			assert(exp);\
			}
