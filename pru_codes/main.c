/*
 * main.c
 * Modified by Pierrick Rauby < PierrickRauby - pierrick.rauby@gmail.com >
 * Based on the cloud9 examples:
 * https://github.com/jadonk/cloud9-examples/blob/master/BeagleBone/
 *       AI/pru/blinkInternalLED.pru1_1.c
 * To compile use: make 
 */

#include <stdint.h>
#include <pru_cfg.h>
#include "resource_table_1.h"
/*#include "prugpio.h"*/
#include <stdio.h>
#include <stdlib.h>            // atoi
#include <string.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>
#include <pru_ctrl.h>
#include "hw_types.h"
#include "tsc_adc.h"
#include "soc_AM335x.h"
#include "hw_control_AM335x.h"
#include "hw_cm_per.h"
#include "hw_cm_wkup.h"


int pru_function(uint8_t i2cDevice);
uint32_t get_sample();
static void ADCConfigure(void);

volatile register unsigned int __R30;
volatile register unsigned int __R31;

/* Host-0 Interrupt sets bit 30 in register R31 */
#define HOST_INT            ((uint32_t) 1 << 31)

/* The PRU-ICSS system events used for RPMsg are defined in the Linux 
   device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 * Be sure to change the values in resource_table_0.h too.
 */
#define TO_ARM_HOST          18
#define FROM_ARM_HOST        19
#define CHAN_NAME            "rpmsg-pru"
#define CHAN_DESC            "Channel 30"
#define CHAN_PORT            30
/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK    4


#define PRU_DMEM0 __far __attribute__((cregister("PRU_DMEM_0_1",  near)))
/*#define PRU_DMEM1 __far __attribute__((cregister("PRU_DMEM_1_0",  near)))*/
#define NUMBER_SAMPLES 1024
PRU_DMEM0 volatile uint32_t pru_mem_array[NUMBER_SAMPLES];
char payload[RPMSG_BUF_SIZE];
struct pru_rpmsg_transport transport;
uint16_t src, dst, len;
volatile uint8_t *status;
/*unsigned long sample;*/
int i;

void main(void) {
  ADCConfigure();
  pru_function(1);
}

uint32_t get_sample(){
  uint32_t sample;
  unsigned int i, count, data ;
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPENABLE) = 0xfe;
  /* Wait for interrupt */
  while (!(HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_IRQSTATUS)&0x02));
  /* Clear interrupt */
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_IRQSTATUS) = 0x02;
  sample = 0xFFFFFFFF;
  count = HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_FIFOCOUNT(0));
  //sample = count;
  for (i = 0; i < count; i++) {
    data = HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_FIFODATA(0));
    if ((data & 0x000F0000) == 0) {
      /* if channel == 0 */
      sample = data & 0xFFF;
    }
  }
  return sample;
}

static void ADCConfigure(void)
{
  unsigned int i, count ;
  /* Enable ADC module clock */
  HWREG(SOC_CM_WKUP_REGS + CM_WKUP_ADC_TSC_CLKCTRL) = 0x02;
  /* Disable ADC module for configuration */
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_CTRL) &= ~0x01;
  /* fs = 24MHz / ((CLKDIV+1)*2*Channels*(OpenDly+Average*(14+SampleDly)))
   *    = 53.57kHz
   * CLKDIV = 0
   * Channels = 1
   * Average = 16
   * OpenDly = 0
   * SampleDly = 0
   */
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_ADC_CLKDIV) = 0;

  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_ADCRANGE) = 0xFFF << 16;

  /* Disable all steps for now */
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPENABLE) &= 0xFF;

  /* Unlock step configuration */
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_CTRL) |= 0x04;

  /* Step 1 config: SW mode, one shot mode, fifo 0, channel 0 */
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(0)) = 0x00000000;
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPDELAY(0)) = 0xFF000000;

  /* Enable channel ID tag */
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_CTRL) |= 0x02;

  /* Clear end-of-sequence interrupt */
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_IRQSTATUS) = 0x02;

  /* Enable end-of-sequence interrupt */
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_IRQENABLE_SET) = 0x02;

  /* Lock step configuration */
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_CTRL) &= ~0x04;

  /* Empty FIFO 0 */
  count = HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_FIFOCOUNT(0));
  for (i = 0; i < count; i++) {
     HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_FIFODATA(0));
  }

  /* Enable ADC module */
  HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_CTRL) |= 0x01;
}

int pru_function(uint8_t i2cDevice)
{
  /* Allow OCP master port access by the PRU so the PRU can read external 
     memories */
  CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;
  /* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
  CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
  /* Make sure the Linux drivers are ready for RPMsg communication */
  status = &resourceTable.rpmsg_vdev.status;
  while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));
  /* Initialize the RPMsg transport structure */
  pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0,
      &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);
  /* Create the RPMsg channel between the PRU and ARM user space using the 
     transport structure. */
  while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, 
        CHAN_PORT) != PRU_RPMSG_SUCCESS);
  while (1) {
    /* Check bit 30 of register R31 to see if the ARM has kicked us */

    if (__R31 & HOST_INT) {
      /* Clear the event status */
      CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
      while (pru_rpmsg_receive(&transport, &src, &dst, payload,
            (uint16_t*)sizeof(int*)) == PRU_RPMSG_SUCCESS) {
        for(i=0;i<NUMBER_SAMPLES;i++){
          pru_mem_array[i]=0; // clear the memory 
          pru_mem_array[i]=get_sample(); // write the sample
        }
        pru_rpmsg_send(&transport, dst, src,"writen" ,6); /*pru_rpmsg_send(&transport, dst, src, payload, 4);*/
      }
    }
    }
}

  // Turns off triggers
#pragma DATA_SECTION(init_pins, ".init_pins")
#pragma RETAIN(init_pins)
  const char init_pins[] =  
    "/sys/class/leds/beaglebone:green:usr1/trigger\0none\0" \
    "/sys/class/leds/beaglebone:green:usr2/trigger\0none\0" \
    "/sys/bus/platform/drivers/ti_am3359-tscadc/unbind\00044e0d000.tscadc\0" \
    "\0\0";
