/**************************************************************************
    PygmyOS ( Pygmy Operating System ) - BootLoader
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
#include <stdlib.h>
#include <string.h>
#include "pygmy_type.h"
#include "xmodem.h"

#define BOOT_SIZE 65536;
#define BOOT_BUILDVERSION   2000 // 1.0 = 1000, 1.1 = 1100

#define VECT_TAB_OFFSET  0x1000
//#define FLASH_BASE       ((u32)0x08000000)

#define BOOT_CANCEL         BIT7
#define BOOT_NOOS           BIT6
#define BOOT_BAUDRATE       115200

u16 generateBootBaud( void );
u8 bootTest( void );
void bootPrintPrompt( void );
//u8 bootTestAndLoad( void );
u8 bootTestAndLoad( u32 Address, u8* FileName );
void bootBootOS( void );
u32 getIDCode( void );
u8 scriptRun( u8 *fileName, PYGMYCMD *pygmyCommands );
u8 executeCmd( u8 *ucBuffer, PYGMYCMD *pygmyCommands );

//void putBufferUSART3( u16 uiLen, u8 *ucBuffer );
//void putcUSART3( u8 ucChar );
//void putstr( u8 *ucBuffer );
//void putIntUSART3( u32 ulData );
//u8 xmodemWritePacket( u8 *ucBuffer );
//void xmodemSendPacket( u8 ucLast );
void bootGetUSART3( void );
void bootGetUSART2( void );
void bootGetUSART1( void );

u8 cmd_erase( u8 *Buffer );
u8 cmd_rx( u8 *Buffer );
u8 cmd_tx( u8 *Buffer );
u8 cmd_reset( u8 *Buffer );
u8 cmd_boot( u8 *Buffer );  
u8 cmd_flash( u8 *Buffer );
u8 cmd_cksum( u8 *Buffer );
u8 cmd_null( u8 *Buffer );

const u8 STRID_L15X[] = "L15x";
const u8 STRID_F100[] = "F100";
const u8 STRID_F103[] = "F103";
const u8 BOOT_OK[] = "\rOK\r";
const u8 BOOT_ERROR[] = "\rERROR\r>";
const u8 BOOT_PROMPT[] = "\r> ";
const u8 BOOT_filename[] = "boot.hex";

#pragma codeseg( ".commands" )
// The commands section follows the vector table and is 0x0F00 in size
// The following command table entries should be maintained in alphabetical order
// The exception to this rule is pygmy_cmd, which should always be the first entry
const PYGMYCMD BOOTCOMMANDS[] = {   {(u8*)"boot",       cmd_boot},
                                    {(u8*)"cksum",      cmd_cksum},
                                    {(u8*)"dump",       cmd_dump},
                                    {(u8*)"erase",      cmd_erase},
                                    {(u8*)"reset",      cmd_reset},
                                    {(u8*)"rx",         cmd_rx},
                                    {(u8*)"tx",         cmd_tx},
                                    {(u8*)"", NULL } // No Commands after NULL
                                    } ;
#pragma codeseg( default )

const u8 *date = __DATE__;
const u8 *time = __TIME__;
u8 *version;
PYGMYXMODEM XModem;
PYGMYFILE pygmyFile;
u32 globalFileLen, globalXMTimeout, globalTransaction, globalXMCount;
volatile u32 globalBootTimeout;
volatile u32 globalPLL, globalID, globalXTAL, globalFreq;//, globalBaseAddress;
volatile u8 globalStatus = 0, globalBootStatus = 0;
volatile u8 *globalStrID;
                                
int main( void )
{   
    u32 i, ii, ulClock;
    u8 floatString[20];
    
    // First test the descriptor page ( last page in FLASH )
    // if configuration exists, use programmed ID, else query
    // Debug registers for ID. This is a workaround for silicon
    // issues in the F103 STM32s
    RCC->APB2ENR |= (RCC_IOPBEN|RCC_IOPCEN|RCC_IOPAEN);
    PYGMY_RCC_USART3_ENABLE;
    PYGMY_RCC_USART2_ENABLE;
    PYGMY_RCC_USART1_ENABLE; 
    PYGMY_RCC_GPIOA_ENABLE;
    PYGMY_RCC_GPIOB_ENABLE;

    //globalID = fpecMCUID( );
    globalID = 0;
    globalPLL = BIT16|BIT1;
    
    // First Init the Clocks
    if( globalID == 0x0416 ){
        // L152
        globalStrID = (u8*)STRID_L15X;  
	globalXTAL = 16000000;  
        globalFreq = 32000000;
        ulClock = globalFreq;
        FPEC->ACR = FPEC_ACR_PRFTBE | FPEC_ACR_LATENCY1;
    } else if( globalID == 0x0420 || globalID == 0x0428 ){
        // F100 
        globalStrID = (u8*)STRID_F100;
        globalXTAL = 12000000;
        globalFreq = 24000000;
        ulClock = globalFreq;
    } else{
        // F103
        globalStrID = (u8*)STRID_F103;
        globalXTAL = 8000000;
        globalFreq = 72000000; //72022900;
        ulClock = 36000000;
        globalPLL = RCC_PLL_X9|BIT16|BIT15|BIT14|BIT1;//BIT10|BIT1;
        FPEC->ACR = FPEC_ACR_PRFTBE | FPEC_ACR_LATENCY2;
    } // else
    
    PYGMY_RCC_HSI_ENABLE;
    PYGMY_RCC_HSE_ENABLE;
    while( !PYGMY_RCC_HSE_READY );
    RCC->CFGR2 = 0;
    RCC->CFGR = globalPLL;
    PYGMY_RCC_PLL_ENABLE;
    while( !PYGMY_RCC_PLL_READY );
    
    taskInit();
    streamInit();
    streamSetPut( COM1, putsUSART1 );
    streamSetPutc( COM1, putcUSART1 );
    streamSetGet( COM1, bootGetUSART1 );
    streamDisableDefaultGet( COM1 );
    streamSetSTDIO( COM1 );
    
    // Basic Port Init
    
    GPIOB->CRH &= ~( PIN10_CLEAR | PIN11_CLEAR );
    GPIOB->CRH |= ( PIN10_OUT50_ALTPUSHPULL | PIN11_IN_FLOAT );

    USART3->BRR = ( ( (ulClock >> 3 ) / BOOT_BAUDRATE ) << 4 ) + ( ( ( ulClock / BOOT_BAUDRATE ) ) & 0x0007 );
    USART3->CR3 = USART_ONEBITE;
    USART3->CR1 = ( USART_OVER8 | USART_UE | USART_RXNEIE | USART_TE | USART_RE  );
    
    GPIOA->CRH &= ~( PIN9_CLEAR | PIN10_CLEAR );
    GPIOA->CRH |= ( PIN9_OUT50_ALTPUSHPULL | PIN10_IN_FLOAT );

    USART1->BRR = ( ( (ulClock >> 3 ) / BOOT_BAUDRATE ) << 4 ) + ( ( ( ulClock / BOOT_BAUDRATE ) ) & 0x0007 );
    USART1->CR3 = USART_ONEBITE;
    USART1->CR1 = ( USART_OVER8 | USART_UE | USART_RXNEIE | USART_TE | USART_RE  );

    // End Basic Port Init

    // HSI Must be ON for Flash Program/Erase Operations
    // End Clock Init 
    // Configure Interrupts
    // Do not read from NVIC LOAD or VAL!!!
    // Reading from Write only registers causes unpredictable behavior!!!
    NVIC->ISER[ 1 ] = 0x00000001 << 7;
    NVIC->ISER[ 1 ] = 0x00000001 << 6;
    NVIC->ISER[ 1 ] = 0x00000001 << 5;
    SYSTICK->VAL = globalFreq / 1000;
    SYSTICK->LOAD = globalFreq / 1000; // Based on  ( 2X the System Clock ) / 1000
    SYSTICK->CTRL = 0x07;   // Enable system timer
    // End Configure Interrupts
    PYGMY_WATCHDOG_UNLOCK;
    PYGMY_WATCHDOG_PRESCALER( IWDT_PREDIV128 );
    PYGMY_WATCHDOG_TIMER( 0x0FFF );
    PYGMY_WATCHDOG_START;
    PYGMY_WATCHDOG_REFRESH;
    
    println( STDIO, "Pygmy Boot Lite V%s\rMCU ", version );
    println( STDIO, "%s\rPage Size %d\rFLASH %dKB\rRAM %dKB", globalStrID, fpecFlashSectorSize(), fpecFlashSize(), fpecRAMSize() );
    println( STDIO, "\rMCUID: 0x%08X", fpecMCUID() );
    
    xmodemInit( &XModem );
    
    while( 1 ){
        // Wait for commands
    }
}

void bootTimeout( void )
{
    //static u8 ucFirstCall = 0;
    
    //if( ucFirstCall ){
        bootBootOS(); 
    //} // if
    //ucFirstCall = TRUE;
}

void bootPrintPrompt( void )
{
    PYGMYFILEVOLUME *pygmyVolume;
    u8 *ucPath;

    pygmyVolume = fileGetCurrentVolume();
    ucPath = fileGetCurrentPath();
    print( STDIO, "\r[%s]$ ", ucPath );
}

u8 bootTest( void )
{
    // This function pre-tests every row in firmware file for corruption before erase and program
    u32 i;
    u8 ucBuffer[ 64 ], *ucSubString;

    //if( !fileOpen( &pygmyFile, (u8*)BOOT_filename, READ ) ){
    //    return( FALSE );
    //}
 
    for( ; !( pygmyFile.Properties.Attributes & EOF ); ){
        // Get an IHEX packet
        for( i = 0; i < 64; i++ ){
            ucBuffer[ i ] = fileGetChar( &pygmyFile );
            if( ucBuffer[ i ] == '\r' ){
                ucBuffer[ i ] = '\0';
                break;
            } // if
        } // for
        ucSubString = getNextSubString( ucBuffer, WHITESPACE|NEWLINE );
        if( *(ucSubString++) != ':' ){
            return( 0 );
        } // if
        // Following works because the output cannot be longer than the input being converted
        convertHexEncodedStringToBuffer( ucSubString, ucSubString );
        if( ucSubString[ 3 ] == IHEX_EOF ){
            break; // We have reached EOF without error
        }
        //for( i = 0, ucCalculatedSum = 0; i < ucSubString[ 0 ]+4; i++ ){
        //    ucCalculatedSum += ucSubString[ i ];
        //} // for
        //ucCalculatedSum = 1 + ( 0xFF ^ (u8)ucCalculatedSum ); 
        //if( (u8)ucCalculatedSum != ucSubString[ i ] ){ // Last short is checksum
        if( sysCRC8( ucSubString, ucSubString[ 0 ]+4 ) != ucSubString[ i ] ){ // Last short is checksum
            return( FALSE ); // Corrupt HEX Row
        } // if
        i = ( (u16)ucSubString[ 1 ] << 8 ) + ucSubString[ 2 ];
        if( ucSubString[ 3 ] == IHEX_DATA && i < 0x2000 ){
            return( 0 );
        } // if 
    } // for

    //putstr( (u8*)BOOT_OK );
    print( STDIO, (u8*)BOOT_OK );
    return( 1 );
}

u8 bootTestAndLoad( u32 Address, u8 *FileName )
{
    PYGMYFILE *File;
    u32 i, Length;
    u8 Buffer[ 64 ], *SubString, Status;

    File = fileOpen( FileName, READ, 0 );
    if( !File ){
        return( FALSE );
    } // if
    //Length = fpecGetIHEXLength( FileName );
    print( STDIO, "\rFlashing %d bytes to 0x%08", Length, Address );
    fpecEraseProgramMemory();
    print( STDIO, "..." );
    for( Status = 0; !( File->Properties.Attributes & EOF ) && Status != 0xFF; ){
        for( i = 0; i < 64; i++ ){
            Buffer[ i ] = fileGetChar( File );
            if( Buffer[ i ] == '\r' ){
                Buffer[ i ] = '\0';
                SubString = getNextSubString( (u8*)Buffer, WHITESPACE|NEWLINE );
                // Add 1 to pointer before passing to skip the ':' packet start char
                Status = fpecProcessIHEX( (u8*)( SubString + 1 ) );
                break; // Time to fetch next IHEX entry
            } // if
        } // for
    } // for
}

void bootBootOS( void )
{
    PYGMYVOIDPTR pygmyMain;
    u32 *ulOS;
    
    ulOS = (u32*)0x08004004; // Address is start vector table + 4 bytes
    if ( *ulOS != 0xFFFFFFFF ){
        //putstr( "\rBooting..." );
        print( STDIO, "\rBooting..." );
        pygmyMain = (PYGMYVOIDPTR)*ulOS;
        RCC->CIR = 0x009F0000;
        SCB->VTOR = ((u32)0x08004000 & (u32)0x1FFFFF80);
        pygmyMain(); // pass control to Pygmy OS
    } // if
    globalBootStatus = BOOT_CANCEL;
    taskDelete( "boot" );
    //putstr( "\rNo OS\r> " );
    print( STDIO, "\rNo OS" );
    bootPrintPrompt();
} 

void bootGetUSART3( void )
{
    static u8 ucBuffer[ 256 ], ucIndex = 0;
    u8 ucByte, CurrentStream;

    //pinSet( LED1, HIGH );
    CurrentStream = streamGetSTDIO();
    streamSetSTDIO( COM3 );

    if( USART3->SR & USART_RXNE ) { 
        USART3->SR &= ~USART_RXNE;
        ucByte = USART3->DR ;
        if( xmodemProcess( &XModem, ucByte ) ){
            streamSetSTDIO( CurrentStream );
            return;
        } // if
        if( ucByte == '+' ){
            taskDelete( "boot" );
            globalBootStatus = BOOT_CANCEL;
            bootPrintPrompt();
            streamSetSTDIO( CurrentStream );
            return;
        } // if
        if( globalBootStatus & BOOT_CANCEL ){
            if( ucByte == '\r' ){
                ucBuffer[ ucIndex ] = '\0'; // Add NULL to terminate string
                ucIndex = 0;
                if( executeCmd( ucBuffer, (PYGMYCMD *)BOOTCOMMANDS ) ){
                    bootPrintPrompt();
                } else{
                    print( STDIO, "\rERROR" );
                    bootPrintPrompt();
                } // else
            } else{
                USART3->DR = ucByte;
                if( ucByte == '\b'  ){
                    if( ucIndex ){
                        --ucIndex;
                    } // if
                } else {
                    ucBuffer[ ucIndex++ ] = ucByte;
                } // else
            } // else
        } // if
    } // if
    streamSetSTDIO( CurrentStream );
    //pinSet( LED1, HIGH );
}

void bootGetUSART2( void )
{
    static u8 COM2Buffer[ 256 ], COM2Index = 0;
    u8 ucByte, CurrentStream;

    //pinSet( LED0, HIGH );
    CurrentStream = streamGetSTDIO();
    streamSetSTDIO( COM2 );
    if( USART2->SR & USART_RXNE ) { 
        USART2->SR &= ~USART_RXNE;
        ucByte = USART2->DR;
        
        if( xmodemProcess( &XModem, ucByte ) ){
            streamSetSTDIO( CurrentStream );
            return;
        } // if
        if( ucByte == '+' ){
            taskDelete( "boot" );
            globalBootStatus = BOOT_CANCEL;
            bootPrintPrompt();
            streamSetSTDIO( CurrentStream );
            return;
        } // if
        if( globalBootStatus & BOOT_CANCEL ){
            if( ucByte == '\r' ){
                COM2Buffer[ COM2Index ] = '\0'; // Add NULL to terminate string
                COM2Index = 0;
                if( executeCmd( COM2Buffer, (PYGMYCMD *)BOOTCOMMANDS ) ){
                    bootPrintPrompt();
                } else{
                    print( STDIO, "\rERROR" );
                    bootPrintPrompt();
                } // else
            } else{
                USART2->DR = ucByte;
                if( ucByte == '\b'  ){
                    if( COM2Index ){
                        --COM2Index;
                    } // if
                } else {
                    COM2Buffer[ COM2Index++ ] = ucByte;
                } // else
            } // else
        } // if
    } // if
    streamSetSTDIO( CurrentStream );
    //pinSet( LED0, LOW );
}

void bootGetUSART1( void )
{
    static u8 ucBuffer[ 256 ], ucIndex = 0;
    u8 ucByte, CurrentStream;

    CurrentStream = streamGetSTDIO();
    streamSetSTDIO( COM1 );
    if( USART1->SR & USART_RXNE ) { 
        USART1->SR &= ~USART_RXNE;
        ucByte = USART1->DR ;
        USART1->DR = ucByte;
        if( xmodemProcess( &XModem, ucByte ) ){
            streamSetSTDIO( CurrentStream );
            return;
        } // if
        if( ucByte == '+' ){
            globalBootStatus = BOOT_CANCEL;
            //putstr( (u8*)BOOT_PROMPT );
            //print( COM3, BOOT_PROMPT );
            bootPrintPrompt();
            streamSetSTDIO( CurrentStream );
            return;
        } // if
        if( globalBootStatus & BOOT_CANCEL ){
            if( ucByte == '\r' ){
                ucBuffer[ ucIndex ] = '\0'; // Add NULL to terminate string
                ucIndex = 0;
                if( executeCmd( ucBuffer, (PYGMYCMD *)BOOTCOMMANDS ) ){
                    //print( COM3, BOOT_PROMPT  ); 
                    bootPrintPrompt();
                } else{
                    //print( COM3, BOOT_ERROR ); 
                    print( STDIO, "\rERROR" );
                    bootPrintPrompt();
                } // else
            } else{
                //putcUSART3( ucByte );
                //print( COM3, "%c", ucByte );
                USART1->DR = ucByte;
                if( ucByte == '\b'  ){
                    if( ucIndex ){
                        --ucIndex;
                    } // if
                } else {
                    ucBuffer[ ucIndex++ ] = ucByte;
                } // else
            } // else
        } // if
    } // if
    streamSetSTDIO( CurrentStream );
}

u8 cmd_erase( u8 *Buffer )
{
    fpecEraseProgramMemory( );// 8, SIZEREG->Pages - 2 );
    globalStatus |= BIT0; // Mark main memory as erased

    return( 1 );
}

u8 cmd_rx( u8 *Buffer )
{
    Buffer = removeLeadingWhitespace( Buffer );
    xmodemEnable( &XModem );
    
    return( xmodemRecv( &XModem, Buffer ) );
}

/*u8 cmd_tx( u8 *Buffer )
{
    Buffer = removeLeadingWhitespace( Buffer );
    xmodemEnable( &XModem );

    return( xmodemSend( &XModem, Buffer ) );
}*/


u8 cmd_reset( u8 *Buffer )
{
    // Call for a system level reset
    PYGMY_RESET;

    return( TRUE ); // Will never actually reach this point
}

u8 cmd_boot( u8 *Buffer )
{
    u32 Address;
    u8 *Name, *AddressString;

    bootTestAndLoad( Address, Name );

    PYGMY_RESET;
    
    return( TRUE ); // Will never actually reach this point
}

void SysTick_Handler( void )
{
    PYGMY_WATCHDOG_REFRESH; 
    taskProcess();
    //taskVoltage();
    xmodemProcessTimer( &XModem );
}
