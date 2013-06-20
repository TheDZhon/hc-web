#include "hc_master.hpp"

HCMaster::HCMaster():
	hc_cntl_()
{

}

void HCMaster::start()
{
	hc_cntl_.start (
	[this] (const hc_data_t & d) {
		handleData (d);
	},
	[this] (const std::string & s) {
		handleError (s);
	});
}

void HCMaster::reg()
{

}

void HCMaster::unreg()
{

}

void HCMaster::handleData(const hc_data_t &)
{

}

void HCMaster::handleError(const std::string &)
{

}