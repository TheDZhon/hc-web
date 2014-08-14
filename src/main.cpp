#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <string>

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WServer>

#include "hc_wgt.hpp"
#include "hc_master.hpp"

using namespace ::std;
using namespace ::Wt;

namespace
{
	const regex kKeyValRegex ("^[\\s]*([:alpha:][\\w]*)[\\s]*=[\\s]*([\\w/]+)[\\s]*$");

	enum kRegExIndexes : int {
		kWhole = 0,
		kKey = 1,
		kVal = 2,
		kCnt = kVal + 1
	};
}

class HCApplication: public WApplication
{
public:
	HCApplication (const WEnvironment& environment, HCMaster& hc_master) :
		WApplication (environment)
	{
		setTitle ("HC-001");

		setCssTheme ("polished");
		messageResourceBundle().use (appRoot() + "res");

		root()->setPadding (10);
		root()->resize (WLength::Auto, WLength::Auto);

		new HCWidget (hc_master, root());

		useStyleSheet ("hc.css");
	}
};

class HCAppFactory
{
public:
	explicit HCAppFactory (HCMaster& hmaster) : hmaster_ (hmaster) {}

	WApplication* operator() (const WEnvironment& env) const
	{
		return new HCApplication (env, hmaster_);
	}
private:
	HCMaster& hmaster_;
};

typedef unordered_map<std::string, std::string> ParamsMap;
template<typename T> T readOrDefault (const ParamsMap& p, const string& key, T def_val)
{
	if (p.count (key) < 1) { return def_val; }

	T ret;
	const string& val = p.find (key)->second;
	istringstream istr (val);
	istr >> ret;

	return ret;
}

int main (int argc, char** argv)
{
	ParamsMap params;

	{
		fstream opts_fh ("port.cfg", ios::in | ios::binary);

		for (string line; getline (opts_fh, line);) {
			smatch matched;
			if (regex_match (line, matched, kKeyValRegex) && (matched.size () == kCnt)) {
				params[matched[kKey]] = matched[kVal];
			}
		}
	}

	WServer server (argv[0]);

	HCMaster hc_master (server);

	const auto baud_rate = readOrDefault<size_t> (params, "baud_rate", 9600U);
	const auto port_name = readOrDefault<string> (params, "port_name", "/dev/ttyUSB0");

	clog << "baud_rate = " << baud_rate << " bps" << endl;
	clog << "port_name = " << port_name << endl;

	hc_master.start (baud_rate, port_name);

	server.setServerConfiguration (argc, argv, WTHTTP_CONFIGURATION);

	server.addEntryPoint (Wt::Application, HCAppFactory (hc_master));

	if (server.start()) {
		int sig = WServer::waitForShutdown();
		std::cerr << "Shutting down: (signal = " << sig << ")" << std::endl;
		server.stop();
	}

	return EXIT_FAILURE;
}
