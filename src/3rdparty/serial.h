#ifndef SERIAL_H__
#define SERIAL_H__

int set_interface_attribs(int fd, int speed, int parity);
void set_blocking(int fd, int should_block);

#endif // SERIAL_H__
