#ifndef HC_WGT_H__
#define HC_WGT_H__

#include <Wt/WContainerWidget>

class HCWidget: public Wt::WContainerWidget
{
public:
    HCWidget(WContainerWidget *parent = 0);
    virtual ~HCWidget();
};

#endif // HC_WGT_H__
