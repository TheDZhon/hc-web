#ifndef HC_MASTER__H_
#define HC_MASTER__H_

#include <unordered_set>

#include "hc_cntl.hpp"

class HCMaster {
public:
	HCMaster ();
	
	void start ();
	
	void reg ();
	void unreg ();
private:
	typedef std::unordered_set<std::string> Sessions;
	
	void handleData (const hc_data_t&);
	void handleError (const std::string&);
	
	HCController hc_cntl_;
	Sessions sess_;
};

#endif // HC_MASTER__H_