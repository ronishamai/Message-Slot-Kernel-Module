#include <string.h> /* for strerror() */
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
    char message[BUF_LEN], *message_slot_file_path;
    unsigned long target_message_channel_id;
    int file_descriptor, message_length;

    /* command line arguments */
    if(!(argc == 3)) /* validate that the correct number of command line arguments is passed. */
        { fprintf(stderr, "An error occurred: invalid number of command line arguments. %s \n", strerror(errno)); exit(1); }
    message_slot_file_path = argv[1]; /* argv[1]: message slot file path. */
    target_message_channel_id = (unsigned long)atoi(argv[2]); /* argv[2]: the target message channel id. Assume a non-negative integer. */

    /* 1. Open the specified message slot device file. */
    file_descriptor = open(message_slot_file_path, O_RDONLY); /* O_RDONLY: read-only flag */
    if(file_descriptor == -1) 
        { fprintf(stderr, "An error occurred: open() failed. %s \n", strerror(errno)); exit(1); }

    /* 2. Set the channel id to the id specified on the command line. */
    if(ioctl(file_descriptor, IOCTAL_COMMAND, target_message_channel_id) == -1)  
        { fprintf(stderr, "An error occurred: ioctl() failed. %s \n", strerror(errno)); close(file_descriptor); exit(1); }

    /* 3. Read a message from the message slot file to a buffer. */
    message_length = read(file_descriptor, message, BUF_LEN);
    if(message_length == -1) 
        { fprintf(stderr, "An error occurred: read() failed. %s \n", strerror(errno)); close(file_descriptor); exit(1); }

    /* 4. Close the device. */
    if(close(file_descriptor) == -1) 
        { fprintf(stderr, "An error occurred: close() failed. %s \n", strerror(errno)); exit(1); }
    
    /* 5. Print the message to standard output (using the write() system call). Print only the message, without any additional text. */
    if(!(write(STDOUT_FILENO, message, message_length) == message_length)) 
        { fprintf(stderr, "An error occurred: write() failed. %s \n", strerror(errno)); exit(1); }

    /* 6. Exit the program with exit value 0. */
    exit(0);
}