#ifndef HC_MASTER__H_
#define HC_MASTER__H_

#include <functional>
#include <unordered_map>
#include <mutex>

#include "hc_cntl.hpp"

class HCWidget;
namespace Wt {
class WServer;  
}

class HCMaster
{
public:
    explicit HCMaster(Wt::WServer & serv);

    void start();

    void reg(HCWidget*);
    void unreg(HCWidget*);
public /*slots*/:
    void changeSpeed (int new_speed);
private:
    typedef std::function <void (HCWidget* f)> Func1;
    typedef std::unordered_map<HCWidget*, std::string> Sessions;

    void handleData(const hc_data_t &);
    void handleError(const std::string &);
    
    void handleImpl (Func1);

    Wt::WServer & serv_;
    
    HCController hc_cntl_;
    Sessions sess_;
    
    mutable std::recursive_mutex mut_;
};

#endif // HC_MASTER__H_
