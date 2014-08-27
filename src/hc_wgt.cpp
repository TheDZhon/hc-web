#include "hc_wgt.hpp"

#include <cmath>

#include <Wt/WString>
#include <Wt/WText>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WApplication>
#include <Wt/WSlider>
#include <Wt/WSpinBox>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPanel>
#include <Wt/WTime>
#include <Wt/WAnimation>
#include <Wt/WTextArea>
#include <Wt/WPushButton>
#include <Wt/Chart/WCartesianChart>

using namespace ::Wt;
using namespace ::Wt::Chart;

using namespace ::std;

namespace
{
	const auto kQuants = 30U;
	const auto kMaxLogRows = 100U;

	enum class Axes
	{
		kTimeAxis,
		kTemperatureAxis,
		kHumidityAxis,
		kSpeedAxis,
		kHeatAxis,

		kAxesCnt
	};

	inline int int_Axes_cast (Axes a)
	{
		return static_cast<int> (a);
	}

	WString hc_data_to_wtstring (const hc_data_t& data)
	{
		const auto h_integral = static_cast<int> (data.humidity);
		const auto t_integral = static_cast<int> (data.temperature);

#pragma STDC FENV_ACCESS ON
		const auto old_round_mode = fegetround();
		fesetround (FE_TONEAREST);

		const auto h_partial = rint ( (data.humidity - h_integral) * 10);
		const auto t_partial = rint ( (data.temperature - t_integral) * 10);

		return WString ("Humidity: {1}.{2}%; Temperature: {3}.{4} C; Fan Speed: {5}%; Heat Power: {6}%")
			   .arg (h_integral).arg (h_partial)
			   .arg (t_integral).arg (t_partial)
			   .arg (data.speed).arg (data.heat);

		fesetround (old_round_mode);
	}

}

HCWidget::HCWidget (HCMaster& hc_master, Wt::WContainerWidget* parent) :
	WContainerWidget {parent},
				 hc_master_ {hc_master},
				 speed_feedback_lineedit_ {nullptr},
				 graph_data_model_ {nullptr},
				 log_textarea_ {nullptr},
				 last_ind_ {0},
last_log_ind_ {0} {
	auto&&  top_level_layout = new WVBoxLayout {};

	graph_data_model_ = new WStandardItemModel {kQuants, int_Axes_cast (Axes::kAxesCnt), this};

	graph_data_model_->setHeaderData (int_Axes_cast (Axes::kTimeAxis), WString {"Time"});
	graph_data_model_->setHeaderData (int_Axes_cast (Axes::kTemperatureAxis), WString {"Temperature"});
	graph_data_model_->setHeaderData (int_Axes_cast (Axes::kHumidityAxis), WString {"Humidity"});
	graph_data_model_->setHeaderData (int_Axes_cast (Axes::kSpeedAxis), WString {"Speed"});
	graph_data_model_->setHeaderData (int_Axes_cast (Axes::kHeatAxis), WString {"Heat"});

	auto&&  plot = new WCartesianChart;
	top_level_layout->addWidget (plot);

	plot->setModel (graph_data_model_);
	plot->setXSeriesColumn (0);
	plot->setLegendEnabled (true);
	plot->setLegendLocation (Chart::LegendInside, Wt::Right, Wt::AlignRight);

	plot->setType (Chart::CategoryChart);

	plot->axis (XAxis).setLocation (Chart::MinimumValue);
	plot->axis (YAxis).setLocation (Chart::MinimumValue);

	plot->setPlotAreaPadding (60, Top);

	WDataSeries temperature_series {int_Axes_cast (Axes::kTemperatureAxis), CurveSeries};
	temperature_series.setShadow (WShadow {3, 3, WColor (0, 0, 0, 127), 3});
	plot->addSeries (temperature_series);

	WDataSeries humidity_series {int_Axes_cast (Axes::kHumidityAxis), CurveSeries};
	humidity_series.setShadow (WShadow {3, 3, WColor (0, 0, 0, 127), 3});
	plot->addSeries (humidity_series);

	WDataSeries speed_series {int_Axes_cast (Axes::kSpeedAxis), CurveSeries};
	speed_series.setShadow (WShadow {3, 3, WColor (0, 0, 0, 127), 3});
	plot->addSeries (speed_series);

	WDataSeries heat_series {int_Axes_cast (Axes::kHeatAxis), CurveSeries};
	speed_series.setShadow (WShadow {3, 3, WColor (0, 0, 0, 127), 3});
	plot->addSeries (heat_series);

	plot->resize (1200, 600); // WPaintedWidget must be given explicit size

	plot->setMargin (10, Top | Bottom);            // add margin vertically
	plot->setMargin (WLength::Auto, Left | Right); // center horizontally

	auto&&  control_panel = new WPanel {};
	top_level_layout->addWidget (control_panel);

	auto&&  control_layout = new WGridLayout {};

	control_panel->setTitle ("Humidifier control");

	const auto add_label = [ = ] (const char* txt, int row)
	{
		auto label = new WText {this};
		control_layout->addWidget (label, row, 0);
		label->setText (txt);
	};

	add_label ("Set FAN speed: ", 0);
	add_label ("Set HEAT power: ", 1);

	const auto add_slider = [ = ] (void (HCMaster::*p) (int), int row)
	{
		auto slider = new WSlider {Wt::Horizontal, this};
		control_layout->addWidget (slider, row, 1);
		slider->setMinimum (0);
		slider->setMaximum (100);
		slider->setValue (0);
		slider->setTickInterval (10);
		slider->setTickPosition (WSlider::TicksBelow);
		slider->valueChanged().connect (&hc_master_, p);
	};

	add_slider (&HCMaster::changeSpeed, 0);
	add_slider (&HCMaster::changeHeat, 1);

	const auto add_feedback = [ = ] (WLineEdit** l, int row)
	{
		auto& lp = *l;

		lp = new WLineEdit {};
		lp->setReadOnly (true);
		lp->setAttributeValue ("style", "text-align: center;");
		control_layout->addWidget (lp, row, 2);
	};

	add_feedback (&speed_feedback_lineedit_, 0);
	add_feedback (&heat_feedback_lineedit_, 1);

	auto refrWater = new WPushButton {"Refresh water"};
	control_layout->addWidget (refrWater, 2, 0);
	refrWater->clicked().connect ([&] (Wt::WMouseEvent&) { hc_master_.refreshWater (); });

	auto control_container = new WContainerWidget {};
	control_container->setLayout (control_layout);
	control_panel->setCentralWidget (control_container);

	auto log_panel = new WPanel {};
	auto log_container = new WContainerWidget {};
	auto log_layout = new WHBoxLayout {};

	log_panel->setCollapsible (true);
	log_panel->setCollapsed (true);
	log_panel->setTitle ("Log");
	log_textarea_ = new WTextArea;
	log_layout->addWidget (log_textarea_);
	log_container->setLayout (log_layout);
	log_panel->setCentralWidget (log_container);

	top_level_layout->addWidget (log_panel);

	setLayout (top_level_layout);

	hc_master_.reg (this);
	WApplication::instance()->enableUpdates (true);
}

