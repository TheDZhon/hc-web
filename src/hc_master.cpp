#include "hc_master.hpp"

#include <Wt/WApplication>
#include <Wt/WServer>

#include "hc_wgt.hpp"

using namespace ::Wt;
using namespace ::std;

namespace
{
	typedef lock_guard<recursive_mutex> MutexGuard;

	const size_t kMaxErrosBuf = 100;
}

HCMaster::HCMaster (Wt::WServer& serv) :
	serv_ (serv),
	hc_cntl_(),
	sess_(),
	error_buf_ (kMaxErrosBuf),
	mut_()
{

}

void HCMaster::start (size_t baud_rate, const string& port_name)
{

	hc_cntl_.start (baud_rate,
					port_name,
					boost::bind (&HCMaster::handleData, this, _1),
					boost::bind (&HCMaster::handleError, this, _1));

}

void HCMaster::reg (HCWidget* wgt)
{
	MutexGuard _ (mut_);

	sess_[wgt] = WApplication::instance()->sessionId();

	for (ErrorsBuf::iterator it = error_buf_.begin();
		 it != error_buf_.end();
		 ++it) {
		wgt->displayError (*it);
	}

	error_buf_.clear();
}

void HCMaster::unreg (HCWidget* wgt)
{
	MutexGuard _ (mut_);

	sess_.erase (wgt);
}

void HCMaster::changeSpeed (int new_speed)
{
	hc_cntl_.setSpeed (new_speed);
}

void HCMaster::handleData (const hc_data_t& d)
{
	handleImpl (boost::bind (&HCWidget::displayData, _1, d));
}

void HCMaster::handleError (const std::string& err)
{
	handleImpl (boost::bind (&HCWidget::displayError, _1, err));

	MutexGuard _ (mut_);
	error_buf_.push_back (err);
}

void HCMaster::handleImpl (Func1 f1)
{
	MutexGuard _ (mut_);

	WApplication* app = WApplication::instance();

	for (Sessions::iterator it = sess_.begin();
		 it != sess_.end();
		 ++it) {
		if (app && app->sessionId() == it->second) {
			f1 (it->first);
		} else {
			serv_.post (it->second, boost::bind (f1, it->first));
		}
	}
}
