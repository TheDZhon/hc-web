#include "hc_master.hpp"

#include <Wt/WApplication>
#include <Wt/WServer>

#include "hc_wgt.hpp"

using namespace ::Wt;
using namespace ::boost;

namespace
{
typedef boost::lock_guard<boost::recursive_mutex> MutexGuard;
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
    hc_cntl_.start(boost::bind(&HCMaster::handleData, this, _1), boost::bind(&HCMaster::handleError, this, _1));
}

void HCMaster::reg(HCWidget *wgt)
{
    MutexGuard _(mut_);

    sess_[wgt] = WApplication::instance()->sessionId();
}

void HCMaster::unreg(HCWidget *wgt)
{
    MutexGuard _(mut_);

    sess_.erase(wgt);
}

void HCMaster::changeSpeed(int new_speed)
{
    hc_cntl_.setSpeed(new_speed);
}

void HCMaster::handleData(const hc_data_t &d)
{
    handleImpl(boost::bind(&HCWidget::displayData, _1, d));
}

void HCMaster::handleError(const std::string &err)
{
    handleImpl(boost::bind(&HCWidget::displayError, _1, err));

    std::cerr << err << std::endl;
}

void HCMaster::handleImpl(Func1 f1)
{
    MutexGuard _(mut_);

    WApplication *app = WApplication::instance();

    for (Sessions::iterator it = sess_.begin();
            it != sess_.end();
            ++it) {
        if (app && app->sessionId() == it->second) {
            f1(it->first);
        } else {
            serv_.post(it->second, boost::bind(f1, it->first));
        }
    }
}
