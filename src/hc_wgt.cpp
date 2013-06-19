#include "hc_wgt.hpp"

#include <Wt/WString>
#include <Wt/WText>
#include <Wt/WStandardItemModel>

#include <Wt/Chart/WCartesianChart>

using namespace ::Wt;
using namespace ::Wt::Chart;

namespace {
	const size_t kQuants = 40;
	
	enum Axes {
		kTimeAxis,
		kTemperatureAxis,
		kHumidityAxis,
		
		kAxesCnt
	};
}

HCWidget::HCWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
	WStandardItemModel * wmodel = new WStandardItemModel (kQuants, kAxesCnt, this);
	wmodel->setHeaderData (kTimeAxis, WString ("Time"));
	wmodel->setHeaderData (kTemperatureAxis, WString ("Temperature"));
	wmodel->setHeaderData (kHumidityAxis, WString ("Humidity"));
	
	for (unsigned i = 0U; i < kQuants; ++i) 
	{
		double x = (static_cast<double>(i) - kQuants) / 4;

		wmodel->setData (i, kTimeAxis, x);
		wmodel->setData (i, kTemperatureAxis, sin(x));
		wmodel->setData (i, kHumidityAxis, cos(x));
	}
	
	WCartesianChart * cart = new WCartesianChart (this);
	cart->setModel (wmodel);
	cart->setXSeriesColumn (0);
	cart->setLegendEnabled (true);

	cart->setType(Chart::ScatterPlot);
	
	cart->axis(XAxis).setLocation (Chart::MinimumValue);
	cart->axis(YAxis).setLocation (Chart::MinimumValue);
	
	cart->setPlotAreaPadding (80, Left);
	cart->setPlotAreaPadding (40, Top | Bottom);
	
	WDataSeries tseries (1, CurveSeries);
	tseries.setShadow (WShadow(3, 3, WColor(0, 0, 0, 127), 3));
	cart->addSeries (tseries);
	
	WDataSeries hseries (2, CurveSeries);
	hseries.setShadow (WShadow(3, 3, WColor(0, 0, 0, 127), 3));
	cart->addSeries (hseries);

	cart->resize(1000, 300); // WPaintedWidget must be given explicit size

	cart->setMargin (10, Top | Bottom);            // add margin vertically
	cart->setMargin (WLength::Auto, Left | Right); // center horizontally
}

HCWidget::~HCWidget()
{

}


