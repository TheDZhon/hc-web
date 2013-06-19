#ifndef HC_SERIAL_H__
#define HC_SERIAL_H__

#include <string>

class HCController
{
public:
    HCController();
    ~HCController();

    void read();
    void setSpeed(int level);

    std::string temperature() const {
        return t_buf_;
    }
    std::string humidity() const {
        return h_buf_;
    }
private:
    int fd_;

    std::string t_buf_;
    std::string h_buf_;
};

#endif // HC_SERIAL_H__
