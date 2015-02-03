/* Beaglebone Pru IO 
 * 
 * Copyright (C) 2015 Rafael Vega <rvega@elsoftwarehamuerto.org> 
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "registers.h"
#include "beaglebone_pruio_pins.h"

/////////////////////////////////////////////////////////////////////
// UTIL
//

#define HWREG(x) (*((volatile unsigned int *)(x)))

/////////////////////////////////////////////////////////////////////
// DECLARATIONS
//

volatile unsigned int* shared_ram;
volatile register unsigned int __R31;

/////////////////////////////////////////////////////////////////////
// RING BUFFER
//
   // Communication with ARM processor is througn a ring buffer in the
   // PRU shared memory area.
   // shared_ram[0] to shared_ram[1023] is the buffer data.
   // shared_ram[1024] is the start (read) pointer.
   // shared_ram[1025] is the end (write) pointer.
   //
   // Messages are 32 bit unsigned integers:
   //
   // A GPIO Message:
   // 0XXX XXXX XXXX XXXX XXXX XXXV NNNN NNNN
   // |             |             |      |
   // |             |             |      |-- 7-0: GPIO number
   // |             |             |
   // |             |             |-- 8: Value
   // |             |         
   // |             |-- 30-9: Unused
   // |
   // |-- 31: Always 0, indicates this is a gpio message
   //
   //
   //
   // An ADC Message:
   // 1XXX XXXX XXXX XXXX VVVV VVVV VVVV NNNN
   // |             |             |       |
   // |             |             |       |-- 3-0: ADC Channel
   // |             |             |
   // |             |             |-- 4-15: Value
   // |             |         
   // |             |-- 30-16: Unused
   // |
   // |-- 31: Always 1, indicates this is an adc message
   //
   // Read these to understand the ring buffer:
   // * http://en.wikipedia.org/wiki/Circular_buffer#Use_a_Fill_Count
   // * https://groups.google.com/forum/#!category-topic/beagleboard/F9JI8_vQ-mE

unsigned int buffer_size;
volatile unsigned int *buffer_start;
volatile unsigned int *buffer_end;

void init_buffer(){
   // data in shared_ram[0] to shared_ram[127]
   buffer_size = 1024; 
   buffer_start = &(shared_ram[1024]);
   buffer_end = &(shared_ram[1025]);
   *buffer_start = 0;
   *buffer_end = 0;
}

inline void buffer_write(unsigned int *message){
   // Note that if buffer is full, messages will be dropped
   unsigned int is_full = (*buffer_end == (*buffer_start^buffer_size)); // ^ is orex
   if(!is_full){
      shared_ram[*buffer_end & (buffer_size-1)] = *message;
      // Increment buffer end, wrap around 2*size
      *buffer_end = (*buffer_end+1) & (2*buffer_size - 1);
   }
}

/////////////////////////////////////////////////////////////////////
// GPIO and Mux Control
//

void init_gpio(){
   // TODO: * measure if hardware debounce mechanism adds latency.
   
   // See BeagleboneBlackP9HeaderTable.pdf from derekmolloy.ie
   // Way easier to read than TI's manual

   // Enable GPIO0 Module.
   HWREG(GPIO0 + GPIO_CTRL) = 0x00;
   // Enable clock for GPIO0 module. 
   HWREG(CM_WKUP + CM_WKUP_GPIO0_CLKCTRL) = (0x02) | (1<<18);
   // Set debounce time for GPIO0 module
   // time = (DEBOUNCINGTIME + 1) * 31uSec
   HWREG(GPIO0 + GPIO_DEBOUNCINGTIME) = 255;

   // Enable GPIO1 Module.
   HWREG(GPIO1 + GPIO_CTRL) = 0x00;
   // Enable clock for GPIO1 module. 
   HWREG(CM_PER + CM_PER_GPIO1_CLKCTRL) = (0x02) | (1<<18);
   // Set debounce time for GPIO1 module
   // time = (DEBOUNCINGTIME + 1) * 31uSec
   /* HWREG(GPIO1 + GPIO_DEBOUNCINGTIME) = 255; */
   
   // Enable GPIO2 Module.
   HWREG(GPIO2 + GPIO_CTRL) = 0x00;
   // Enable clock for GPIO2 module. 
   HWREG(CM_PER + CM_PER_GPIO2_CLKCTRL) = (0x02) | (1<<18);
   // Set debounce time for GPIO2 module
   // time = (DEBOUNCINGTIME + 1) * 31uSec
   /* HWREG(GPIO2 + GPIO_DEBOUNCINGTIME) = 255; */
   
   // Enable GPIO3 Module.
   HWREG(GPIO3 + GPIO_CTRL) = 0x00;
   // Enable clock for GPIO3 module. 
   HWREG(CM_PER + CM_PER_GPIO3_CLKCTRL) = (0x02) | (1<<18);
   // Set debounce time for GPIO3 module
   // time = (DEBOUNCINGTIME + 1) * 31uSec
   /* HWREG(GPIO3 + GPIO_DEBOUNCINGTIME) = 255; */
}

