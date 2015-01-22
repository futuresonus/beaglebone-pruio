#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "bbb_pruio.h"
#include "registers.h"
#include "bbb_pruio_pins.h"

#ifndef BBB_PRUIO_START_ADDR_0
   #error "BBB_PRUIO_START_ADDR_0 must be defined."
#endif

#ifndef BBB_PRUIO_PREFIX
   #error "BBB_PRUIO_PREFIX must be defined."
#endif

/* #define DEBUG */


/////////////////////////////////////////////////////////////////////
// MEMORY MAP

volatile unsigned int* gpio0_output_enable = NULL;
volatile unsigned int* gpio1_output_enable = NULL;
volatile unsigned int* gpio2_output_enable = NULL;
volatile unsigned int* gpio3_output_enable = NULL;

volatile unsigned int* gpio0_data_out = NULL;
volatile unsigned int* gpio1_data_out = NULL;
volatile unsigned int* gpio2_data_out = NULL;
volatile unsigned int* gpio3_data_out = NULL;

static int map_device_registers(){
   // Get pointers to hardware registers. See memory map in manual for addresses.
   
   int memdev = open("/dev/mem", O_RDWR | O_SYNC);
   
   // Get pointer to gpio0 registers (start at address 0x44e07000, length 0x1000 (4KB)).
   volatile void* gpio0 = mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, GPIO0);
   if(gpio0 == MAP_FAILED){ return 1; }
   gpio0_output_enable = (volatile unsigned int*)(gpio0 + GPIO_OE);
   gpio0_data_out = (volatile unsigned int*)(gpio0 + GPIO_DATAOUT);

   // same for gpio1, 2 and 3.
   volatile void* gpio1 = mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, GPIO1);
   if(gpio1 == MAP_FAILED){
      return 1;
   }
   gpio1_output_enable = (volatile unsigned int*)(gpio1 + GPIO_OE);
   gpio1_data_out = (volatile unsigned int*)(gpio1 + GPIO_DATAOUT);

   volatile void* gpio2 = mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, GPIO2);
   if(gpio2 == MAP_FAILED){
      return 1;
   }
   gpio2_output_enable = (volatile unsigned int*)(gpio2 + GPIO_OE);
   gpio2_data_out = (volatile unsigned int*)(gpio2 + GPIO_DATAOUT);

   volatile void* gpio3 = mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, GPIO3);
   if(gpio3 == MAP_FAILED){
      return 1;
   }
   gpio3_output_enable = (volatile unsigned int*)(gpio3 + GPIO_OE);
   gpio3_data_out = (volatile unsigned int*)(gpio3 + GPIO_DATAOUT);

   return 0;
}

/////////////////////////////////////////////////////////////////////
// GPIO PINS

typedef struct gpio_pin{ 
   bbb_pruio_gpio_mode mode;
   int gpio_number;
} gpio_pin;
static gpio_pin used_pins[BBB_PRUIO_MAX_GPIO_CHANNELS];
static int used_pins_count = 0;

static int init_gpio(){
   // Only pinmux is set here, enabling GPIO modules, 
   // clocks, debounce, etc. is set on the PRU side.
   
   // Pins used to control analog mux:
   int e1 = bbb_pruio_init_gpio_pin(P9_27, BBB_PRUIO_OUTPUT_MODE);
   int e2 = bbb_pruio_init_gpio_pin(P9_30, BBB_PRUIO_OUTPUT_MODE);
   int e3 = bbb_pruio_init_gpio_pin(P9_42A, BBB_PRUIO_OUTPUT_MODE);

   if(e1 || e2 || e3){
      return 1;
   } 
   else{
      return 0;
   }
}

