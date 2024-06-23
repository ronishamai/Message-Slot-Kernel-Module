#include <string.h> /* for strlen() method, strerror()... */
#include <stdio.h> /* standart input-output */
#include <stdlib.h> /* defines four variable types, several macros, and various functions for performing general functions */
#include <errno.h> /* for strerror() */
#include <fcntl.h> /* flags for open() */
#include <unistd.h> /* symbolic constants are defined for file streams; here, for STDOUT_FILENO (1) */
#include <sys/ioctl.h> /* declares the function ioctl */
#include "message_slot.h" 

int main(int argc, char *argv[])
{
    /* init variables */
    char *message_slot_file_path, *message;
    unsigned long target_message_channel_id;
    int file_descriptor, message_length;

    /* command line arguments */
    if(!(argc == 4))  /* validate that the correct number of command line arguments is passed. */
        { fprintf(stderr, "An error occurred: invalid number of command line arguments. %s \n", strerror(errno)); exit(1); }
    message_slot_file_path = argv[1]; /* argv[1]: message slot file path. */
    
    target_message_channel_id = (unsigned long)atoi(argv[2]); /* argv[2]: the target message channel id. Assume a non-negative integer. */
    message = argv[3]; /* argv[3]: the message to pass. */
    message_length = strlen(message);

    /* 1. Open the specified message slot device file. */
    file_descriptor = open(message_slot_file_path, O_WRONLY); /* O_WRONLY: write-only flag */
    if(file_descriptor == -1) 
        { fprintf(stderr, "An error occurred: open() failed. %s \n", strerror(errno)); exit(1); }

    /* 2. Set the channel id to the id specified on the command line. */
    if(ioctl(file_descriptor, IOCTAL_COMMAND, target_message_channel_id) == -1) 
        { fprintf(stderr, "An error occurred: ioctl() failed. %s \n", strerror(errno)); close(file_descriptor); exit(1); }

    /* 3. Write the specified message to the message slot file. Don’t include the terminating null character of the C string as part of the message */
    if(write(file_descriptor, message, message_length) != message_length) 
        { fprintf(stderr, "An error occurred: write() failed. %s \n", strerror(errno)); close(file_descriptor); exit(1); }
    
    /* 4. Close the device. */
    if(close(file_descriptor) == -1) 
        { fprintf(stderr, "An error occurred: close() failed. %s \n", strerror(errno)); exit(1); }

    /* 5. Exit the program with exit value 0. */
    exit(0);
}
