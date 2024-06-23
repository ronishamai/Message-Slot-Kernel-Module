#ifndef MESSAGESLOT_H
#define MESSAGESLOT_H

/* INCLUDES */
#include <linux/ioctl.h>

/* DEFINES */
#define IOCTAL_COMMAND MSG_SLOT_CHANNEL
#define MAJOR_NUM 235 /* the message slot files all have the same major number, which is hard-coded to 235 */
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long) /* Set the message of the device driver (=ioctl command id, as written in the assignment desc)*/
#define DEVICE_RANGE_NAME "message_slot"
#define BUF_LEN 128 /* size of char in 32-bit UNIX applications is 1 byte; in our assignment instructions - message of up to 128 bytes from the user’s buffer */
#define DEVICE_FILE_NAME "simple_message_slot"
#define different_message_slots_device_files 256 /* there can be at most 256 different message slots device files */
#define SUCCESS 0

/* STRUCTURES 
The data structure I chose to implement is: 
    - an array of slots (because there is a limited number of possible slots - 256)
    - where each slot points to a linked list (to a node - the 1st channel if exists, NULL otherwise) of the channels associated with it [linked list bcs for each message slot file, assuming that no more than 2^20 message channels in use - bad memory complexity in case of use of few channels &  the channel ids wont defenitally be smaller than 2^20 -> a channel ID as the cell index in the array will not work...]
    - the messages are saved as data in the relevant channel */

/* message structure. Fields: 1. data - the message, 2. message length (at most 128 bytes) 
typedef struct message {
  int length;
  char data[BUF_LEN];
} message;
typedef message *message_p; */

/* channel structure. Fields: 1. the channels id (at most 2^20 different ids), 2. data - message (the mesagge & it`s length), 3. next node in the linked list (next channel of the slot) */
typedef struct channel_struct {
  unsigned long id; 
  char message[BUF_LEN];
  int length;
  struct channel_struct *next;
} channel_struct;
typedef channel_struct *channel_pointer;

/* message-slot structure (device files with different minor numbers). Fields: 1. minor number (different message slot files will have different minor numbers, allowing your driver to distinguish between them), 2. channels list - a pointer to it`s channels linked list (to the 1st channel if exists, NULL otherwise) */
typedef struct slot_struct {
  channel_pointer channels_list;
} slot_struct;
typedef slot_struct* slot_pointer; 

#endif
