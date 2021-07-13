/*
 * simple_ping_pong_user_space.pru1_1.c
 * Modified by Pierrick Rauby < PierrickRauby - pierrick.rauby@gmail.com >
 * Based on the examples distributed by TI
 * redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the name of Texas Instruments Incorporated nor the names of
 *      its contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 * To compile use: make 
 * To deploy use: sh ./run.sh
 */



#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define MAX_BUFFER_SIZE        512
#define PRU_DATA_ADDRESS 0x4A300000
char readBuf[MAX_BUFFER_SIZE];
#define DEVICE_NAME        "/dev/rpmsg_pru30"

static void delay(int number_of_dixieme_de_seconde);

int main(void)
{
  struct pollfd pollfds[1];
  int result = 0;
  int fd, i, j;
  void *map_base, *virt_addr;
  unsigned long read_result, writeval;
  unsigned int numberOutputSamples = 1;
  off_t target = PRU_DATA_ADDRESS;

  /* Open the rpmsg_pru character device file */
  pollfds[0].fd = open(DEVICE_NAME, O_RDWR);

  /*
   * If the RPMsg channel doesn't exist yet the character device
   * won't either.
   * Make sure the PRU firmware is loaded and that the rpmsg_pru
   * module is inserted.
   */
  if (pollfds[0].fd < 0) {
    printf("Failed to open %s\n", DEVICE_NAME);
    return -1;
  }

  /* The RPMsg channel exists and the character device is opened */
  printf("Opened %s, sending address to read to PRU\n", DEVICE_NAME);
  /* In this example only the adress to read with the PRU is sent*/
  result = write(pollfds[0].fd, "hello PRU!", 10);
  if (result > 0)
    printf("Waiting for PRU answer through character device %s\n", DEVICE_NAME);

  /*close(pollfds[0].fd);*/
  delay(4);
  printf("done\n");

  /* Poll until we receive a message from the PRU and then print it */
  result = read(pollfds[0].fd, readBuf, sizeof(int*));
  if (result > 0){
    printf("Sample is ready to read in PRU memory, \n Reading Data points\n",
        readBuf);
  }
  /* Close the rpmsg_pru character device file */
  close(pollfds[0].fd);

  if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1){
    printf("Failed to open memory!\n");
    return -1;
  }
  fflush(stdout);

  map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
  if(map_base == (void *) -1) {
    printf("Failed to map base address\n");
    return -1;
  }
  fflush(stdout);

  for(j=0; j<1000; j++){
    for(i=0; i<numberOutputSamples; i++){
      virt_addr = map_base + (target & MAP_MASK);
      read_result = *((unsigned long *) virt_addr);
      //printf("Value at address 0x%X is: 0x%X\n", target, read_result);
      displayDistance((unsigned int)read_result);
      usleep(500000);
    }
    fflush(stdout);
  }

  if(munmap(map_base, MAP_SIZE) == -1) {
    printf("Failed to unmap memory");
    return -1;
  }
  close(fd);
  return 0;



  return 0;
}


void delay(int number_of_dixieme_de_seconde) 
{ 
  // Converting time into milli_seconds
  int milli_seconds = 100000 * number_of_dixieme_de_seconde;
  // Storing start time
  clock_t start_time = clock();
  // looping till required time is not achieved
  while (clock() < start_time + milli_seconds);
}
