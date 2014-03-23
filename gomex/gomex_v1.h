/**************************************************************************
    PygmyOS ( Pygmy Operating System )
    Copyright (C) 2011-2012  Warren D Greenway

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#define __PYGMYGOMEX

#define LED1                PC6
#define LED2                PC7
#define BAT1                PC4
#define BAT2                PC5
#define BT_CONNECT          PC3
#define MODE0               PC2
#define MODE1               PC1
#define MODE2               PC0
#define CURRENT_OUT         PB0
#define VOLTAGE_OUT         PB1
#define BT_RESET            PC11
#define ENABLE_RES          PB7

#define FLASH_CS            PA4
    #define FLASH_CS_INIT   GPIOA->CRL &= ~PIN4_CLEAR; GPIOA->CRL |= PIN4_OUT50_GPPUSHPULL
    #define FLASH_CS_HIGH   GPIOA->BSRR = BIT4
    #define FLASH_CS_LOW    GPIOA->BRR = BIT4
#define FLASH_SCK           PA5
    #define FLASH_SCK_INIT  GPIOA->CRL &= ~PIN5_CLEAR; GPIOA->CRL |= PIN5_OUT50_GPPUSHPULL
    #define FLASH_SCK_HIGH  GPIOA->BSRR = BIT5
    #define FLASH_SCK_LOW   GPIOA->BRR = BIT5
#define FLASH_MISO          PA6
    #define FLASH_MISO_INIT GPIOA->CRL &= ~PIN6_CLEAR; GPIOA->CRL |= PIN6_IN_FLOAT
    #define FLASH_MISO_IN   ( GPIOA->IDR & BIT6 )
    #define FLASH_MISO_STATE FLASH_MISO_IN
#define FLASH_MOSI          PA7
    #define FLASH_MOSI_INIT GPIOA->CRL &= ~PIN7_CLEAR; GPIOA->CRL |= PIN7_OUT50_GPPUSHPULL
    #define FLASH_MOSI_HIGH GPIOA->BSRR = BIT7
    #define FLASH_MOSI_LOW  GPIOA->BRR = BIT7

#define COM1_TX             PA9
    #define TX1             PA9
#define COM1_RX             PA10
    #define RX1             PA10
#define COM1_RTS            PA12
    #define RTS1            PA12
#define COM1_CTS            PA11
    #define CTS1            PA11
#define COM2_TX             PA2
    #define TX2             PA2
#define COM2_RX             PA3
    #define RX2             PA3
#define COM2_RTS            PA1
    #define RTS2            PA1
#define COM2_CTS            PA0
    #define CTS2            PA0
#define COM3_TX             PB10
    #define TX3             PB10
#define COM3_RX             PB11
    #define RX3             PB11
#define COM3_RTS            PB14
    #define RTS3            PB14
#define COM3_CTS            PB13
    #define CTS3            PB13
#define COM3_RI             PB12
#define COM3_DCD            PB8
#define COM3_DTR            PB9
#define COM4_TX             PC10
#define COM4_RX             PC9

#define PYGMY_MAX_XTAL      ((u32)72000000) // Maximum main clock frequency supported
#define PYGMY_XTAL          ((u32)8000000) // XTAL frequency in Hz
#define PYGMY_HSI           ((u32)8000000) // This is a generic value for default internal frequency
#define PYGMY_SUPPLYVOLTAGE 3.3


