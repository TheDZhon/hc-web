#ifndef HC_MASTER__H_
#define HC_MASTER__H_

#include <functional>
#include <unordered_map>
#include <mutex>

#include <boost/circular_buffer.hpp>

#include "hc_cntl.hpp"

class HCWidget;
namespace Wt
{
	class WServer;
}

class HCMaster
{
public:
	explicit HCMaster (Wt::WServer& serv);

	void start (size_t baud_rate, const std::string& port_name);

	void reg (HCWidget*);
	void unreg (HCWidget*);
public /*slots*/:
	void changeSpeed (int new_speed);
	void changeHeat (int new_heat);
	void refreshWater ();
private:
	typedef std::function <void (HCWidget* f) > Func1;
	typedef std::unordered_map<HCWidget*, std::string> Sessions;
	typedef boost::circular_buffer<std::string> ErrorsBuf;

	void handleData (const hc_data_t&);
	void handleError (const std::string&);

	void handleImpl (Func1);

	Wt::WServer& serv_;

	HCController hc_cntl_;
	Sessions sess_;

	ErrorsBuf error_buf_;

	mutable std::recursive_mutex mut_;
};

#endif // HC_MASTER__H_
