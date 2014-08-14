#ifndef HC_WGT_H__
#define HC_WGT_H__

#include <functional>

#include <Wt/WContainerWidget>

#include "hc_master.hpp"

namespace Wt
{
	class WString;
}

class HCWidget: public Wt::WContainerWidget
{
public:
	explicit HCWidget (HCMaster& hc_master, WContainerWidget* parent = 0);
	virtual ~HCWidget();

	void displayData (const hc_data_t& d);
	void displayError (const std::string& err);
private:
	enum LogLevel {
		kDebug,
		kInfo,
		kError
	};

	void log (LogLevel l, const Wt::WString& mess);

	void makeVisibleToUser();

	HCMaster& hc_master_;

	Wt::WLineEdit* speed_feedback_lineedit_;
	Wt::WStandardItemModel* graph_data_model_;
	Wt::WTextArea* log_textarea_;

	int last_ind_;
	int last_log_ind_;
};

#endif // HC_WGT_H__
