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

		kAxesCnt
	};

	inline size_t size_t_Axes_cast (Axes a)
	{
		return static_cast<size_t> (a);
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

		return WString ("Humidity: {1}.{2}%; Temperature: {3}.{4}C°; Fan Speed: {5}%")
			   .arg (h_integral).arg (h_partial)
			   .arg (t_integral).arg (t_partial)
			   .arg (data.speed);

		fesetround (old_round_mode);
	}

}

HCWidget::HCWidget (HCMaster& hc_master, Wt::WContainerWidget* parent) :
	WContainerWidget (parent),
	hc_master_ (hc_master),
	speed_feedback_lineedit_ (0),
	graph_data_model_ (0),
	log_textarea_ (0),
	last_ind_ (0),
	last_log_ind_ (0)
{
	auto top_level_layout = new WVBoxLayout;

	graph_data_model_ = new WStandardItemModel (kQuants, size_t_Axes_cast (Axes::kAxesCnt), this);

	graph_data_model_->setHeaderData (size_t_Axes_cast (Axes::kTimeAxis), WString ("Time"));
	graph_data_model_->setHeaderData (size_t_Axes_cast (Axes::kTemperatureAxis), WString ("Temperature"));
	graph_data_model_->setHeaderData (size_t_Axes_cast (Axes::kHumidityAxis), WString ("Humidity"));
	graph_data_model_->setHeaderData (size_t_Axes_cast (Axes::kSpeedAxis), WString ("Speed"));

	auto plot = new WCartesianChart;
	top_level_layout->addWidget (plot);

	plot->setModel (graph_data_model_);
	plot->setXSeriesColumn (0);
	plot->setLegendEnabled (true);
	plot->setLegendLocation (Chart::LegendInside, Wt::Right, Wt::AlignRight);

	plot->setType (Chart::CategoryChart);

	plot->axis (XAxis).setLocation (Chart::MinimumValue);
	plot->axis (YAxis).setLocation (Chart::MinimumValue);

	plot->setPlotAreaPadding (60, Top);

	WDataSeries temperature_series (size_t_Axes_cast (Axes::kTemperatureAxis), CurveSeries);
	temperature_series.setShadow (WShadow (3, 3, WColor (0, 0, 0, 127), 3));
	plot->addSeries (temperature_series);

	WDataSeries humidity_series (size_t_Axes_cast (Axes::kHumidityAxis), CurveSeries);
	humidity_series.setShadow (WShadow (3, 3, WColor (0, 0, 0, 127), 3));
	plot->addSeries (humidity_series);

	WDataSeries speed_series (size_t_Axes_cast (Axes::kSpeedAxis), CurveSeries);
	speed_series.setShadow (WShadow (3, 3, WColor (0, 0, 0, 127), 3));
	plot->addSeries (speed_series);

	plot->resize (1200, 600); // WPaintedWidget must be given explicit size

	plot->setMargin (10, Top | Bottom);            // add margin vertically
	plot->setMargin (WLength::Auto, Left | Right); // center horizontally

	auto speed_control_panel = new WPanel;
	top_level_layout->addWidget (speed_control_panel);

	auto control_layout = new WHBoxLayout;

	speed_control_panel->setTitle ("Humidifier control");

	auto speed_control_label = new WText (this);
	control_layout->addWidget (speed_control_label);
	speed_control_label->setText ("Set FAN speed: ");
	speed_control_label->setMargin (15, Bottom | Top);

	auto speed_control_slider = new WSlider (Wt::Horizontal, this);
	control_layout->addWidget (speed_control_slider, 1);
	speed_control_slider->setMinimum (0);
	speed_control_slider->setMaximum (100);
	speed_control_slider->setValue (0);
	speed_control_slider->setTickInterval (10);
	speed_control_slider->setTickPosition (WSlider::TicksBelow);
	speed_control_slider->valueChanged().connect (&hc_master_, &HCMaster::changeSpeed);

	speed_feedback_lineedit_ = new WLineEdit;
	speed_feedback_lineedit_->setReadOnly (true);
	speed_feedback_lineedit_->setAttributeValue ("style", "text-align: center;");
	control_layout->addWidget (speed_feedback_lineedit_);

	auto speed_control_container = new WContainerWidget;
	speed_control_container->setLayout (control_layout);
	speed_control_panel->setCentralWidget (speed_control_container);

	auto log_panel = new WPanel;
	auto log_container = new WContainerWidget;
	auto log_layout = new WHBoxLayout;

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
	speed_feedback_lineedit_->setText (boost::lexical_cast<std::string> (d.speed));

	const auto taxis = size_t_Axes_cast (Axes::kTimeAxis);
	const auto tempaxis = size_t_Axes_cast (Axes::kTemperatureAxis);
	const auto haxis = size_t_Axes_cast (Axes::kHumidityAxis);
	const auto saxis = size_t_Axes_cast (Axes::kSpeedAxis);

	if (last_ind_ != (kQuants - 1)) {
		last_ind_ = ++last_ind_;
	} else {
		for (int i = 1; i < kQuants; ++i) {
			graph_data_model_->setData (i - 1, taxis, graph_data_model_->data (graph_data_model_->index (i, taxis)));
			graph_data_model_->setData (i - 1, tempaxis, graph_data_model_->data (graph_data_model_->index (i, tempaxis)));
			graph_data_model_->setData (i - 1, haxis, graph_data_model_->data (graph_data_model_->index (i, haxis)));
			graph_data_model_->setData (i - 1, saxis, graph_data_model_->data (graph_data_model_->index (i, saxis)));
		}
	}

	graph_data_model_->setData (last_ind_, taxis, WTime::currentServerTime());
	graph_data_model_->setData (last_ind_, tempaxis, d.temperature);
	graph_data_model_->setData (last_ind_, haxis, d.humidity);
	graph_data_model_->setData (last_ind_, saxis, d.speed);

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
