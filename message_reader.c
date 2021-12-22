#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include "message_slot.h"
#include <errno.h>

int main(int argc,char* argv[]) {
    int fd,num_read;
    char buff[128];
    if (argc != 3) {
        errno = EINVAL;
        perror("Too many or not enough arguments");
        exit(1);
    }
    if ((fd = open(argv[1], O_RDWR)) < 0) {
        perror("Could not open the file");
        exit(1);
    }
    // Assume argv[2] is a number
    if (ioctl(fd,MSG_SLOT_CHANNEL,(unsigned int)atoi(argv[2])) < 0 ) {
        perror("Setting channel in fd failed");
        exit(1);
    }
    if ((num_read = read(fd,buff,128)) < 0) {
        perror("reading in message slot failed");
        exit(1);
    }
    if (write(1,buff, num_read) < 0) {
        perror("Writing in message slot failed");
        exit(1);
    }
    if (close(fd) < 0) {
        perror("Closed failed");
        exit(1);
    }
    exit(0);
}


