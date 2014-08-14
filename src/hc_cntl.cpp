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
	const size_t kMaxErrorsCnt = 5;
	const size_t kTimerTimeoutS = 5;

	const size_t kSpeedAddition = 1;
	const char kWriteTermSymbol = '|';
	const char kReadTermSymbol = '^';
	const std::regex kReadRe ("H:\\((\\d+\\.\\d+)\\)T:\\((\\d+\\.\\d+)\\)S:\\((\\d+)\\)");

	enum RcvIndxs {
		kHumidity = 1,
		kTemperature,
		kSpeed
	};
}

HCController::HCController (WObject* parent) :
	WObject (parent),
	io_ (1U),
	sport_ (io_),
	read_buf_(),
	work_ (new as::io_service::work (io_)),
	worker_(),
	timer_ (io_)
{
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

	worker_ = thread ([this] () { io_.run(); });

	asyncRead();
}

void HCController::setSpeed (int level)
{
	std::string s;
	s += char (level + kSpeedAddition);
	s += kWriteTermSymbol;
	const shared_ptr<std::string> s_ptr = make_shared<std::string> (s);

	as::async_write (sport_, as::buffer (s_ptr->data(), s_ptr->size()),
	[ = ] (const error_code & ec, size_t bytes) { handleWrite (s_ptr, ec, bytes); });
}

void HCController::asyncRead()
{
	as::async_read_until (sport_, read_buf_, kReadTermSymbol, [this] (const error_code& ec, size_t bytes) { handleRead(ec, bytes); });
}

void HCController::startTimer()
{
	timer_.expires_from_now (seconds (kTimerTimeoutS));
	timer_.async_wait ([this] (const error_code& ec) { handleTimer(ec); });
}

void HCController::handleWrite (shared_ptr<std::string> buf, const error_code& ec, size_t bytes)
{
	if (ec || (buf->size() != bytes)) {
		err_ ("Can't transmit speed to serial port");
	}
}

void HCController::handleRead (const error_code& ec, size_t bytes)
{
	if ( (!ec) && (bytes > 0)) {
		err_counter_ = 0;

		read_buf_.commit (bytes);

		std::istream is (&read_buf_);
		std::string s;
		is >> s;

		smatch sm;
		if (regex_search (s, sm, kReadRe)) {
			hc_data_t data;

			data.humidity = stod (sm[kHumidity]);
			data.temperature = stod (sm[kTemperature]);
			data.speed = stoi (sm[kSpeed]);

			cb_ (data);
		} else {
			err_ ("Mallformed string from serial port: " + s);
		}
	} else {
		++err_counter_;

		err_ ("Can't read data from serial port");
	}

	if (err_counter_ < kMaxErrorsCnt) {
		asyncRead ();
	} else {
		startTimer ();
	}
}

void HCController::handleTimer (const error_code& ec)
{
	if (!ec) { asyncRead (); }
}