inline void set_mux_control(unsigned int ctl){
   // 3 bits for mux control: 
   // [P9_27 GPIO3[19], P9_30 GPIO3[16], P9_42A GPIO0[7]]
   switch(ctl){
      case 0:
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<7);
         break;

      case 1:
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<7);
         break;

      case 2:
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<7);
         break;

      case 3:
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<7);
         break;

      case 4:
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<7);
         break;

      case 5:
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) &= ~(1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<7);
         break;

      case 6:
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<7);
         break;

      default: //7
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<19);
         HWREG(GPIO3 + GPIO_DATAOUT) |= (1<<16);
         HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<7);
         break;
   }
}

inline char * get_gpio_module_address(int module_number){
   int r;
   switch(module_number) {
      case 0:
         r = GPIO0; 
         break;
      case 1:
         r = GPIO1; 
         break;
      case 2:
         r = GPIO2; 
         break;
      case 3:
         r = GPIO3; 
         break;
   }
   return (char *)r;
}

/////////////////////////////////////////////////////////////////////
// TIMER
//
void init_iep_timer(){
   // We'll count 83.333 micro seconds with compare0 register and 45uSec
   // with compare1 register.
   // clock is 200MHz, use increment value of 5, 
   // compare values are then 83333 and 45000

   // 1. Initialize timer to known state
   // 1.1 Disable timer counter
   HWREG(IEP + IEP_TMR_GLB_CFG) &= ~(1); 
   // 1.2 Reset counter (write 1 to clear)
   HWREG(IEP + IEP_TMR_CNT) = 0xffffffff; 
   // 1.3 Clear overflow status
   HWREG(IEP + IEP_TMR_GLB_STS) = 0; 
   // 1.4Clear compare status (write 1 to clear)
   HWREG(IEP + IEP_TMR_CMP_STS) = 0xf; 


   // 2. Set compare values 
   HWREG(IEP + IEP_TMR_CMP0) = 83333; 
   // 2.1 Compare register 1 to 45000
   /* HWREG(IEP + IEP_TMR_CMP1) = 45000; // Used when debugging timing */ 

   // 3. Enable compare events and reset counter when 
   // compare 0 event happens
   HWREG(IEP + IEP_TMR_CMP_CFG) = (1 << 1) | 1; // Compare event 0 only
   /* HWREG(IEP + IEP_TMR_CMP_CFG) = (1 << 2) | (1 << 1) | 1; // Compare evts 0 and 1 */ 
   
   // 4. Set increment value (5)
   HWREG(IEP + IEP_TMR_GLB_CFG) |= 5<<4; 

   // 5. Set compensation value (not needed now)
   HWREG(IEP + IEP_TMR_COMPEN) = 0; 
   
   // 6. Enable counter
   HWREG(IEP + IEP_TMR_GLB_CFG) |= 1; 
}

inline void wait_for_timer(){
   // Wait for compare 0 status to go high
   while((HWREG(IEP+IEP_TMR_CMP_STS) & 1) == 0){
      // nothing 
   }

   // Clear compare 0 status (write 1)
   HWREG(IEP+IEP_TMR_CMP_STS) |= 1;
}

inline void wait_for_short_timer(){
   // Wait for compare 1 status to go high
   while((HWREG(IEP+IEP_TMR_CMP_STS) & (1<<1)) == 0){
      // nothing 
   }

   // Clear compare 1 status (write 1)
   HWREG(IEP+IEP_TMR_CMP_STS) |= (1<<1);
}

/////////////////////////////////////////////////////////////////////
// Analog Digital Conversion
//
inline void wait_for_adc(){
   // Wait for irqstatus[1] to go high
   while((HWREG(ADC_TSC + ADC_TSC_IRQSTATUS) & (1<<1)) == 0){
      // nothing 
   }

   // Clear status (write 1)
   HWREG(ADC_TSC + ADC_TSC_IRQSTATUS) |= (1<<1);
}

