/* CREDIT: the template for the code is taken from rec6 code file */
/* ------------------------------ GIVEN PROTOTYPES ------------------------------ */
/* Declare what kind of code we want from the header files. Defining __KERNEL__ and MODULE allows us to access kernel-level code not usually available to userspace programs. */
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h! */
#include <linux/slab.h>     /* for Allocate memory using kmalloc() with GFP_KERNEL flag */

/* ------------------------------ ADDITIONAL PROTOTYPES ------------------------------ */
#include "message_slot.h"   /* Our custom definitions of IOCTL operations */
MODULE_LICENSE("GPL");
static slot_pointer slots_arr[different_message_slots_device_files] = {NULL}; /* static slots array */

/* ------------------------------ HELPERS ------------------------------ */
/* create & init slot by minor number if it doesn't exist, otherwise returns the slot we alreay created from index = minor in slots array */
slot_pointer create_or_get_slot(int minor_number) {
  slot_pointer slot = slots_arr[minor_number];
  if (slot != NULL) { /* check if it has already created a channels list data structure for the file being opened */
    return slot; 
  }
  slot = (slot_pointer)kmalloc(sizeof(slot_struct), GFP_KERNEL); /* create one if not + allocate memory using kmalloc() with GFP_KERNEL flag */
  if (slot == NULL) {
    return NULL;
  }
  slot->channels_list = NULL;
  return slot;
}

/* given a reference to the head of a list and an int, push a new node on the front of the list. returns false in case of error, otherwise returns true */
int push_new_channel_to_head_of_empty_channels_list (slot_pointer slot, unsigned long ioctl_param) {
  channel_pointer new_channel = (channel_pointer)kmalloc(sizeof(channel_struct), GFP_KERNEL); /* allocate channel node */
  if (new_channel == NULL) { /* Error case: failing to allocate memory */
    return 0;
  } 
  new_channel->id = ioctl_param; /* put in the key (the new id) */  
  new_channel->length = 0;
  new_channel->next = NULL;
  slot->channels_list = new_channel; /* move the head to point to the new node */
  return 1;
}

int push_new_channel_to_head_of_non_empty_channels_list (slot_pointer slot, unsigned long ioctl_param) {
  channel_pointer new_channel = (channel_pointer)kmalloc(sizeof(channel_struct), GFP_KERNEL); /* allocate channel node */
  if (new_channel == NULL) { /* Error case: failing to allocate memory */
    return 0;
  }
  new_channel->id = ioctl_param; /* put in the key (the new id) */  
  new_channel->length = 0;
  new_channel->next = slot->channels_list;
  slot->channels_list = new_channel; /* move the head to point to the new node */
  return 1;
}

/* checks whether the id is present in the channels linked list */
int search_id (slot_pointer slot, unsigned long ioctl_param)
{
  channel_pointer current_channel = slot->channels_list;
  while (current_channel != NULL) {
    if (current_channel->id == ioctl_param) {
      return 1; /* success */
    }
    current_channel = current_channel->next;
  }
  return 0; /* fail */
}


int id_is_head(slot_pointer slot, unsigned long ioctl_param) {
  if (slot->channels_list->id == ioctl_param) {
    return 1; /* success */
  }
  return 0; /* fail */
}

/* change the place of the exist channel to the head. returns false in case of error, otherwise returns true */
/* CREDIT: https://stackoverflow.com/questions/30135871/move-item-to-the-front-of-the-linkedlist */
void move_channel_to_head_of_non_empty_channels_list(slot_pointer slot, unsigned long ioctl_param) {
  channel_pointer prev_channel = NULL;
  channel_pointer curr_channel = NULL;
  channel_pointer head_channel = NULL;
  head_channel = slot->channels_list;
  curr_channel = slot->channels_list;

  while (curr_channel != NULL)
  {
    if ((curr_channel->id == ioctl_param) && (prev_channel != NULL)) {
      prev_channel->next = curr_channel->next; /* Update the previous node to point to the next node */
      curr_channel->next = head_channel; /* Move the current node to point to the starting position */
      head_channel = curr_channel;
    }
    prev_channel = curr_channel;
    curr_channel = curr_channel->next;
  }
  slot->channels_list = head_channel;
}

/* free all memory allocated: slots & channels */
void free_memory(void){
  int i;
  slot_pointer slot;
  channel_pointer curr_channel = NULL;
  channel_pointer next_channel = NULL;

  for (i = 0; i < different_message_slots_device_files; i++){
    if (slots_arr[i] != NULL) {
      slot = slots_arr[i];
      if (slot->channels_list != NULL) {
        curr_channel = slot->channels_list; 
        while (curr_channel != NULL) {
          next_channel = curr_channel->next;
          kfree(curr_channel);
          curr_channel = next_channel;
        }
      }
      kfree(slot);
      slots_arr[i] = NULL;
    }
  }
}

/* ---------------------------- DEVICE FUNCTIONS ---------------------------- */
/* ---------------------- OPEN ---------------------- */
/* open the message slot for use (open the file that represents it) */
static int device_open(struct inode* inode, struct file* file )
{ 
  int minor_number = iminor(inode); /* we can get the opened file’s minor number using the iminor() kernel function (applied to the struct inode* argument of device_open()). */
  slot_pointer slot = create_or_get_slot(minor_number);
  if (slot == NULL) { /* Error case: module initialization fails, print an error message using printk */
    printk("KERNEL ERROR: %s \n", "failing to allocate memory"); 
    return(-1); 
  } 
  slots_arr[minor_number] = slot;
  file->private_data = slot;
  return SUCCESS;
}