HCWidget::~HCWidget()
{
	hc_master_.unreg (this);
	WApplication::instance()->enableUpdates (false);
}

void HCWidget::displayData (const hc_data_t& d)
{
	speed_feedback_lineedit_->setText (to_string (d.speed));
	heat_feedback_lineedit_->setText (to_string (d.heat));

	const auto taxis = int_Axes_cast (Axes::kTimeAxis);
	const auto tempaxis = int_Axes_cast (Axes::kTemperatureAxis);
	const auto huaxis = int_Axes_cast (Axes::kHumidityAxis);
	const auto saxis = int_Axes_cast (Axes::kSpeedAxis);
	const auto heataxis = int_Axes_cast (Axes::kHeatAxis);

	if (last_ind_ != (kQuants - 1)) {
		last_ind_ = ++last_ind_;
	} else {
		for (int i = 1; i < kQuants; ++i) {
			graph_data_model_->setData (i - 1, taxis, graph_data_model_->data (graph_data_model_->index (i, taxis)));
			graph_data_model_->setData (i - 1, tempaxis, graph_data_model_->data (graph_data_model_->index (i, tempaxis)));
			graph_data_model_->setData (i - 1, huaxis, graph_data_model_->data (graph_data_model_->index (i, huaxis)));
			graph_data_model_->setData (i - 1, saxis, graph_data_model_->data (graph_data_model_->index (i, saxis)));
			graph_data_model_->setData (i - 1, heataxis, graph_data_model_->data (graph_data_model_->index (i, heataxis)));
		}
	}

	graph_data_model_->setData (last_ind_, taxis, WTime::currentServerTime());
	graph_data_model_->setData (last_ind_, tempaxis, d.temperature);
	graph_data_model_->setData (last_ind_, huaxis, d.humidity);
	graph_data_model_->setData (last_ind_, saxis, d.speed);
	graph_data_model_->setData (last_ind_, heataxis, d.heat);

	log (kInfo, hc_data_to_wtstring (d));

	makeVisibleToUser();
}

void HCWidget::displayError (const string& err)
{
	log (kError, err);

	makeVisibleToUser();
}

void HCWidget::log (HCWidget::LogLevel l, const WString& mess)
{
	++last_log_ind_;

	if (last_log_ind_ >= kMaxLogRows) {
		last_log_ind_ = 0;
		log_textarea_->setText ("");
	}

	log_textarea_->setText (log_textarea_->text() + "[" +  WTime::currentServerTime().toString() + "] " + mess + "\n");
}

void HCWidget::makeVisibleToUser()
{
	WApplication::instance()->triggerUpdate();
}
