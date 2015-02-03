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

#ifndef REGISTERS_H
#define REGISTERS_H

#include "beaglebone_pruio_pins.h"

/////////////////////////////////////////////////////////////////////
// Register addresses
//

// PRU Module Registers
#define PRU_ICSS_CFG 0x26000
#define PRU_ICSS_CFG_SYSCFG 0x04

// IEP Module Registers
#define IEP 0x2e000
#define IEP_TMR_GLB_CFG 0x00
#define IEP_TMR_GLB_STS 0x04
#define IEP_TMR_COMPEN 0x08
#define IEP_TMR_CNT 0x0C
#define IEP_TMR_CMP_CFG 0x40
#define IEP_TMR_CMP_STS 0x44
#define IEP_TMR_CMP0 0x48
#define IEP_TMR_CMP1 0x4C

// Clock Module registers
#define CM_PER 0x44e00000
#define CM_PER_GPIO1_CLKCTRL 0xac
#define CM_PER_GPIO2_CLKCTRL 0xb0
#define CM_PER_GPIO3_CLKCTRL 0xb4
#define CM_WKUP 0x44e00400
#define CM_WKUP_GPIO0_CLKCTRL 0x08
#define CM_WKUP_ADC_TSK_CLKCTL 0xbc

// GPIO Module registers
#define GPIO0 0x44e07000
#define GPIO1 0x4804c000
#define GPIO2 0x481ac000
#define GPIO3 0x481ae000
#define GPIO_CTRL 0x130
#define GPIO_OE 0x134
#define GPIO_DATAIN 0x138
#define GPIO_DATAOUT 0x13c
#define GPIO_DEBOUNCENABLE 0x150
#define GPIO_DEBOUNCINGTIME 0x154
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194

// ADC Registers
#define ADC_TSC 0x44e0d000
#define ADC_TSC_IRQSTATUS 0x28
#define ADC_TSC_IRQENABLE_SET 0x2c
#define ADC_TSC_CTRL 0x40
#define ADC_TSC_ADCRANGE 0x48
#define ADC_TSC_CLKDIV 0x4c
#define ADC_TSC_STEPENABLE 0x54
#define ADC_TSC_STEPCONFIG1 0x64
#define ADC_TSC_STEPDELAY1 0x68
#define ADC_TSC_STEPCONFIG2 0x6c
#define ADC_TSC_STEPDELAY2 0x70
#define ADC_TSC_STEPCONFIG3 0x74
#define ADC_TSC_STEPDELAY3 0x78
#define ADC_TSC_STEPCONFIG4 0x7c
#define ADC_TSC_STEPDELAY4 0x80
#define ADC_TSC_STEPCONFIG5 0x84
#define ADC_TSC_STEPDELAY5 0x88
#define ADC_TSC_STEPCONFIG6 0x8c
#define ADC_TSC_STEPDELAY6 0x90
#define ADC_TSC_STEPCONFIG7 0x94
#define ADC_TSC_STEPDELAY7 0x98
#define ADC_TSC_STEPCONFIG8 0x9c
#define ADC_TSC_STEPDELAY8 0xa0
#define ADC_TSC_STEPCONFIG9 0xa4
#define ADC_TSC_STEPDELAY9 0xa8
#define ADC_TSC_STEPCONFIG10 0xac
#define ADC_TSC_STEPDELAY10 0xb0
#define ADC_TSC_STEPCONFIG11 0xb4
#define ADC_TSC_STEPDELAY11 0xb8
#define ADC_TSC_STEPCONFIG12 0xbc
#define ADC_TSC_STEPDELAY12 0xc0
#define ADC_TSC_STEPCONFIG13 0xc4
#define ADC_TSC_STEPDELAY13 0xc8
#define ADC_TSC_STEPCONFIG14 0xcc
#define ADC_TSC_STEPDELAY14 0xd0
#define ADC_TSC_STEPCONFIG15 0xd4
#define ADC_TSC_STEPDELAY15 0xd8
#define ADC_TSC_STEPCONFIG16 0xdc
#define ADC_TSC_STEPDELAY16 0xe0
#define ADC_TSC_FIFO0COUNT 0xe4
#define ADC_TSC_FIFO1COUNT 0xf0
#define ADC_TSC_FIFO0DATA 0x100
#define ADC_TSC_FIFO1DATA 0x200

#endif //REGISTERS_H