static int get_gpio_pin_name(int gpio_number, char* pin_name){
   switch(gpio_number){
      case P9_11: 
         strcpy(pin_name, "P9_11");
         break;
      case P9_12: 
         strcpy(pin_name, "P9_12");
         break;
      case P9_13: 
         strcpy(pin_name, "P9_13");
         break;
      case P9_14: 
         strcpy(pin_name, "P9_14");
         break;
      case P9_15: 
         strcpy(pin_name, "P9_15");
         break;
      case P9_16: 
         strcpy(pin_name, "P9_16");
         break;
      case P9_17: 
         strcpy(pin_name, "P9_17");
         break;
      case P9_18: 
         strcpy(pin_name, "P9_18");
         break;
      case P9_21: 
         strcpy(pin_name, "P9_21");
         break;
      case P9_22: 
         strcpy(pin_name, "P9_22");
         break;
      case P9_23: 
         strcpy(pin_name, "P9_23");
         break;
      case P9_24: 
         strcpy(pin_name, "P9_24");
         break;
      case P9_26: 
         strcpy(pin_name, "P9_26");
         break;
      case P9_27: 
         strcpy(pin_name, "P9_27");
         break;
      case P9_30: 
         strcpy(pin_name, "P9_30");
         break;
      case P9_41A: 
         strcpy(pin_name, "P9_41A");
         break;
      case P9_41B: 
         strcpy(pin_name, "P9_41B");
         break;
      case P9_42A: 
         strcpy(pin_name, "P9_42A");
         break;
      case P9_42B: 
         strcpy(pin_name, "P9_42B");
         break;
      default: 
         return 1;
         break;
   }
   return 0;
}

static int get_gpio_config_file(int gpio_number, char* path){
   char pin_name[256] = "";
   if(get_gpio_pin_name(gpio_number, pin_name)){
      return 1; 
   }

   char tmp[256] = "";

   // look for a path that looks like ocp.* in /sys/devices/
   DIR *dir = opendir("/sys/devices/");
   if(dir==NULL){
      return 1;
   }
   struct dirent* dir_info;
   while(dir){
      dir_info = readdir(dir);
      if(dir_info==NULL){
         closedir(dir);
         return 1;
      }
      // Substring "ocp."
      if(strstr(dir_info->d_name, "ocp.")!=NULL){
         strcat(tmp, "/sys/devices/");
         strcat(tmp, dir_info->d_name);
         strcat(tmp, "/");

         DIR *dir2 = opendir(tmp);
         if(dir2==NULL){
            printf("DD");
            return 1;
         }
         while(dir2){
            dir_info = readdir(dir2);
            if(dir_info==NULL){
               closedir(dir2);
               closedir(dir);
               printf("EE");
               return 1;
            }
            // Substring pin name
            if(strstr(dir_info->d_name, pin_name)!=NULL){
               strcat(tmp, dir_info->d_name);
               strcat(tmp, "/state");
               break; // while dir2
            }
         }
         closedir(dir2);
         break; // while dir1
      }
   }
   closedir(dir);
   strcpy(path, tmp);
   return 0;
}

/////////////////////////////////////////////////////////////////////
// ADC CHANNELS

typedef struct adc_channel{ 
   int channel_number;
} adc_channel;

static adc_channel used_adc_channels[BBB_PRUIO_MAX_ADC_CHANNELS];
static int used_adc_channels_count = 0;


/////////////////////////////////////////////////////////////////////
// PRU Initialization
//

static int load_device_tree_overlay(char* dto){
   // Check if the device tree overlay is loaded, load if needed.
   int device_tree_overlay_loaded = 0; 
   FILE* f;
   f = fopen("/sys/devices/bone_capemgr.9/slots","rt");
   if(f==NULL){
      return 1;
   }
   char line[256];
   while(fgets(line, 256, f) != NULL){
      if(strstr(line, dto) != NULL){
         device_tree_overlay_loaded = 1; 
      }
   }
   fclose(f);

   if(!device_tree_overlay_loaded){
      f = fopen("/sys/devices/bone_capemgr.9/slots","w");
      if(f==NULL){
         return 1;
      }
      fprintf(f, "%s", dto);
      fclose(f);
   }

   return 0;
}

static int load_device_tree_overlays(){
   if(load_device_tree_overlay("PRUIO-DTO")){
      return 1;
   }
   usleep(100000);
   return 0;
}

static int init_pru_system(){
   tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
   if(prussdrv_init()) return 1;
   if(prussdrv_open(PRU_EVTOUT_0)) return 1;
   if(prussdrv_pruintc_init(&pruss_intc_initdata)) return 1;

   // Get pointer to shared ram
   void* p;
   if(prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &p)) return 1;
   bbb_pruio_shared_ram = (volatile unsigned int*)p;

   return 0;
}

