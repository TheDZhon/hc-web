#include <cgicc/Cgicc.h>

#include <stdexcept>

using namespace ::cgicc;
using namespace ::std;

int main (int argc, char ** argv)
{
	try {
		Cgicc cgi_helper;
	} 
	catch (const exception & e) {
		// Try log here
	}
	catch (...) {
		// Try log here too
	}

	return 0;
}