/* ---------------------- READ ---------------------- */
/* a process which has already opened the device file attempts to read from it the last message written (in the channel) into the user’s buffer. Returns the number of bytes read, unless an error occurs. */
static ssize_t device_read(struct file* file, char __user* buffer, size_t length,loff_t* offset )
{
  slot_pointer slot = (slot_pointer)(file->private_data);
  channel_pointer channel = slot->channels_list;
  int chars_read = 0;

  if ((channel == NULL) || (buffer == NULL)) { /* no channel has been set on the file descriptor */
    return -EINVAL; 
  } 
  if (channel->length == 0) { /* no message exists on the channel */
    return -EWOULDBLOCK; 
  } 
  if (channel->length > length) { /* the provided buffer length is too small to hold the last message written on the channel */
    return -ENOSPC; 
  }
     

  /* reads data from channel to the given buffer, returns EFAULT in case of error */
  for (chars_read = 0; chars_read < channel->length; chars_read++) {
    if ((put_user(channel->message[chars_read], &buffer[chars_read])) < 0) { /* Error case: not an atomic reading */
      return -EFAULT; 
    } 
  }
  return chars_read; /* returns the number of bytes read, unless an error occurs */
}

/* ---------------------- WRITE ---------------------- */
/* a processs which has already opened the device file attempts to write to it a non-empty message of up to 128 bytes from the user’s buffer to the channel. Returns the number of bytes written, unless an error occurs. */
static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset)
{
  slot_pointer slot = (slot_pointer)(file->private_data);
  channel_pointer channel = slot->channels_list;
  char draft_message[BUF_LEN];
  int chars_written = 0;

  if ((channel == NULL) || (buffer == NULL)) { /* no channel has been set on the file descriptor */
    return -EINVAL; 
  } 
  if ((length == 0) || (length > BUF_LEN)) { /* the passed message length is 0 or more than 128 */
    return -EMSGSIZE; 
  } 
  
  /* message slot reads/write should be atomic: they should always read/write the entire passed message and not parts of it. So a successful write() always returns the number of bytes in the supplied message and a successful read() returns the number of bytes in the last message written on the channel. */
  for (chars_written = 0; chars_written < length; chars_written++) { /* get_user (variable to store result, source address in user space) */
    if ((get_user(draft_message[chars_written], &buffer[chars_written])) < 0) { /* reads data from channel to the given buffer, returns EFAULT in case of error */
      return -EFAULT;
    } 
  }
  /* succeed writing the atomic meassage into draft_message. now, we can copy it from draft_message to the field data of out message struct. */
  for (chars_written = 0; chars_written < length; chars_written++) {
    channel->message[chars_written] = draft_message[chars_written];
  }

  channel->length = chars_written; /* update message length (a field of message struct) */
  return chars_written; /* returns the number of bytes written, unless an error occurs.*/
}

/* ---------------------- IOCTL ---------------------- */
/* sets the file descriptor’s channel id */
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param )
{  
  slot_pointer slot = (slot_pointer)(file->private_data);
  if (ioctl_command_id != MSG_SLOT_CHANNEL) { /* Error case: the passed command is not MSG_SLOT_CHANNEL */
    return -EINVAL;
  } 
  if (ioctl_param == 0) { /* Error case: the passed channel id is 0 */
    return -EINVAL;
  } 

  /* case 1: channels list is empty */
  if (slot->channels_list == NULL) {
    if (push_new_channel_to_head_of_empty_channels_list(slot, ioctl_param) == 0) {
      return -1; 
    }
  }

  /* case 2: channels list is not empty */ 
  else {
    /* case 2A: ioctl_param (id) does not exist in channel list */
    if (search_id(slot, ioctl_param) == 0) {
      if (push_new_channel_to_head_of_non_empty_channels_list(slot, ioctl_param) == 0) {
        return -1; 
      }
    }
    /* case 2B: ioctl_param (id) exist in channel list */
    
    else {
      /* case 2Ba: ioctl_param (id) is not head */
      if (id_is_head(slot, ioctl_param) == 0) {
        if (id_is_head(slot,ioctl_param) == 0) {
          move_channel_to_head_of_non_empty_channels_list(slot, ioctl_param); 
        }
      }
      /* case 2Bb: ioctl_param (id) is head - do nothing */
    }
  }
  return SUCCESS;
}
    
/* ---------------------- DEVICE SETUP ---------------------- */
/* this structure will hold the functions to be called when a process does something to the device we created */
struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
};

/* initialize the module - Register the character device */
static int __init simple_init(void) {
  int rc = -1;
  rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops); /* Register driver capabilities. Obtain major num */
  if(rc < 0) {  /* Negative values signify an error */
    printk(KERN_ALERT "%s registraion failed for  %d\n", DEVICE_FILE_NAME, MAJOR_NUM);
    return rc; 
  }
  return 0; 
}

/* cleanup the module */
static void __exit simple_cleanup(void) {
  free_memory();
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME); 
}

module_init(simple_init);
module_exit(simple_cleanup);