inline void adc_start_sampling(){
   // Enable steps 1 to 7
   HWREG(ADC_TSC + ADC_TSC_STEPENABLE) = 0xfe;
   /* HWREG(ADC_TSC + ADC_TSC_STEPENABLE) = 0b111110; // 1 to 5 */
}

void init_adc(){
   // Enable clock for adc module.
   HWREG(CM_WKUP + CM_WKUP_ADC_TSK_CLKCTL) = 0x02;

   // Disable ADC module temporarily.
   HWREG(ADC_TSC + ADC_TSC_CTRL) &= ~(0x01);

   // To calculate sample rate:
   // fs = 24MHz / (CLK_DIV*2*Channels*(OpenDly+Average*(14+SampleDly)))
   // We want 48KHz. (Compromising to 50KHz)
   unsigned int clock_divider = 1;
   unsigned int open_delay = 0;
   unsigned int average = 0;       // can be 0 (no average), 1 (2 samples), 
                                   // 2 (4 samples),  3 (8 samples) 
                                   // or 4 (16 samples)
   unsigned int sample_delay = 0;

   // Set clock divider (set register to desired value minus one). 
   HWREG(ADC_TSC + ADC_TSC_CLKDIV) = clock_divider - 1;

   // Set values range from 0 to FFF.
   HWREG(ADC_TSC + ADC_TSC_ADCRANGE) = (0xfff << 16) & (0x000);

   // Disable all steps. STEPENABLE register
   HWREG(ADC_TSC + ADC_TSC_STEPENABLE) &= ~(0xff);

   // Unlock step config register.
   HWREG(ADC_TSC + ADC_TSC_CTRL) |= (1 << 2);

   // Set config and delays for step 1: 
   // Sw mode, one shot mode, fifo0, channel 0.
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG1) = 0 | (0<<26) | (0<<19) | (0<<15) | (average<<2) | (0);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY1)  = 0 | (sample_delay - 1)<<24 | open_delay;

   // Set config and delays for step 2: 
   // Sw mode, one shot mode, fifo0, channel 1.
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG2) = 0 | (0x0<<26) | (0x01<<19) | (0x01<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY2)  = 0 | (sample_delay - 1)<<24 | open_delay;

   // Set config and delays for step 3: 
   // Sw mode, one shot mode, fifo0, channel 2.
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG3) = 0 | (0x0<<26) | (0x02<<19) | (0x02<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY3)  = 0 | ((sample_delay - 1)<<24) | open_delay;

   // Set config and delays for step 4: 
   // Sw mode, one shot mode, fifo0, channel 3.
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG4) = 0 | (0x0<<26) | (0x03<<19) | (0x03<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY4)  = 0 | ((sample_delay - 1)<<24) | open_delay;

   // Set config and delays for step 5: 
   // Sw mode, one shot mode, fifo0, channel 4.
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG5) = 0 | (0x0<<26) | (0x04<<19) | (0x04<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY5)  = 0 | ((sample_delay - 1)<<24) | open_delay;

   // Set config and delays for step 6: 
   // Sw mode, one shot mode, fifo0, CHANNEL 6!
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG6) = 0 | (0x0<<26) | (0x06<<19) | (0x06<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY6)  = 0 | ((sample_delay - 1)<<24) | open_delay;

   // Set config and delays for step 7: 
   // Sw mode, one shot mode, fifo0, CHANNEL 5!
   HWREG(ADC_TSC + ADC_TSC_STEPCONFIG7) = 0 | (0x0<<26) | (0x05<<19) | (0x05<<15) | (average<<2) | (0x00);
   HWREG(ADC_TSC + ADC_TSC_STEPDELAY7)  = 0 | ((sample_delay - 1)<<24) | open_delay;

   // Enable tag channel id. Samples in fifo will have channel id bits ADC_CTRL register
   HWREG(ADC_TSC + ADC_TSC_CTRL) |= (1 << 1);

   // Clear End_of_sequence interrupt
   HWREG(ADC_TSC + ADC_TSC_IRQSTATUS) |= (1<<1);

   // Enable End_of_sequence interrupt
   HWREG(ADC_TSC + ADC_TSC_IRQENABLE_SET) |= (1 << 1);
   
   // Lock step config register. ACD_CTRL register
   HWREG(ADC_TSC + ADC_TSC_CTRL) &= ~(1 << 2);
   
   // Clear FIFO0 by reading from it.
   unsigned int count = HWREG(ADC_TSC + ADC_TSC_FIFO0COUNT);
   unsigned int data, i;
   for(i=0; i<count; i++){
      data = HWREG(ADC_TSC + ADC_TSC_FIFO0DATA);
   }

   // Clear FIFO1 by reading from it.
   count = HWREG(ADC_TSC + ADC_TSC_FIFO1COUNT);
   for(i=0; i<count; i++){
      data = HWREG(ADC_TSC + ADC_TSC_FIFO1DATA);
   }
   shared_ram[500] = data; // just remove unused value warning;

   // Enable ADC Module. ADC_CTRL register
   HWREG(ADC_TSC + ADC_TSC_CTRL) |= 1;
}

