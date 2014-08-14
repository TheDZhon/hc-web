#ifndef HC_CNTL_H__
#define HC_CNTL_H__

#include <memory>
#include <thread>

#include <boost/asio/io_service.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/steady_timer.hpp>

#include <Wt/WObject>
#include <Wt/WTimer>

#include "data/hc_data.hpp"

namespace as = boost::asio;

class HCController:
	public Wt::WObject
{
public:
	typedef std::function<void (const hc_data_t&) > RcvdCb;
	typedef std::function<void (const std::string& err) > ErrCb;

	explicit HCController (WObject* parent = 0);
	~HCController();

	void start (size_t baud_rate, const std::string& port_name, const RcvdCb& r, const ErrCb& err);

	void setSpeed (int percents);
private:
	void asyncRead();
	void startTimer();

	void handleWrite (std::shared_ptr<std::string> buf, const boost::system::error_code& ec, size_t bytes);
	void handleRead (const boost::system::error_code& ec, size_t bytes);
	void handleTimer (const boost::system::error_code& ec);

	as::io_service io_;
	as::serial_port sport_;
	as::streambuf read_buf_;

	std::unique_ptr<as::io_service::work> work_;
	std::thread worker_;

	RcvdCb cb_;
	ErrCb err_;

	as::steady_timer timer_;

	size_t err_counter_;
};

#endif // HC_CNTL_H__
