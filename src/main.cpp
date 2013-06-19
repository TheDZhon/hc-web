// General purpose Linux and ANSI C headers
#include "unistd.h"
#include "termios.h"
#include "errno.h"
#include "fcntl.h"

// C++ standard headers
#include <stdexcept>
#include <iostream>
#include <string>

// GNU CGI C++ Library
#include <cgicc/Cgicc.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTMLClasses.h>

// Lightweight interface for RS232
#include "serial.h"

using namespace ::cgicc;
using namespace ::std;

namespace {
	const char kTextLength = 16;
	const char kPortName[] = "/dev/ttyUSB0";
}

class HCController {
public:
	HCController (): fd_ (-1) {
		fd_ = open (kPortName, O_RDWR | O_NOCTTY | O_SYNC);
		if (fd_ < 0) { throw runtime_error ("Can't open port"); }

		set_interface_attribs (fd_, B9600, 0);
		set_blocking (fd_, 1);

		read ();
	}

	void read () {
		sprintf (kTBuf_, "DATA0");
		sprintf (kHBuf_, "DATA1");
	}
	void setSpeed (int level) { write (fd_, "spd:|", 5); }
	
	const char * temperature () const { return kTBuf_; }
	const char * humidity () const { return kHBuf_; }	

	~HCController () {
		close (fd_);
	}
private:
	int fd_;

	char kTBuf_[kTextLength];
	char kHBuf_[kTextLength];
};

int main (int argc, char ** argv)
{
	try {
		Cgicc cgi_helper;
		HCController hc;

		cout << HTTPHTMLHeader () << endl;
		cout << html () << head (title ("HC001 Control")) << endl;
		cout << body () << endl;

		cout << table () << endl;
			cout << tr () << endl;
				cout << td ("Temperature: ") << endl;
				cout << td ("Humidity: ") << endl;
			cout << tr () << endl;
				cout << td (hc.temperature()) << endl;
				cout << td (hc.humidity()) << endl;
			cout << tr () << endl;
			cout << tr () << endl;
		cout << table () << endl;

		cout << body () << html ();
	} 
	catch (const exception & e) {
		std::cerr << "Exception: " << e.what () << std::endl;
	}
	catch (...) {
		std::cerr << "Unknown exception!" << std::endl;
	}

	return 0;
}
