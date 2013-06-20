#include "hc_cntl.hpp"

#include <regex>
#include <stdexcept>

#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>

using namespace ::std;
using namespace ::Wt;
using namespace ::boost;
using namespace ::boost::system;

namespace
{
const size_t kBaudRate = 9600U;
const char kPortName[] = "/dev/ttyUSB0";
const char kTermSymbol = '|';
const std::regex kReadRe("[H:(\\d+)][T:(\\d+)][S:(\\d+)]");

enum RcvIndxs {
    kHumidity = 1,
    kTemperature,
    kSpeed
};
}

HCController::HCController(WObject *parent) :
    WObject(parent),
    io_(1U),
    sport_(io_),
    read_buf_(),
    work_(new as::io_service::work(io_)),
    worker_()
{
    sport_.set_option(as::serial_port_base::baud_rate(kBaudRate));
}

HCController::~HCController()
{
	sport_.cancel();
	sport_.close();
	
    work_.reset();
    worker_.join();
}

void HCController::start(const HCController::RcvdCb &r, const HCController::ErrCb &err)
{
    cb_ = r;
    err_ = err;

    error_code ec;
    sport_.open(kPortName, ec);
    if (ec) {
        err_("Can't open port!");
        return;
    }

    worker_ = std::thread([&]() {
        io_.run();
    });

    asyncRead();
}

void HCController::setSpeed(int level)
{
    auto && s = std::to_string(level) + kTermSymbol;
    const auto s_ptr = std::make_shared<std::string> (std::move(s));

    as::async_write(sport_, as::buffer(s_ptr->data(), s_ptr->size()),
    [s_ptr, this](const error_code & ec, size_t bytes) {
        if (ec || (s_ptr->size() != bytes)) {
            err_("Can't transmit speed to serial port");
        }
    });
}

void HCController::asyncRead()
{
    as::async_read_until(sport_, read_buf_, kTermSymbol,
    [&](const error_code & ec, size_t bytes) {
        handleRead(ec, bytes);
    });
}

void HCController::handleRead(const error_code &ec, size_t bytes)
{
    if ((!ec) && (bytes > 0)) {
        read_buf_.commit(bytes);

        std::istream is(&read_buf_);
        std::string s;
        is >> s;

        std::smatch sm;
        if (std::regex_match(s, sm, kReadRe)) {
            hc_data_t data;
			
			data.humidity = std::stod (sm[kHumidity]);
			data.temperature = std::stod (sm[kTemperature]);
			data.speed = std::stoi (sm[kSpeed]);
			
			cb_ (data);
		} else {
            err_("Mallformed string from serial port");
        }
    } else {
        err_("Can't read data from serial port");
    }

    asyncRead();
}
