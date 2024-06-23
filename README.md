## Message Slot Kernel Module

This project demonstrates an inter-process communication (IPC) mechanism using a custom kernel module in Linux. It allows processes to exchange messages through virtual "message slots" managed by the kernel.

## Overview:

The project consists of three main components:

1. **Kernel Module (`message_slot`)**:
   - Acts as a virtual device driver (`/dev/slotX`) with fixed configurations.
   - Supports creating multiple message slots, each with its own channels for communication.
   - Handles operations like setting channels (`ioctl`), sending messages (`write`), and receiving messages (`read`).

2. **User Programs**:
   - **`message_sender`**: Sends messages to specific channels of message slots.
   - **`message_reader`**: Reads messages from specific channels of message slots.
   - These programs interact with the kernel module to manage and exchange messages.

3. **Makefile**:
   - Provides commands to compile the kernel module (`message_slot.ko`) and user programs (`message_sender` and `message_reader`).
   - Includes cleanup commands to remove compiled binaries and object files.

## Usage Instructions:

### Setting Up:

1. **Building**:
   - Open a terminal and navigate to the project directory.
   - Run the following command to compile the kernel module and user programs:
     ```
     make
     ```

2. **Loading the Kernel Module**:
   - Load the kernel module using the following command (requires sudo/root access):
     ```
     sudo insmod message_slot.ko
     ```

3. **Creating Device Files**:
   - Create device files (`/dev/slot0`, `/dev/slot1`, etc.) using `mknod`. For example:
     ```
     sudo mknod /dev/slot0 c 235 0
     ```
   - Repeat this step to create additional message slots (`/dev/slot1`, `/dev/slot2`, etc.).

### Using the Programs:

4. **Running User Programs**:
   - Execute `message_sender` to send a message to a specific channel of a message slot. Replace `[message_slot_path]`, `[channel_id]`, and `[message]` with appropriate values:
     ```
     ./message_sender /dev/slot0 1 "Hello, World!"
     ```
   - Execute `message_reader` to read a message from a specific channel of a message slot. Replace `[message_slot_path]` and `[channel_id]` with appropriate values:
     ```
     ./message_reader /dev/slot0 1
     ```

5. **Cleaning Up**:
   - Unload the kernel module when done using:
     ```
     sudo rmmod message_slot
     ```
   - Clean up compiled binaries and object files with:
     ```
     make clean
     ```

## (This assignment is part of the Operating Systems course at Tel Aviv University).
