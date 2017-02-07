



/*! \file assert.cpp
        \brief Implements a better 'Assert'
 */


#include <iostream>
#include <stdlib.h>

namespace tpp {

/*! \def MyAssertFunction
    \brief Function used by 'Assert' function in _DEBUG mode.
   
    Details.
*/
bool MyAssertFunction( bool b, const char* desc, int line, const char* file){
	if (b) return true;
	std::cerr << "\n\nAssertion Failure\n";
	std::cerr << "Description : " << desc << std::endl;
	std::cerr << "Filename    : " << file << std::endl;
	std::cerr << "Line No     : " << line << std::endl;
	exit(1);
}


} // end of namespace
