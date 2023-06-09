// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <signal.h>
#include <stdlib.h>
#include <time.h>

static int serial_port = 0;

void sig_handler(int signo) {
    if (signo == SIGTERM || signo == SIGKILL)
        printf("received SIGTERM\n");

    close(serial_port);


}

int main() {

    serial_port = open("/dev/cu.usbmodem113403", O_RDWR);

    struct termios tty;

    if (tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)


    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                     ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 30;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    // Write to serial port
//    unsigned char msg[] = {0xc0, 0x00, 0x80, 0x00, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
//                           0xee, 0xee, 0xee, 0xee, 0xee, 0xc0};
    unsigned char msg[261];
    msg[0] = 0xc0;
    msg[1] = 0x00;
    msg[2] = 0x80;
    msg[3] = 0x00;
    for (int i = 4; i < sizeof(msg) / sizeof(unsigned char); ++i) {
        msg[i] = 0xAA;
    }
    msg[257] = 0xdb;
    msg[258] = 0xdc;
    msg[259] = 0xaa;
    msg[260] = 0xc0;

    struct timespec tw = {0, 215000000};
    struct timespec tr;

    uint8_t ret_msg[1024];
    for (int i = 0; i < 50000; ++i) {
        //write(serial_port, msg, sizeof(msg) / sizeof(unsigned char));
        int num_bytes = read(serial_port, ret_msg, sizeof(msg) / sizeof(unsigned char));
        printf("Callback: ");
        for (int j = 0; j < num_bytes; ++j) {
            printf("0x%02X ", ret_msg[j]);
        }
        printf("\n");
//        fflush(stdout);
        nanosleep(&tw, &tr);
    }

    // Allocate memory for read buffer, set size according to your needs
    char read_buf[256];

    // Normally you wouldn't do this memset() call, but since we will just receive
    // ASCII data for this example, we'll set everything to 0 so we can
    // call printf() easily.
    memset(&read_buf, '\0', sizeof(read_buf));

    // Read bytes. The behaviour of read() (e.g. does it block?,
    // how long does it block for?) depends on the configuration
    // settings above, specifically VMIN and VTIME

//    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
//
//    // n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
//    if (num_bytes < 0) {
//        printf("Error reading: %s", strerror(errno));
//        return 1;
//    }
//
//    // Here we assume we received ASCII data, but you might be sending raw bytes (in that case, don't try and
//    // print it to the screen like this!)
//    printf("Read %i bytes. Received message:\n", num_bytes);
//    for (int i = 0; i < num_bytes; ++i) {
//        printf("0x%02X ", read_buf[i]);
//    }
//    printf("\n");


    close(serial_port);

    return 0;
}
