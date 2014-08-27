#include "hc_master.hpp"

#include <Wt/WApplication>
#include <Wt/WServer>

#include "hc_wgt.hpp"

using namespace ::Wt;
using namespace ::std;

namespace
{
	using MutexGuard = lock_guard<recursive_mutex>;

	const auto kMaxErrosBuf = 100U;
}

HCMaster::HCMaster (Wt::WServer& serv) :
	serv_ {serv},
	  hc_cntl_ {},
	  sess_ {},
	  error_buf_ {kMaxErrosBuf},
mut_ {} {

}

void HCMaster::start (size_t baud_rate, const string& port_name)
{

	hc_cntl_.start (baud_rate,
					port_name,
	[this] (const hc_data_t& hcdata) { handleData (hcdata); },
	[this] (const string & err_s) { handleError (err_s); });

}

void HCMaster::reg (HCWidget* wgt)
{
	MutexGuard scoped_guard {mut_};

	sess_[wgt] = WApplication::instance()->sessionId();

	for (auto && e : error_buf_) { wgt->displayError (e); }

	error_buf_.clear();
}

void HCMaster::unreg (HCWidget* wgt)
{
	MutexGuard scoped_guard {mut_};

	sess_.erase (wgt);
}

void HCMaster::changeSpeed (int new_speed)
{
	hc_cntl_.setSpeed (new_speed);
}

void HCMaster::changeHeat (int new_heat)
{
	hc_cntl_.setHeat (new_heat);
}

void HCMaster::refreshWater()
{
	hc_cntl_.refreshWater();
}

void HCMaster::handleData (const hc_data_t& d)
{
	handleImpl ([d] (HCWidget * hcw) { hcw->displayData (d); });
}

void HCMaster::handleError (const std::string& err)
{
	handleImpl ([err] (HCWidget * hcw) { hcw->displayError (err); });

	MutexGuard scoped_guard {mut_};
	error_buf_.push_back (err);
}

void HCMaster::handleImpl (Func1 f1)
{
	MutexGuard scoped_guard {mut_};

	auto&& app = WApplication::instance();

	for (auto && s : sess_) {
		if (app && app->sessionId() == s.second) {
			f1 (s.first);
		} else {
			serv_.post (s.second, [ = ] () { f1 (s.first); });
		}
	}
}
