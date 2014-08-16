#include "hc_cntl.hpp"

#include <stdexcept>
#include <regex>
#include <chrono>

#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>

using namespace ::std;
using namespace ::std::chrono;
using namespace ::Wt;
using namespace ::boost::system;

namespace
{
	const auto kTimerTimeoutS = 10U;
	const auto kTermSymbol = '|';

	const std::regex kReadStateRe ("H:\\((\\d+\\.\\d+)\\)T:\\((\\d+\\.\\d+)\\)S:\\((\\d+)\\)P:\\((\\d+)\\)");

	enum class RcvIndxs : size_t
	{
		kHumidity = 1U,
		kTemperature,
		kSpeed,
		kHeat
	};

	inline size_t size_t_RI_cast (RcvIndxs i)
	{
		return static_cast<size_t> (i);
	}
}

HCController::HCController (WObject* parent) :
	WObject (parent),
	io_ (1U),
	sport_ (io_),
	read_buf_(),
	work_ (new as::io_service::work (io_)),
	worker_(),
	baud_rate_ (0U),
	port_name_(),
	deadline_timer_ (io_)
{
	worker_ = thread ([this] () { io_.run(); });
}

HCController::~HCController()
{
	sport_.cancel();
	sport_.close();

	work_.reset();
	worker_.join();
}

void HCController::start (size_t baud_rate, const std::string& port_name, const HCController::RcvdCb& r, const HCController::ErrCb& err)
{
	baud_rate_ = baud_rate;
	port_name_ = port_name;
	cb_ = r;
	err_ = err;

	error_code ec;
	sport_.open (port_name, ec);
	if (ec) {
		err_ ("Can't open port " + port_name);
		return;
	}

	sport_.set_option (as::serial_port_base::baud_rate (baud_rate), ec);
	if (ec) {
		err_ ("Can't set port speed " + to_string (baud_rate));
		return;
	}

	asyncRead();
}

void HCController::setSpeed (int percents)
{
	prepareSend (string ("S:") + to_string (percents) + kTermSymbol);
}

void HCController::setHeat (int percents)
{
	prepareSend (string ("P:") + to_string (percents) + kTermSymbol);
}

void HCController::refreshWater()
{
	prepareSend (string ("R") + kTermSymbol);
}

void HCController::prepareSend (const string& data)
{
	const auto s_ptr = make_shared<string> (data);
	as::async_write (sport_, as::buffer (s_ptr->data(), s_ptr->size()),
	[ = ] (const error_code & ec, size_t bytes) { handleWrite (s_ptr, ec, bytes); });
}

void HCController::asyncRead()
{
	as::async_read_until (sport_, read_buf_, kTermSymbol,
	[this] (const error_code & ec, size_t bytes) { handleRead (ec, bytes); });
	startTimer();
}

void HCController::startTimer()
{
	deadline_timer_.expires_from_now (seconds (kTimerTimeoutS));
	deadline_timer_.async_wait ([this] (const error_code & ec) { handleTimer (ec); });
}

void HCController::handleWrite (shared_ptr<std::string> buf, const error_code& ec, size_t bytes)
{
	if (ec || (buf->size() != bytes)) {
		err_ ("Can't transmit data to serial port");
	}
}

void HCController::handleRead (const error_code& ec, size_t bytes)
{
	if (ec == as::error::operation_aborted) { return; }

	deadline_timer_.cancel();

	if ( (!ec) && (bytes > 0)) {
		read_buf_.commit (bytes);

		std::istream is (&read_buf_);
		std::string s;
		is >> s;

		smatch sm;
		if (regex_search (s, sm, kReadStateRe)) {
			hc_data_t data;

			data.humidity = stod (sm[size_t_RI_cast (RcvIndxs::kHumidity)]);
			data.temperature = stod (sm[size_t_RI_cast (RcvIndxs::kTemperature)]);
			data.speed = stoi (sm[size_t_RI_cast (RcvIndxs::kSpeed)]);
			data.heat = stoi (sm[size_t_RI_cast (RcvIndxs::kHeat)]);

			cb_ (data);
		} else {
			err_ (string ("Mallformed string from serial port: ") + s);
		}
	} else {
		err_ (string ("Can't read data from serial port: ") + ec.message());
	}

	asyncRead();
}

void HCController::handleTimer (const error_code& ec)
{
	if (ec == as::error::operation_aborted) { return; }

	err_ ("Serial port read timeout!");

	sport_.close();
	start (baud_rate_, port_name_, cb_, err_);
}
