#include "hc_serial.hpp"

#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "serial.h"

using namespace ::std;
using namespace ::Wt;

namespace
{
const char kPortName[] = "/dev/ttyUSB0";
}

HCController::HCController(WObject *parent) :
    WObject(parent),
    fd_(-1)
{
    fd_ = open(kPortName, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd_ < 0) {
        throw runtime_error(std::string("Can't open port ") + kPortName);
    }

    set_interface_attribs(fd_, B9600, 0);
    set_blocking(fd_, 1);

    read();
}

HCController::~HCController()
{
    close(fd_);
}

void HCController::read()
{

}

void HCController::setSpeed(int level)
{

}
