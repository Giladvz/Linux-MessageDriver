#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include "message_slot.h"
#include <linux/slab.h>

MODULE_LICENSE("GPL");

typedef struct node {
    unsigned int channel;
    char * message;
    struct node* next;
    int message_length;
}list_node;

static list_node* create_list(void) {
    list_node* head = (list_node*)kmalloc(sizeof(list_node),GFP_KERNEL);
    if(head == NULL) {
        return NULL;
    }
    head -> next = NULL;
    head -> channel = 0;
    head -> message = NULL;
    return head;
}

static list_node * add_node(list_node* head,unsigned int channel) {
    list_node* new_node = (list_node*)kmalloc(sizeof(list_node),GFP_KERNEL);
    if (new_node == NULL) {
        return NULL;
    }
    new_node->channel = channel;
    new_node->next = head->next;
    new_node -> message = NULL;
    head->next = new_node;
    return new_node;
}

static list_node * search_list(list_node * head,unsigned int channel) {
    if (head->channel == channel) {
        return head;
    }
    if (head->next == NULL) {
        return NULL;
    }
    return search_list(head ->next,channel);
}

static void free_list(list_node* head) {
    list_node* next;
    if (head -> message != NULL){
        kfree(head -> message);
    }
    next = head -> next;
    kfree(head);
    if (next != NULL) {
        free_list(next);
    }
}

// Include array to point to 256 minor numbers, that way we know which devices where already created
static list_node* minor_array[256] = {NULL};

static ssize_t device_read( struct file* file,
                            char __user* buffer,
                            size_t       length,
                            loff_t*      offset )
{

    int i;
    list_node * message_list;
    list_node * message_node;
    unsigned int private;

    // Checks if private data has a valid argument
    if (file ->private_data == NULL) {
        return -EINVAL;
    }
    private =(unsigned int)(unsigned long)file ->private_data;
    // Get Message slot
    message_list = minor_array[iminor(file->f_inode)];

    // Looks for proper message slot
    message_node = search_list(message_list, private);
    if (message_node == NULL) {
        printk("The channel wasn't created");
        return -EWOULDBLOCK;
    }
    // Checks if message slot has a message
    if (message_node -> message == NULL) {
        return -EWOULDBLOCK;
    }
    // Checks if buffer is big enough
    if (message_node -> message_length > length) {
        return -ENOSPC;
    }
    for (i = 0; i < length;i++) {
        if (put_user(message_node -> message[i], &buffer[i]) != 0) {
            return -EFAULT;
        }
    }
    return message_node -> message_length;
}


static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
                             size_t             length,
                             loff_t*            offset)
{
    int i;
    list_node * message_list;
    list_node *message_node;
    unsigned int private;

    // Checks if private data has a valid argument
    if (file ->private_data == NULL) {
        return -EINVAL;
    }
    private =(unsigned int)(unsigned long)file ->private_data;
    message_list = minor_array[iminor(file->f_inode)];

    if (length == 0 || length > 128) {
        return -EMSGSIZE;
    }

    // Sets first use of message list
    if (message_list -> channel == 0) {
        message_node = message_list;
        message_node -> channel = private;
    }
    else {
        // Gets message slot
        message_node = search_list(message_list, private);
        // If message slot doesnt exists, creates one
        if (message_node == NULL) {
            message_node = add_node(message_list, private);
            if (message_node == NULL) {
                return -ENOMEM;
            }
        }
    }

    message_node -> message = krealloc(message_node -> message,length,GFP_KERNEL);
    if (message_node->message == NULL) {
        return -ENOMEM;
    }
    for (i = 0; i < length;i++) {
        if (get_user(message_node -> message[i], &buffer[i]) != 0) {
            return -EFAULT;
        }
    }
    message_node -> message_length = length;
    return length;
}

static int device_release (struct inode* inode,
                           struct file*  file)
{
    return 0;
}

static int device_open( struct inode* inode,
                        struct file*  file )
{
    int minor = iminor(inode);
    if (minor_array[minor] == NULL) {
        list_node* head = create_list();
        if (head == NULL) {
            return -ENOMEM;
        }
        minor_array[minor] = head;
    }
    return 0;
}

static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long   ioctl_param )
{
    unsigned int param;
    param = (unsigned int)ioctl_param;
    // Switch according to the ioctl called
    if( MSG_SLOT_CHANNEL == ioctl_command_id )
    {
        if (ioctl_param == 0) {
            return -EINVAL;
        }
        // Set private data to be the channel specified
        file->private_data = (void*)ioctl_param;
    }
    else {
        return -EINVAL;
    }

    return 0;
}

struct file_operations Fops =
        {
                .owner	        = THIS_MODULE,
                .read           = device_read,
                .write          = device_write,
                .open           = device_open,
                .unlocked_ioctl = device_ioctl,
                .release        = device_release,
        };

static int init(void){
    int regret = -1;
    ///////////////////////////////////////////////////memset????////////////////////////////////////////
    regret = register_chrdev( MAJOR_NUM, DEVICE_NAME, &Fops );
    if (regret < 0) {
        printk(KERN_ERR "There was a problem initializing Message_slot");
        return regret;
    }
    return 0;
}

static void cleanup(void) {
    int i;
    for (i = 0; i < 256; i++) {
        if (minor_array[i] != NULL) {
            free_list(minor_array[i]);
        }
    }
    unregister_chrdev(MAJOR_NUM,DEVICE_NAME);
}

module_init(init);
module_exit(cleanup);