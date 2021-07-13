# Shared Memory from PRU to ARM 

This project shows a way to sample the ADC of the Beaglbone Black with the PRU and to send the samples back to the ARM using the PRU SHARED Memory. 


## How to use the codes
This project was tested under: __XXXX__ there are newer images available but this images is at the time of the beginning of the project the one recommanded on the bealgbone.org website.
The different steps to use the project on a Beaglebone Black are:
1. Clone the project on the debian directory
2. Move the resource table from the includes to the  __XXXX__ directory
3. Compile the pru codes from the `pru_codes` directory with `make`
4. Do the same in the user_space directory 
5. Still in the user_space directory start the user space execution with: `


## Structure of the Project 

The folder contains 

### 



## TODOs:

This is a work in progress so please bear with me if things are not perfect. I will be happy to hear any feed back and to  keep improving things.
The current toddos for this project are:
- [x] validate simple rpmsg example between the arm and the PRUs for communication only (no ADC in this example
- [x] send notification back to arm 
- [x] Create a simple working example to send ADC sample back to the ARM
- [x] write ADC samples to PRU Shared memory from PRU
- [ ] update comments between Beaglbone AI and Beaglebone Black in the codes
- [ ] Complete _How to use the codes_ section of the main readme
- [ ] Complete _Structure of the project_ section of the main readme

## Misc Notes:
remoteproc in makefile does for PRU1 point to remoteproc2 which is for the M3 cortex, this is potentially an issue with the copy paste made from the Beaglebone repository so there maybe an issue with that ! the PRU dire should point to remoteproc0 for PRU0 and to remoteproc1 for pru1. 