/////////////////////////////////////////////////////////////////////
// ANALYZE ADC VALUES 
//

typedef struct adc_channel{
   int mode; 
   unsigned int value; 
   unsigned int past_values[8]; 
} adc_channel;

adc_channel adc_channels[BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS];

unsigned int mux_control;

inline void process_adc_value(unsigned int channel_number, unsigned int value){
   // Channel is in ON mode. Send new value if it's different to the old one.
   if(adc_channels[channel_number].mode == 1){
      unsigned int i, average;
      unsigned int message;

      value = value >> 5;

      // Channels 0 to 5 are sampled 8 times faster than channels 6 to 13.
      // Calculate 8 times average. mux_control counter is re-used to count 
      // the 8 samples.
      if(channel_number < 6){
         adc_channels[channel_number].past_values[mux_control] = value;
         if(mux_control==7){
            average = 0;
            for(i=0; i<8; i++) {
               average += adc_channels[channel_number].past_values[i];
            }
            average = average >> 3;  // Integer division by 8.

            // Send the value to ARM. See message format in comments 
            // in ring buffer section below
            if(adc_channels[channel_number].value != average){
               adc_channels[channel_number].value = average;
               message = ((unsigned int)1<<31) | (average<<4) | (channel_number);
               buffer_write(&message);
            }
         }
      }
      else{ // channels 6 to 13
         // Send the value to ARM. See message format in comments 
         // in ring buffer section below
         if(adc_channels[channel_number].value != value){
            adc_channels[channel_number].value = value;
            message = ((unsigned int)1<<31) | (value<<4) | (channel_number);
            buffer_write(&message);
         }
      }
   }
}

// Read available samples from fifo0 in blocks of seven, figure out
// which channel they belong to (using step id and mux control) and
// send them to process_adc_value (singular) function.
inline void process_adc_values(){
   unsigned int data, i, step_id, value, channel_number;
   unsigned int count = HWREG(ADC_TSC + ADC_TSC_FIFO0COUNT);
   while(count > 6){
      for(i=0; i<7; i++){
         data = HWREG(ADC_TSC + ADC_TSC_FIFO0DATA);
         step_id = (data & (0x000f0000)) >> 16;
         value = (data & 0xfff);
         if(step_id==5){ // The channel with the mux, channel 6 on adc.
            if(mux_control == 0)
               channel_number = 13;
            else
               channel_number = 6+mux_control-1;
         }
         else if(step_id==6){ // Last step is actually channel 5
            channel_number = 5;
         }
         else{ // Other channels
            channel_number = step_id;
         }
         process_adc_value(channel_number, value);
      }
      count = HWREG(ADC_TSC + ADC_TSC_FIFO0COUNT);
   }
}

void init_adc_values(){
   int i;

   adc_channel new_channel;
   new_channel.mode = 0;
   new_channel.value = 0xFFFF;
   for(i=0; i<8; i++) {
      new_channel.past_values[i] = 0xFFFF;
   }

   for(i=0; i<BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS; i++){
      adc_channels[i] = new_channel; 
   }
}

inline void init_adc_channels(){
   /**
    * Checks if ARM code has requested input from new adc pins (channels) and 
    * adds them to the adc_channels array.
    * Each one of bit in shared_ram[1030] represent the ADC channels. If a bit 
    * is set, it means that the ARM code needs input for that ADC channel. 
    *
    * shared_ram[1030]
    *
    */
   unsigned int config = shared_ram[1030];
   int bit;
   for(bit=0; bit<BEAGLEBONE_PRUIO_MAX_ADC_CHANNELS; ++bit){
      // If bit is set
      if((config&(1<<bit)) != 0){
         adc_channels[bit].mode = 1; // 1 is on
      }
   }
}

