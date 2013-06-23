#include "hc_cntl.hpp"

#include <stdexcept>

#include <boost/regex.hpp>

#include <boost/lexical_cast.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>

using namespace ::std;
using namespace ::Wt;
using namespace ::boost;
using namespace ::boost::system;
using namespace ::boost::posix_time;

namespace
{
const size_t kBaudRate = 9600U;
const char kPortName[] = "/dev/ttyUSB0";
const char kTermSymbol = '|';
const boost::regex kReadRe("H:(\\d+)T:(\\d+)S:(\\d+)");

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
    worker_(),
    timer_(io_)
{
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

    sport_.set_option(as::serial_port_base::baud_rate(kBaudRate), ec);
    if (ec) {
        err_("Can't set port speed!");
        return;
    }

    worker_ = boost::thread(boost::bind(&as::io_service::run, boost::ref(io_)));

    asyncRead();
	startTimer();
}

void HCController::setSpeed(int level)
{
    const std::string &s = boost::lexical_cast<std::string>(level) + kTermSymbol;
    const boost::shared_ptr<std::string> s_ptr = boost::make_shared<std::string> (s);

    as::async_write(sport_, as::buffer(s_ptr->data(), s_ptr->size()),
                    boost::bind(&HCController::handleWrite, this, s_ptr, _1, _2));
}

void HCController::asyncRead()
{
    as::async_read_until(sport_, read_buf_, kTermSymbol,
                         boost::bind(&HCController::handleRead, this, _1, _2));
}

void HCController::startTimer()
{
    timer_.expires_from_now(seconds(3));
    timer_.async_wait(boost::bind(&HCController::handleTimer, this, _1));
}

void HCController::handleWrite(boost::shared_ptr<std::string> buf, const error_code &ec, size_t bytes)
{
    if (ec || (buf->size() != bytes)) {
        err_("Can't transmit speed to serial port");
    }
}

void HCController::handleRead(const error_code &ec, size_t bytes)
{
    if ((!ec) && (bytes > 0)) {
        read_buf_.commit(bytes);

        std::istream is(&read_buf_);
        std::string s;
        is >> s;

        boost::smatch sm;
        if (boost::regex_match(s, sm, kReadRe)) {
            hc_data_t data;

            data.humidity = boost::lexical_cast<double>(sm[kHumidity]);
            data.temperature = boost::lexical_cast<double>(sm[kTemperature]);
            data.speed = boost::lexical_cast<int>(sm[kSpeed]);

            cb_(data);
        } else {
            err_("Mallformed string from serial port");
        }
    } else {
        err_("Can't read data from serial port");
    }

    asyncRead();
}

void HCController::handleTimer(const error_code &ec)
{
	//TODO(DZhon): Timed actions can be performed here
	
	startTimer();
}
