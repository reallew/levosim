#ifndef _DEBUG_MACROS_
#define _DEBUG_MACROS_

//#define BUG_CHECK_ON
//#define DEBUG_MESSAGES

#ifdef DEBUG_MESSAGES
#define debug_msg(msg) (std::clog << msg << std::endl)
#else
#define debug_msg(msg) 
#endif

#ifdef BUG_CHECK_ON
#define BUG_CHECK(condition, bugtext) if (condition) {std::clog << "Bug: " << bugtext \
	<< std::endl << "(file: " << __FILE__ << ", line: " << __LINE__ << ", function: " \
	<< __FUNCTION__ << ")" << std::endl << "Program stopped." << std::endl; std::exit(3);}
#else
#define BUG_CHECK(condition, bugtext)
#endif

#endif // _DEBUG_MACROS_