static int start_pru0_program(){
   char path[512] = "";
   strcat(path, BBB_PRUIO_PREFIX); 
   strcat(path, "/lib/libbbb_pruio_data0.bin");
   if(prussdrv_load_datafile(0, path)) return 1;

   strcpy(path, BBB_PRUIO_PREFIX); 
   strcat(path, "/lib/libbbb_pruio_text0.bin");
   if(prussdrv_exec_program_at(0, path, BBB_PRUIO_START_ADDR_0)) return 1;

   return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Ring buffer (see header file for more)
//

static void buffer_init(){
   // These shared ram positions control which adc and gpio channels
   // we want to receive data from, see other comments in this file.
   bbb_pruio_shared_ram[1026] = 0;
   bbb_pruio_shared_ram[1027] = 0;
   bbb_pruio_shared_ram[1028] = 0;
   bbb_pruio_shared_ram[1029] = 0;
   bbb_pruio_shared_ram[1030] = 0;

   bbb_pruio_buffer_size = 1024;
   bbb_pruio_buffer_start = &(bbb_pruio_shared_ram[1024]); // value inited to 0 in pru
   bbb_pruio_buffer_end = &(bbb_pruio_shared_ram[1025]); // value inited to 0 in pru
}

/////////////////////////////////////////////////////////////////////
// "Public" functions.
//

int bbb_pruio_start(){
   if(load_device_tree_overlays()){
      fprintf(stderr, "libbbb_pruio: Could not load device tree overlays.\n");
      return 1;
   }

   if(map_device_registers()){
      fprintf(stderr, "libbbb_pruio: Could not map device's registers to memory.\n");
      return 1;
   }

   if(init_pru_system()){
      fprintf(stderr, "libbbb_pruio: Could not init PRU system.\n");
      return 1;
   }

   buffer_init();

   if(start_pru0_program()){
      fprintf(stderr, "libbbb_pruio: Could not load PRU0 program.\n");
      return 1;
   }

   if(init_gpio()){
      fprintf(stderr, "libbbb_pruio: Could not init GPIO.\n");
      return 1;
   }


   return 0;
}

/* int bbb_pruio_get_gpio_number(char* pin_name){ */
/*    if(strcmp(pin_name, "P9_11") == 0){ */
/*       return P9_11; */
/*    } */
/*    else if(strcmp(pin_name, "P9_12") == 0){ */
/*       return P9_12; */
/*    } */
/*    else if(strcmp(pin_name, "P9_13") == 0){ */
/*       return P9_13; */
/*    } */
/*    else if(strcmp(pin_name, "P9_14") == 0){ */
/*       return P9_14; */
/*    } */
/*    else if(strcmp(pin_name, "P9_15") == 0){ */
/*       return P9_15; */
/*    } */
/*    else if(strcmp(pin_name, "P9_16") == 0){ */
/*       return P9_16; */
/*    } */
/*    else if(strcmp(pin_name, "P9_17") == 0){ */
/*       return P9_17; */
/*    } */
/*    else if(strcmp(pin_name, "P9_18") == 0){ */
/*       return P9_18; */
/*    } */
/*    else if(strcmp(pin_name, "P9_21") == 0){ */
/*       return P9_21; */
/*    } */
/*    else if(strcmp(pin_name, "P9_22") == 0){ */
/*       return P9_22; */
/*    } */
/*    else if(strcmp(pin_name, "P9_23") == 0){ */
/*       return P9_23; */
/*    } */
/*    else if(strcmp(pin_name, "P9_24") == 0){ */
/*       return P9_24; */
/*    } */
/*    else if(strcmp(pin_name, "P9_26") == 0){ */
/*       return P9_26; */
/*    } */
/*    else if(strcmp(pin_name, "P9_27") == 0){ */
/*       return P9_27; */
/*    } */
/*    else if(strcmp(pin_name, "P9_30") == 0){ */
/*       return P9_30; */
/*    } */
/*    else if(strcmp(pin_name, "P9_41A") == 0){ */
/*       return P9_41A; */
/*    } */
/*    else if(strcmp(pin_name, "P9_41B") == 0){ */
/*       return P9_41B; */
/*    } */
/*    else if(strcmp(pin_name, "P9_42A") == 0){ */
/*       return P9_42A; */
/*    } */
/*    else if(strcmp(pin_name, "P9_42B") == 0){ */
/*       return P9_42B; */
/*    } */
/*    return -1; */
/* } */

int bbb_pruio_init_adc_pin(int channel_number){
   // Check if channel already in use.
   int i;
   for(i=0; i<used_adc_channels_count; ++i){
      adc_channel channel = used_adc_channels[i];
      if(channel.channel_number==channel_number){
         return 0;
      }
   }

   // Save new channel info;
   adc_channel new_channel;
   new_channel.channel_number = channel_number;
   used_adc_channels[used_adc_channels_count] = new_channel;
   used_adc_channels_count++;

   /** 
    * Tell the PRU unit that we are interested in input from this channel.
    * If a bit is set in shared_ram[1030], it means that we need input 
    * for that ADC channel.
    *
    * shared_ram[1030]
    *
    */
   bbb_pruio_shared_ram[1030] |= (1<<channel_number);
   return 0;
}

int bbb_pruio_init_gpio_pin(int gpio_number, bbb_pruio_gpio_mode mode){
   // Check if pin already in use.
   int i;
   for(i=0; i<used_pins_count; ++i){
      gpio_pin pin = used_pins[i];
      if(pin.gpio_number==gpio_number && pin.mode==mode){
         return 0;
      }
      else if(pin.gpio_number==gpio_number && pin.mode!=mode){
         return 1;
      }
   }

   // Save new pin info;
   gpio_pin new_pin;
   new_pin.gpio_number = gpio_number;
   new_pin.mode = mode;
   used_pins[used_pins_count] = new_pin;
   used_pins_count++;
   
   // Set the pinmux of the pin by writing to the appropriate config file
   char path[256] = "";
   if(get_gpio_config_file(gpio_number, path)){
      return 1;
   }
   FILE *f = fopen(path, "w");
   if(f==NULL){
      return 1;
   }
   int gpio_module = gpio_number >> 5;
   int gpio_bit = gpio_number % 32;
   if(mode == BBB_PRUIO_OUTPUT_MODE){
      fprintf(f, "%s", "output"); 
      // Clear the output enable bit in the gpio config register to actually enable output.
      switch(gpio_module){
         case 0: *gpio0_output_enable &= ~(1<<gpio_bit); break;
         case 1: *gpio1_output_enable &= ~(1<<gpio_bit); break;
         case 2: *gpio2_output_enable &= ~(1<<gpio_bit); break;
         case 3: *gpio3_output_enable &= ~(1<<gpio_bit); break;
      }
   }
   else{
      fprintf(f, "%s", "input"); 
      // Set the output enable bit in the gpio config register to disable output.
      switch(gpio_module){
         case 0: *gpio0_output_enable |= (1<<gpio_bit); break;
         case 1: *gpio1_output_enable |= (1<<gpio_bit); break;
         case 2: *gpio2_output_enable |= (1<<gpio_bit); break;
         case 3: *gpio3_output_enable |= (1<<gpio_bit); break;
      }

      /** 
       * Tell the PRU unit that we are interested in input from this pin.
       *
       * Each one of the 32 bits in shared_ram[1026] represent the 32 pins for
       * the GPIO0 module. If a bit is set, it means that we need
       * input for that GPIO channel. Same mechanism is used for GPIO1, 2 and 3
       * using the following shared ram positions:
       *
       * shared_ram[1026] -> GPIO0
       * shared_ram[1027] -> GPIO1
       * shared_ram[1028] -> GPIO2
       * shared_ram[1029] -> GPIO3
       *
       */
      bbb_pruio_shared_ram[gpio_module+1026] |= (1<<gpio_bit);
 
   }
   fclose(f);
   return 0;
}

void bbb_pruio_set_pin_value(int gpio_number, int value){
   int gpio_module = gpio_number >> 5;
   int gpio_bit = gpio_number % 32;
   volatile unsigned int* reg=NULL;
   switch(gpio_module){
      case 0: reg = gpio0_data_out; break;
      case 1: reg = gpio1_data_out; break;
      case 2: reg = gpio2_data_out; break;
      case 3: reg = gpio3_data_out; break;
   }

   if(value==1){
      *reg |= (1<<gpio_bit);
   }
   else{
      *reg &= ~(1<<gpio_bit);
   }
}

int bbb_pruio_stop(){
   // TODO: send terminate message to PRU

   prussdrv_pru_disable(0);
   prussdrv_exit();

   return 0;
}

