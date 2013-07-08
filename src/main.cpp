#include <boost/program_options.hpp>

// Wt
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WServer>

#include "hc_wgt.hpp"
#include "hc_master.hpp"

using namespace ::std;
using namespace ::Wt;

namespace po = ::boost::program_options;

class HCApplication: public WApplication
{
public:
    HCApplication(const WEnvironment &environment, HCMaster &hc_master):
        WApplication(environment) {
        setTitle("HC-001");

        setCssTheme("polished");
        messageResourceBundle().use(appRoot() + "res");

        root()->setPadding(10);
        root()->resize(WLength::Auto, WLength::Auto);

        new HCWidget(hc_master, root());

        useStyleSheet("hc.css");
    }
};

class HCAppFactory
{
public:
    explicit HCAppFactory(HCMaster &hmaster): hmaster_(hmaster) {}

    WApplication *operator()(const WEnvironment &env) const {
        return new HCApplication(env, hmaster_);
    }
private:
    HCMaster &hmaster_;
};

int main(int argc, char **argv)
{
	size_t baud_rate;
	string port_name;
	
	po::options_description opts ("HC001 web-daemon options");
	opts.add_options()
		("baud_rate", po::value<size_t> (&baud_rate)->default_value(9600), "COM port baud rate")
		("port_name", po::value<string> (&port_name)->default_value("/dev/ttyUSB0"), "COM port name")
	;
	po::variables_map vm;
	
	po::parsed_options parsed = po::parse_config_file<char> ("port.cfg", opts);
	po::store (parsed, vm);
	
	po::notify (vm);
		
    WServer server(argv[0]);

    HCMaster hc_master(server);
    hc_master.start(baud_rate, port_name);

    server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(Wt::Application, HCAppFactory(hc_master));

    if (server.start()) {
        int sig = WServer::waitForShutdown();
        std::cerr << "Shutting down: (signal = " << sig << ")" << std::endl;
        server.stop();
    }

    return EXIT_FAILURE;
}
