#include "hc_master.hpp"

#include <Wt/WApplication>
#include <Wt/WServer>

#include "hc_wgt.hpp"

using namespace ::Wt;

namespace
{
typedef std::lock_guard<std::recursive_mutex> MutexGuard;
}

HCMaster::HCMaster(Wt::WServer &serv):
    serv_(serv),
    hc_cntl_(),
    sess_(),
    mut_()
{

}

void HCMaster::start()
{
    hc_cntl_.start(
    [this](const hc_data_t & d) {
        handleData(d);
    },
    [this](const std::string & s) {
        handleError(s);
    });
}

void HCMaster::reg(HCWidget *wgt)
{
    MutexGuard _(mut_);

    sess_[wgt] = WApplication::instance()->sessionId();
}

void HCMaster::unreg(HCWidget *wgt)
{
    MutexGuard _(mut_);

    sess_.erase (wgt);
}

void HCMaster::handleData(const hc_data_t &d)
{
    handleImpl([ = ](HCWidget * w) {
        w->displayData(d);
    });
}

void HCMaster::handleError(const std::string &err)
{
    handleImpl([ = ](HCWidget * w) {
        w->displayError(err);
    });
}

void HCMaster::handleImpl(Func1 f1)
{
    MutexGuard _(mut_);

    WApplication *app = WApplication::instance();

    for (auto w: sess_) {
        if (app && app->sessionId() == w.second) {
            f1(w.first);
        } else {
            serv_.post(w.second, [ = ]() {
                f1(w.first);
            });
        }
    }
}
