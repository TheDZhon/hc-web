// Wt
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WText>

#include "hc_wgt.hpp"

using namespace ::std;
using namespace ::Wt;

class HCApplication: public WApplication
{
public:
    HCApplication(const WEnvironment &environment, WtLibVersion version = WtWT_VERSION):
        WApplication(environment, version) {
        setTitle("HC-001");

        setCssTheme("polished");
        messageResourceBundle().use(appRoot() + "res");

        root()->setPadding(10);
        root()->resize(WLength::Auto, WLength::Auto);

        root()->addWidget(new HCWidget);

        useStyleSheet("hc.css");
    }
};

WApplication *createApp(const WEnvironment &env)
{
    return new HCApplication(env);
}

int main(int argc, char **argv)
{
    return WRun(argc, argv, &createApp);
}
