// Wt
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WServer>

#include "hc_wgt.hpp"
#include "hc_master.hpp"

using namespace ::std;
using namespace ::Wt;

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

int main(int argc, char **argv)
{
    WServer server(argv[0]);
    HCMaster hc_master(server);

    server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(Wt::Application, [&hc_master](const WEnvironment & env) {
        return new HCApplication(env, hc_master);
    });
    
    if (server.start()) {
	auto sig = WServer::waitForShutdown ();
	std::cerr << "Shutting down: (signal = " << sig << ")" << std::endl;
	server.stop ();
    }
    
    return EXIT_FAILURE;
}
