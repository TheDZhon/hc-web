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
					[this] (const hc_data_t& hcdata) { handleData (hcdata); },
					[this] (const string& err_s) { handleError(err_s); });

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
	handleImpl ([d] (HCWidget * hcw) { hcw->displayData (d); });
}

void HCMaster::handleError (const std::string& err)
{
	handleImpl ([err] (HCWidget * hcw) { hcw->displayError(err); });

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
			serv_.post (it->second, [=] () { f1 (it->first); });
		}
	}
}
