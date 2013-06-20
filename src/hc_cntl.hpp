#ifndef HC_CNTL_H__
#define HC_CNTL_H__

#include <functional>
#include <memory>
#include <thread>

#include <boost/asio/io_service.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>

#include <Wt/WObject>
#include <Wt/WTimer>

#include "data/hc_data.hpp"

namespace as = boost::asio;

class HCController:
    public Wt::WObject
{
public:
    typedef std::function<void (const hc_data_t &)> RcvdCb;
    typedef std::function<void (const std::string &err)> ErrCb;

    HCController(WObject *parent = 0);
    ~HCController();

    void start(const RcvdCb &r, const ErrCb &err);

    void setSpeed(int percents);
private:
    void asyncRead();
    void handleRead(const boost::system::error_code &ec, size_t bytes);

    as::io_service io_;
    as::serial_port sport_;
    as::streambuf read_buf_;

    std::unique_ptr<as::io_service::work> work_;
    std::thread worker_;

    RcvdCb cb_;
    ErrCb err_;
};

#endif // HC_CNTL_H__
