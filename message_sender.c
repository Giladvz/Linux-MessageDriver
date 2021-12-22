#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message_slot.h"
#include <errno.h>


int main(int argc,char* argv[]) {
    int fd;
    if (argc != 4) {
        errno = EINVAL;
        perror("Too many or not enough arguments");
        exit(1);
    }
    if ((fd = open(argv[1], O_RDWR)) < 0) {
        perror("Could not open the file");
        exit(1);
    }
    // Assume argv[2] is a number
    if (ioctl(fd,MSG_SLOT_CHANNEL,(unsigned int)atoi(argv[2])) <0 ) {
        perror("Setting channel in fd failed");
        exit(1);
    }
    if (write(fd,argv[3],strlen(argv[3])) < 0) {
        perror("Writing in message slot failed");
        exit(1);
    }
    if (close(fd) < 0) {
        perror("Closed failed");
        exit(1);
    }
    exit(0);

}


