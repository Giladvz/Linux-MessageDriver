#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>
#define MAJOR_NUM 240
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int*)
#define DEVICE_NAME "message_slot"
#define DEVICE_FILE_NAME "message_slot"


#endif
