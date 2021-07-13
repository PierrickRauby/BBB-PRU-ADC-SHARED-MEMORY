# Shared Memory from PRU to ARM 

This project shows a way to sample the ADC of the Beaglbone Black with the PRU and to send the samples back to the ARM using the PRU SHARED Memory. The project uses RPMSG for the communication between the Host ARM and the PRU cores.


## How to use the codes
This project was tested under: __4.19.94-ti-r42__ there are newer images available but this images is at the time of the beginning of the project the one recommanded on the beaglebone.org website.
The different steps to use the project on a Beaglebone Black are:
1. Clone the project on the debian directory
2. Compile the pru codes from the `pru_codes` directory with `make`. This will compile the PRU codes, place the firmware and start the PRU. The PRU will be then waiting to Recive a message from the ARM to start the ADC sampling
3. Change to root user by doing `sudo su` (the default password for debian is tmppwd), this will let you read `/dev/mem` .
3. In the `user_space` directory do `make`. This will compile the user space application and execute it. You should see the printout of the data in the console after that.
The ADC is sampling its channel 1, this is  __P9.39__  on the Beaglebone Black, be carefull, the ADC of the beaglebone black only support 1.8V so don't fry your board. 

## Structure of the Project 

The repo contains 2 directories, `pru_codes` and `user_space`.

### [pru_codes](pru_codes/)
Contains the codes for the PRU, makefile and some includes. This codes can be compile, placed and run using `make` from inside the directory. 
The main.c will configure the ADC and wait for a message to arrive on the rpmsg channel from the ARM. Once the PRU receives the rmpsg message, it will start the acquisition of 1024 Data points and place them in it is DATA Memory. After the acquisition the PRU will respond to the ARM to notify that the ARM can read the DATA from the PRU memory.

### [user_space](user_space/)
Contains the ARM side codes and the make file. The code can requires root privilegies and can be compiled and run using `make`.
The code will initiate the rpmsg transport structure and send a message to the PRU, then it will wait for the PRU to answer and read the DATA from the PRU memory. The sample points are printout by the code but the code can be easily modified.


## TODOs:

This is a work in progress so please bear with me if things are not perfect. I will be happy to hear any feed back and to  keep improving things.
The current toddos for this project are:
- [x] validate simple rpmsg example between the arm and the PRUs for communication only (no ADC in this example
- [x] send notification back to arm 
- [x] Create a simple working example to send ADC sample back to the ARM
- [x] write ADC samples to PRU Shared memory from PRU
- [x] update comments between Beaglbone AI and Beaglebone Black in the codes
- [x] Complete _How to use the codes_ section of the main readme
- [x] Complete _Structure of the project_ section of the main readme
