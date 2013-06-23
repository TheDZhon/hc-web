#ifndef HC_WGT_H__
#define HC_WGT_H__

#include <functional>

#include <Wt/WContainerWidget>

#include "hc_master.hpp"

class HCWidget: public Wt::WContainerWidget
{
public:
    explicit HCWidget (HCMaster & hc_master, WContainerWidget *parent = 0);
    virtual ~HCWidget();
    
    void displayData (const hc_data_t & d);
    void displayError (const std::string & err);
private:
    typedef boost::function<void (void)> Func0;
  
    void makeVisibleToUser (Func0 f);
  
    HCMaster & hc_master_;    
};

#endif // HC_WGT_H__
