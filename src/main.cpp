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
	const regex kKeyValRegex {"^\\s*([a-zA-Z_]\\w+)\\s*=\\s*([\\w/]+)\\s*$"};
	const string kConfigPath {"hc-web.cfg"};

	enum class RegExIndexes : size_t
	{
		kWhole = 0,
		kKey = 1,
		kVal = 2,
		kCnt = kVal + 1
	};

	inline size_t size_t_REI_cast (RegExIndexes r)
	{
		return static_cast<size_t> (r);
	}
}

class HCApplication: public WApplication
{
public:
	HCApplication (const WEnvironment& environment, HCMaster& hc_master) :
		WApplication {environment} {
		setTitle ("HC-001");

		setCssTheme ("polished");
		messageResourceBundle().use (appRoot() + "res");

		root()->setPadding (10);
		root()->resize (WLength::Auto, WLength::Auto);

		new HCWidget {hc_master, root() };

		useStyleSheet ("hc.css");
	}
};

class HCAppFactory
{
public:
	explicit HCAppFactory (HCMaster& hmaster) :
		hmaster_ {hmaster}
	{}

	WApplication* operator() (const WEnvironment& env) const
	{
		return new HCApplication {env, hmaster_};
	}
private:
	HCMaster& hmaster_;
};

using ParamsMap = unordered_map<std::string, std::string>;

struct config_reader_f {
	explicit config_reader_f (const ParamsMap& params) : params_ {params} {}

	template<typename T> T read (const string& key, T def_val = T {})
	{
		if (params_.count (key) < 1) { return def_val; }

		T ret {};
		const auto& val = params_.find (key)->second;
		istringstream istr {val};
		istr >> ret;

		return ret;
	}

	const ParamsMap& params_;
};

int main (int argc, char** argv)
{
	ParamsMap params {};

	{
		fstream opts_fh {kConfigPath, ios::in | ios::binary};
		if (!opts_fh.is_open()) { clog << "Can't open " << kConfigPath << " configuration file." << endl; }

		for (string line {}; getline (opts_fh, line);) {
			smatch matched {};
			if (regex_match (line, matched, kKeyValRegex) && (matched.size () == size_t_REI_cast (RegExIndexes::kCnt))) {
				params[matched[size_t_REI_cast (RegExIndexes::kKey)]] = matched[size_t_REI_cast (RegExIndexes::kVal)];
			}
		}
	}

	for (auto && kv : params) {
		clog << "parsed: [" << kv.first << "] -> " << kv.second << endl;
	}

	WServer server {argv[0]};
	HCMaster hc_master {server};
	config_reader_f creader {params};

	const auto& baud_rate = creader.read ("baud_rate", 9600U);
	const auto& port_name = creader.read ("port_name", string ("/dev/ttyUSB0"));
	const auto& doc_root = creader.read ("doc_root", string ("."));
	const auto& http_address = creader.read ("http_address", string ("0.0.0.0"));
	const auto& http_port = to_string (creader.read ("http_port", 80U));

	hc_master.start (baud_rate, port_name);

	const char* argv_sentinel[] = {
		argv[0],
		"--docroot",
		doc_root.data(),
		"--http-address",
		http_address.data(),
		"--http-port",
		http_port.data()
	};

	server.setServerConfiguration ( (sizeof argv_sentinel / sizeof argv_sentinel[0]), const_cast<char**> (argv_sentinel));

	server.addEntryPoint (Wt::Application, HCAppFactory {hc_master});

	if (server.start()) {
		const auto sig = WServer::waitForShutdown();
		std::cerr << "Shutting down: (signal = " << sig << ")" << std::endl;
		server.stop();

		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}