/////////////////////////////////////////////////////////////////////
// ANALYZE GPIO VALUES 
//

typedef struct gpio_channel{
   int gpio_number; 
   unsigned int value; 
} gpio_channel;

gpio_channel gpio_channels[BEAGLEBONE_PRUIO_MAX_GPIO_CHANNELS];
int gpio_channel_count=0;

inline void process_gpio_values(){
   int i, module_number, pin;
   unsigned int new_value, message;
   char *module;
   gpio_channel *channel;

   for(i=0; i<gpio_channel_count; i++){
      channel = &gpio_channels[i];
      module_number = channel->gpio_number >> 5; // integer division by 32
      pin = channel->gpio_number % 32;
      module = get_gpio_module_address(module_number);
      new_value = ((HWREG(module+GPIO_DATAIN)&(1<<pin)) != 0);

      if(channel->value != new_value){
         // See message format explanation in comments in ring buffer section
         message = (0<<31) | (new_value<<8) | channel->gpio_number; 
         buffer_write(&message);

         channel->value = new_value;
      }
   }
}

void init_gpio_values(){
   gpio_channel_count = 0;
}

inline void init_gpio_channel(int gpio_module, int bit){
   int gpio_number = gpio_module*32 + bit;

   // Check if channel is already initialized
   int i;
   for(i=0; i<gpio_channel_count; ++i){
      if(gpio_channels[i].gpio_number == gpio_number){
         return;      
      }
   }

   // Add the new channel to the gpio_channels array
   gpio_channel new_channel;
   new_channel.value = 2;
   new_channel.gpio_number = gpio_number;
   gpio_channels[gpio_channel_count] = new_channel;
   gpio_channel_count++;
}

inline void init_gpio_channels(){
   /**
    * Checks if ARM code has requested input from new gpio pins (channels) and 
    * adds them to the gpio_channels array.
    *
    * Each one of the 32 bits in shared_ram[1026] represent the 32 pins for
    * the GPIO0 module. If a bit is set, it means that the ARM code needs
    * input for that GPIO channel. Same mechanism is used for GPIO1, 2 and 3
    * using the following shared ram positions:
    *
    * shared_ram[1026] -> GPIO0
    * shared_ram[1027] -> GPIO1
    * shared_ram[1028] -> GPIO2
    * shared_ram[1029] -> GPIO3
    *
    */
   int gpio_module;
   for(gpio_module=0; gpio_module<4; ++gpio_module){
      unsigned int config = shared_ram[1026+gpio_module];
      int bit;
      for(bit=0; bit<32; ++bit){
         // If bit is set
         if((config&(1<<bit)) != 0){
            init_gpio_channel(gpio_module, bit); 
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////
// MAIN
//

void init_ocp(){
   // Enable OCP so we can access the whole memory map for the
   // device from the PRU. Clear bit 4 of SYSCFG register
   HWREG(PRU_ICSS_CFG + PRU_ICSS_CFG_SYSCFG) &= ~(1 << 4);

   // Pointer to shared memory region
   shared_ram = (volatile unsigned int *)0x10000;
}

int main(int argc, const char *argv[]){
   init_ocp();
   init_buffer();
   init_adc();
   init_adc_values();
   init_gpio();
   init_gpio_values();
   init_iep_timer();

   // Debug:
   /* unsigned int i; */

   mux_control = 7;

   unsigned int finished = 0;

   // TODO: exit condition
   while(!finished){
      mux_control>6 ? mux_control=0 : mux_control++;
      set_mux_control(mux_control);
      adc_start_sampling();
      
      process_adc_values();
      process_gpio_values();

      init_gpio_channels();
      init_adc_channels();

      // Debug:
      /* HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<30); */
      /* for(i=0; i<20; i++);  */
      /* HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<30); */

      // Debug:
      /* wait_for_short_timer(); */
      /* HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<30); */
      /* for(i=0; i<20; i++);  */
      /* HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<30); */

      // Debug:
      /* wait_for_adc(); */
      /* HWREG(GPIO0 + GPIO_DATAOUT) |= (1<<30); */
      /* for(i=0; i<20; i++);  */
      /* HWREG(GPIO0 + GPIO_DATAOUT) &= ~(1<<30); */

      wait_for_timer(); // Timer resets itself after this
   }

   __halt();
   return 0;
}

