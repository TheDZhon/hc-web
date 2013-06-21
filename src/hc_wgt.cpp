#include "hc_wgt.hpp"

#include <Wt/WString>
#include <Wt/WText>
#include <Wt/WStandardItemModel>
#include <Wt/WApplication>
#include <Wt/WSlider>
#include <Wt/WSpinBox>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPanel>
#include <Wt/WTime>
#include <Wt/WAnimation>

#include <Wt/Chart/WCartesianChart>

using namespace ::Wt;
using namespace ::Wt::Chart;

namespace
{
const size_t kQuants = 40;

enum Axes {
    kTimeAxis,
    kTemperatureAxis,
    kHumidityAxis,
    kSpeedAxis,

    kAxesCnt
};
}

HCWidget::HCWidget(HCMaster &hc_master, Wt::WContainerWidget *parent):
    WContainerWidget(parent),
    hc_master_(hc_master)
{
    WVBoxLayout * top_layout = new WVBoxLayout;
  
    WStandardItemModel *wmodel = new WStandardItemModel(kQuants, kAxesCnt, this);
    wmodel->setHeaderData(kTimeAxis, WString("Time"));
    
    wmodel->setHeaderData(kTemperatureAxis, WString("Temperature"));
    wmodel->setHeaderData(kHumidityAxis, WString("Humidity"));
    wmodel->setHeaderData(kSpeedAxis, WString ("Speed"));
    
    for (unsigned i = 0U; i < kQuants; ++i) {
        const double x = (static_cast<double>(i) - kQuants);

	WTime t (22, 54, i);	
	
        wmodel->setData(i, kTimeAxis, t);
        wmodel->setData(i, kTemperatureAxis, 27 + 0.3*sin(x));
        wmodel->setData(i, kHumidityAxis, 65 + 0.1*cos(x));
	wmodel->setData(i, kSpeedAxis, 50 + cos(x) * sin(x));
    }

    WCartesianChart *cart = new WCartesianChart;
    top_layout->addWidget(cart);
    
    cart->setModel(wmodel);
    cart->setXSeriesColumn(0);
    cart->setLegendEnabled(true);
    cart->setLegendLocation(Chart::LegendInside, Wt::Right, Wt::AlignRight);

    cart->setType(Chart::CategoryChart);

    cart->axis(XAxis).setLocation(Chart::MinimumValue);
    cart->axis(YAxis).setLocation(Chart::MinimumValue);

    //cart->setPlotAreaPadding(60, Left | Right);
    cart->setPlotAreaPadding(60, Top);

    WDataSeries tseries(kTemperatureAxis, CurveSeries);
    tseries.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
    cart->addSeries(tseries);

    WDataSeries hseries(kHumidityAxis, CurveSeries);
    hseries.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
    cart->addSeries(hseries);
    
    WDataSeries sseries(kSpeedAxis, CurveSeries);
    sseries.setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
    cart->addSeries(sseries);

    cart->resize(800, 200); // WPaintedWidget must be given explicit size

    cart->setMargin(10, Top | Bottom);             // add margin vertically
    cart->setMargin(WLength::Auto, Left | Right);  // center horizontally
  
    WPanel * control_panel = new WPanel; 
    top_layout->addWidget(control_panel);
    
    WHBoxLayout * control_layout = new WHBoxLayout;

    control_panel->setTitle ("Humidifier control");
    
    WText * slider_txt = new WText (this);
    control_layout->addWidget(slider_txt);
    slider_txt->setText("Set FAN speed: ");
    slider_txt->setMargin (15, Bottom | Top);
    
    WSlider * slider = new WSlider (Wt::Horizontal, this);
    control_layout->addWidget (slider, 1);
    slider->setMinimum(0); slider->setMaximum(100);
    slider->setValue (0);
    slider->setTickInterval(10);
    slider->setTickPosition(WSlider::TicksBelow);
    slider->valueChanged().connect (&hc_master_, &HCMaster::changeSpeed);
    
    WContainerWidget * control = new WContainerWidget;
    control->setLayout(control_layout);  
    control_panel->setCentralWidget (control);
    this->setLayout(top_layout);
    
    hc_master_.reg(this);
    WApplication::instance()->enableUpdates(true);
}

HCWidget::~HCWidget()
{
    hc_master_.unreg(this);
    WApplication::instance()->enableUpdates(false);
}

void HCWidget::displayData(const hc_data_t &d)
{
   
}

void HCWidget::displayError(const std::string &err)
{
    
}

void HCWidget::makeVisibleToUser (Func0 f)
{
    f ();
    WApplication::instance()->triggerUpdate();
}