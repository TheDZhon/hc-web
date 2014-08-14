#include "hc_wgt.hpp"

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
	const size_t kQuants = 30;
	const size_t kMaxLogRows = 100;

	enum Axes {
		kTimeAxis,
		kTemperatureAxis,
		kHumidityAxis,
		kSpeedAxis,

		kAxesCnt
	};

	WString hc_data_to_wtstring (const hc_data_t& data)
	{
		const int h_integral = data.humidity;
		const int t_integral = data.temperature;

		const int h_partial = (data.humidity - h_integral) * 10 + 0.5;
		const int t_partial = (data.temperature - t_integral) * 10 + 0.5;

		return WString ("Humidity: {1}.{2}%; Temperature: {3}.{4}C; Fan Speed: {5}%")
			   .arg (h_integral).arg (h_partial)
			   .arg (t_integral).arg (t_partial)
			   .arg (data.speed);
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
	WVBoxLayout* top_level_layout = new WVBoxLayout;

	graph_data_model_ = new WStandardItemModel (kQuants, kAxesCnt, this);

	graph_data_model_->setHeaderData (kTimeAxis, WString ("Time"));
	graph_data_model_->setHeaderData (kTemperatureAxis, WString ("Temperature"));
	graph_data_model_->setHeaderData (kHumidityAxis, WString ("Humidity"));
	graph_data_model_->setHeaderData (kSpeedAxis, WString ("Speed"));

	WCartesianChart* plot = new WCartesianChart;
	top_level_layout->addWidget (plot);

	plot->setModel (graph_data_model_);
	plot->setXSeriesColumn (0);
	plot->setLegendEnabled (true);
	plot->setLegendLocation (Chart::LegendInside, Wt::Right, Wt::AlignRight);

	plot->setType (Chart::CategoryChart);

	plot->axis (XAxis).setLocation (Chart::MinimumValue);
	plot->axis (YAxis).setLocation (Chart::MinimumValue);

	plot->setPlotAreaPadding (60, Top);

	WDataSeries time_series (kTemperatureAxis, CurveSeries);
	time_series.setShadow (WShadow (3, 3, WColor (0, 0, 0, 127), 3));
	plot->addSeries (time_series);

	WDataSeries humidity_series (kHumidityAxis, CurveSeries);
	humidity_series.setShadow (WShadow (3, 3, WColor (0, 0, 0, 127), 3));
	plot->addSeries (humidity_series);

	WDataSeries speed_series (kSpeedAxis, CurveSeries);
	speed_series.setShadow (WShadow (3, 3, WColor (0, 0, 0, 127), 3));
	plot->addSeries (speed_series);

	plot->resize (1200, 600); // WPaintedWidget must be given explicit size

	plot->setMargin (10, Top | Bottom);            // add margin vertically
	plot->setMargin (WLength::Auto, Left | Right); // center horizontally

	WPanel* speed_control_panel = new WPanel;
	top_level_layout->addWidget (speed_control_panel);

	WHBoxLayout* control_layout = new WHBoxLayout;

	speed_control_panel->setTitle ("Humidifier control");

	WText* speed_control_label = new WText (this);
	control_layout->addWidget (speed_control_label);
	speed_control_label->setText ("Set FAN speed: ");
	speed_control_label->setMargin (15, Bottom | Top);

	WSlider* speed_control_slider = new WSlider (Wt::Horizontal, this);
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

	WContainerWidget* speed_control_container = new WContainerWidget;
	speed_control_container->setLayout (control_layout);
	speed_control_panel->setCentralWidget (speed_control_container);

	WPanel* log_panel = new WPanel;
	WContainerWidget* log_container = new WContainerWidget;
	WHBoxLayout* log_layout = new WHBoxLayout;

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

	if (last_ind_ != (kQuants - 1)) {
		last_ind_ = ++last_ind_;
	} else {
		for (int i = 1; i < kQuants; ++i) {
			graph_data_model_->setData (i - 1, kTimeAxis, graph_data_model_->data (graph_data_model_->index (i, kTimeAxis)));
			graph_data_model_->setData (i - 1, kTemperatureAxis, graph_data_model_->data (graph_data_model_->index (i, kTemperatureAxis)));
			graph_data_model_->setData (i - 1, kHumidityAxis, graph_data_model_->data (graph_data_model_->index (i, kHumidityAxis)));
			graph_data_model_->setData (i - 1, kSpeedAxis, graph_data_model_->data (graph_data_model_->index (i, kSpeedAxis)));
		}
	}

	graph_data_model_->setData (last_ind_, kTimeAxis, WTime::currentServerTime());
	graph_data_model_->setData (last_ind_, kTemperatureAxis, d.temperature);
	graph_data_model_->setData (last_ind_, kHumidityAxis, d.humidity);
	graph_data_model_->setData (last_ind_, kSpeedAxis, d.speed);

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
