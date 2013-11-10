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
#include "pygmy_profile.h"
#include "pygmy_type.h"
#include "pygmy_xmodem.h"
//#include "profiles/memory/stm32.h"

//FLASH_START = 0x08000000 
//FLASH_SIZE = 0x00008000

#define BOOT_BUILDVERSION   2000 // 1.0 = 1000, 1.1 = 1100

#define VECT_TAB_OFFSET  0x1000
//#define FLASH_BASE       ((u32)0x08000000)

#define BOOT_CANCEL         BIT7
#define BOOT_NOOS           BIT6
#define BOOT_BAUDRATE       230400

u16 generateBootBaud( void );
u8 bootTest( void );
void bootPrintPrompt( void );
//u8 bootTestAndLoad( void );
u8 bootTestAndLoad( u32 Address, u8* FileName );
void bootBootOS( void );
u32 getIDCode( void );
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
//s8 isQuote( u8 ucChar );
//s8 isNewline( u8 ucChar );

u8 cmdHandler( u8 ucByte );
u8 cmd_erase( u8 *Buffer );
u8 cmd_format( u8 *Buffer );
u8 cmdRFGet( u8 *Buffer );
u8 cmdRFPut( u8 *Buffer );
u8 cmd_rx( u8 *Buffer );
u8 cmd_tx( u8 *Buffer );
u8 cmd_read( u8 *Buffer );
u8 cmd_rm( u8 *Buffer );
u8 cmd_cd( u8 *Buffer );
u8 cmd_append( u8 *Buffer );
u8 cmd_open( u8 *ucBuffer );
u8 cmd_new( u8 *Buffer );
u8 cmd_mkdir( u8 *Buffer );
u8 cmd_rmdir( u8 *Buffer );
u8 cmd_echo( u8 *Buffer );
u8 cmd_cat( u8 *Buffer );
u8 cmd_strings( u8 *Buffer );
u8 cmd_dump( u8 *Buffer );
u8 cmd_ls( u8 *Buffer );
u8 cmd_touch( u8 *Buffer );
u8 cmd_mv( u8 *Buffer );
u8 cmd_cp( u8 *Buffer );
u8 cmd_reset( u8 *Buffer );
u8 cmd_boot( u8 *Buffer );  
u8 cmd_flash( u8 *Buffer );
u8 cmd_fdisk( u8 *Buffer );
u8 cmd_umount( u8 *Buffer );
u8 cmd_mount( u8 *Buffer );
u8 cmd_verify( u8 *Buffer );
u8 cmd_test( u8 *Buffer );
u8 cmd_date( u8 *Buffer );
u8 cmd_time( u8 *Buffer );
u8 cmd_find( u8 *Buffer );
u8 cmd_df( u8 *Buffer );
u8 cmd_du( u8 *Buffer );
u8 cmd_pwd( u8 *Buffer );
u8 cmd_tail( u8 *Buffer );
u8 cmd_cksum( u8 *Buffer );
u8 cmd_if( u8 *Buffer );
u8 cmd_sleep( u8 *Buffer );
u8 cmd_lsof( u8 *Buffer );
u8 cmd_gawk( u8 *Buffer );
u8 cmd_declare( u8 *Buffer );
u8 cmd_dc( u8 *Buffer );
u8 cmd_kill( u8 *Buffer );
u8 cmd_killall( u8 *Buffer );
u8 cmd_wait( u8 *Buffer );
u8 cmd_watch( u8 *Buffer );
u8 cmd_while( u8 *Buffer );
u8 cmd_true( u8 *Buffer );
u8 cmd_false( u8 *Buffer );
u8 cmd_test( u8 *Buffer );
u8 cmd_man( u8 *Buffer );
u8 cmd_null( u8 *Buffer );

const u8 STRID_L15X[] = "L15x";
const u8 STRID_F100[] = "F100";
const u8 STRID_F103[] = "F103";
const u8 BOOT_OK[] = "\rOK\r";
const u8 BOOT_ERROR[] = "\rERROR\r>";
const u8 BOOT_PROMPT[] = "\r> ";
const u8 BOOT_filename[] = "boot.hex";


const PYGMYCMD BOOTCOMMANDS[] = {   {(u8*)"erase",      cmd_erase},
                                    {(u8*)"format",     cmd_format},
                                    {(u8*)"rx",         cmd_rx},
                                    {(u8*)"tx",         cmd_tx},
                                    {(u8*)"rfget",      cmdRFGet},
                                    {(u8*)"rfput",      cmdRFPut},
                                    {(u8*)"cd",         cmd_cd},
                                    {(u8*)"read",       cmd_read},
                                    {(u8*)"open",       cmd_open},
                                    {(u8*)"append",     cmd_append},
                                    {(u8*)"strings",    cmd_strings},
                                    {(u8*)"new",        cmd_new},
                                    {(u8*)"mkdir",      cmd_mkdir},
                                    {(u8*)"rmdir",      cmd_rmdir},
                                    {(u8*)"touch",      cmd_touch},
                                    {(u8*)"cat",        cmd_cat},
                                    {(u8*)"echo",       cmd_echo},
                                    {(u8*)"rm",         cmd_rm},
                                    {(u8*)"ls",         cmd_ls},
                                    {(u8*)"mv",         cmd_mv},
                                    {(u8*)"cp",         cmd_cp},
                                    {(u8*)"reset",      cmd_reset},
                                    {(u8*)"boot",       cmd_boot},
                                    {(u8*)"dump",       cmd_dump},
                                    {(u8*)"fdisk",      cmd_fdisk},
                                    {(u8*)"mount",      cmd_mount},
                                    {(u8*)"umount",     cmd_umount},
                                    //{(u8*)"flash", cmdFlash},
                                    //{(u8*)"verify", cmdVerify},
                                    {(u8*)"date",       cmd_date},
                                    {(u8*)"time",       cmd_time},
                                    {(u8*)"find",       cmd_find},
                                    {(u8*)"df",         cmd_df},
                                    {(u8*)"du",         cmd_du},
                                    {(u8*)"pwd",        cmd_pwd},
                                    {(u8*)"tail",       cmd_tail},
                                    {(u8*)"cksum",      cmd_cksum},
                                    {(u8*)"if",         cmd_if},
                                    {(u8*)"sleep",      cmd_sleep},
                                    {(u8*)"lsof",       cmd_lsof},
                                    {(u8*)"gawk",       cmd_gawk},
                                    {(u8*)"declare",    cmd_declare},
                                    {(u8*)"dc",         cmd_dc},
                                    {(u8*)"kill",       cmd_kill},
                                    {(u8*)"killall",    cmd_killall},
                                    {(u8*)"wait",       cmd_wait},
                                    {(u8*)"watch",      cmd_watch},
                                    {(u8*)"while",      cmd_while},
                                    {(u8*)"true",       cmd_true},
                                    {(u8*)"false",      cmd_false},
                                    {(u8*)"test",       cmd_test},
                                    {(u8*)"man",        cmd_man},
                                    {(u8*)"", NULL }//cmdNull} // No Commands after NULL
                                    } ;//__attribute__((section("COMMANDS")));

PYGMYXMODEM XModem;
PYGMYFILE pygmyFile;
u32 globalFileLen, globalXMTimeout, globalTransaction, globalXMCount;
volatile u32 globalBootTimeout;
volatile u32 globalPLL, globalID, globalXTAL, globalFreq;//, globalBaseAddress;
volatile u8 globalStatus = 0, globalBootStatus = 0;
volatile u8 *globalStrID;
  
void bootTimeout( void )
{
    //static u8 ucFirstCall = 0;
    
    //if( ucFirstCall ){
        bootBootOS(); 
    //} // if
    //ucFirstCall = TRUE;
}
                                
int main( void )
{   
    //PYGMYSPIPORT pygmySPI;
    PYGMYFILEVOLUME *Volume;
    //PYGMYFILEPROPERTIES Properties;
    //PYGMYFILEADDRESS *Sectors;
    //PYGMYFILE *fileTest;
    //u64 Address;
    u32 i, ii, ulClock;
    //u8 full, ucLen, ucParams[] = "Test String";
    //u8 DataByte;
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
    pinConfig( LED0, OUT );
    pinConfig( LED1, OUT );
    pinSet( LED0, HIGH );
    globalID = fpecMCUID( );
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
    //PYGMY_RCC_HSI_ENABLE;
    PYGMY_RCC_HSE_ENABLE;
    while( !PYGMY_RCC_HSE_READY );
    RCC->CFGR2 = 0;
    RCC->CFGR = globalPLL;
    PYGMY_RCC_PLL_ENABLE;
    while( !PYGMY_RCC_PLL_READY );
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
   
    // Basic Port Init
    
    
    // End Basic Port Init
    // USART3 Init, PB10 TXD, PB11 RXD
    //comConfig( COM3, 0, 0, 230400 );
    //comConfig( COM1, 0, 0, 230400 );
    GPIOB->CRH &= ~( PIN10_CLEAR | PIN11_CLEAR );
    GPIOB->CRH |= ( PIN10_OUT50_ALTPUSHPULL | PIN11_IN_FLOAT );

    USART3->BRR = ( ( (ulClock >> 3 ) / BOOT_BAUDRATE ) << 4 ) + ( ( ( ulClock / BOOT_BAUDRATE ) ) & 0x0007 );
    //USART3->BRR = ( ( (ulClock >> 3 ) / 9600 ) << 4 ) + ( ( ( ulClock / 9600 ) ) & 0x0007 );
    USART3->CR3 = USART_ONEBITE;
    USART3->CR1 = ( USART_OVER8 | USART_UE | USART_RXNEIE | USART_TE | USART_RE  );

    /*GPIOA->CRH &= ~( PIN9_CLEAR | PIN10_CLEAR );
    GPIOA->CRH |= ( PIN9_OUT50_ALTPUSHPULL | PIN10_IN_FLOAT );

    USART1->BRR = ( ( (ulClock >> 3 ) / BOOT_BAUDRATE ) << 4 ) + ( ( ( ulClock / BOOT_BAUDRATE ) ) & 0x0007 );
    USART1->CR3 = USART_ONEBITE;
    USART1->CR1 = ( USART_OVER8 | USART_UE | USART_RXNEIE | USART_TE | USART_RE  );

    //GPIOA->CRH &= ~( PIN2_CLEAR | PIN3_CLEAR );
    //comConfig( COM2, 0,  0, 230400 );
    
    //GPIOA->CRH |= ( PIN2_OUT50_ALTPUSHPULL | PIN3_IN_FLOAT );

    USART2->BRR = ( ( (ulClock >> 3 ) / 9600 ) << 4 ) + ( ( ( ulClock / 9600 ) ) & 0x0007 );
    USART2->CR3 = USART_ONEBITE;
    USART2->CR1 = ( USART_OVER8 | USART_UE | USART_RXNEIE | USART_TE | USART_RE  );
    */
    //interruptEnable( USART1_IRQ );
    //interruptSetPriority( USART1_IRQ, 1 );
    //pinConfig( PA9, ALT );
    //pinConfig( PA10, IN );
    //USART1->CR3 = 0;
    // Warning! CTS and RTS are not supported on UART4-UART5
   
    //USART1->BRR = ( ( (ulClock >> 3 ) / BOOT_BAUDRATE ) << 4 ) + ( ( ( ulClock / BOOT_BAUDRATE ) ) & 0x0007 ); //comGenerateBaud( 72000000, 230400 );
    //USART1->CR3 = USART_ONEBITE;
    //USART1->CR1 = ( USART_OVER8 | USART_UE | USART_RXNEIE | USART_TE | USART_RE  );
    //ptrUSART->CR1 = ( USART_UE | USART_RXNEIE | USART_TE | USART_RE  );
    //USART1->SR = 0;
    // End USART3 Init
        
    // Note: SST FLASH ICs must have write-protection removed by formatting before use
    //pinConfig( LED0, OUT );
    //pinConfig( LED1, OUT );
    streamInit();
    streamSetPut( COM3, putsUSART3 );
    streamSetPutc( COM3, putcUSART3 );
    streamSetGet( COM3, bootGetUSART3 );
    streamDisableDefaultGet( COM3 );
    /*
    streamSetPut( COM1, putsUSART1 );
    streamSetPutc( COM1, putcUSART1 );
    streamSetGet( COM1, bootGetUSART1 );
    streamDisableDefaultGet( COM1 );

    streamSetPut( COM2, putsUSART2 );
    streamSetPutc( COM2, putcUSART2 );
    streamSetGet( COM2, bootGetUSART2 );
    streamDisableDefaultGet( COM2 );
    */
    streamSetSTDIO( COM3 );
    pinSet( LED1, HIGH );
    print( STDIO, "\rPygmy Boot V2.0\rMCU " );
    print( STDIO, "%s\rPage Size %d\rFLASH %dKB\rRAM %dKB", globalStrID, fpecFlashSectorSize(), fpecFlashSize(), fpecRAMSize() );
    print( STDIO, "\rMCUID: 0x%08X", fpecMCUID() );

    //print( COM3, "%s\rFLASH %d\r", globalStrID, SIZEREG->Pages );
    //fpecWriteWord( (u16*)0x8008000, 0xAAFF );
    //fpecWriteWord( (u16*)0x8008000, 0xFFAA );
    //print( COM3, "\r" );
    //for( i = 0; ucParams[ i ]; i++ ){  
    //    fpecWriteByte( 0x8008000 + i, ucParams[ i ] ); // Test String
    //} // for
    
    //fileFormat( (PYGMYMEMIO *)&STM32MEMIO, 0, FPEC_BASEADDRESS, 0x8000, 0xF8000 );
    //Volume = fileNewVolume( (PYGMYMEMIO *)&STM32MEMIO, "nebula", FPEC_BASEADDRESS, 0x8000, 0xF8000 );
    timeInit();
    xmodemInit( &XModem );
    fileFormat( (PYGMYMEMIO *)&SST25VF, TRUE, FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
    Volume = fileNewVolume( (PYGMYMEMIO *)&SST25VF, "nebula", FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
    fileLoadVolume( (PYGMYMEMIO *)&SST25VF, "nebula", FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
    //spiConfig( &pygmySPI, FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI, 0 );
    //s25flPutChar( &pygmySPI, 0, 0xAA );

    /*print( COM3, "\rFormatting Spansion Flash" );
    fileFormat( (PYGMYMEMIO *)&S25FL, TRUE, FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
    print( COM3, "\rCreating volume" );
    Volume = fileNewVolume( (PYGMYMEMIO *)&S25FL, "nebula", FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
    print( COM3, "\rLoading volume" );
    fileLoadVolume( (PYGMYMEMIO *)&S25FL, "nebula", FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
    print( COM3, "..." );
    */

    //spiConfig( &pygmySPI, FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI, SPIMODE0|SPIADDRESS1BYTE );
    //spiPutCommand( ptrSPI, S25FL_WREN ); // WREN must be written before status register may be modified
    //pygmySPI.PortCS->BRR = pygmySPI.PinCS;
    //spiWriteByte( &pygmySPI, 0xAA );
    //pygmySPI.PortCS->BSRR = pygmySPI.PinCS;
    //spiGetChar( &pygmySPI, 0x0000000 );//, 0xAA );
    //pygmySPI.PortCS->BRR = pygmySPI.PinCS;
    //spiWriteAddress( pygmySPI, ulAddress );
    //if( pygmySPI.CR & SPIDUMMYONREAD ){
    //    spiReadByte( pygmySPI );
    //} // if
    //pygmySPI.PortSCK->BSRR = pygmySPI.PinSCK;	
    //for( i = 0, DataByte = 0; i < 8; i++ ){               // 
    //    pygmySPI.PortSCK->BSRR = pygmySPI.PinSCK;         // clock must start low, transition high 	
        
        //if( pygmySPI.PortMISO->IDR & pygmySPI.PinMISO ){  // Test port input for high and set bit in ucByte
        //    DataByte |= ( BIT7 >> i );                    //
        //} // if                                           //
        //delay( 60 );
    //    pygmySPI.PortSCK->BRR = pygmySPI.PinSCK;
    //} // for
    //pygmySPI.PortCS->BRR = pygmySPI.PinCS;
    //print( COM3, "\rByte: 0x%02X", DataByte );
    
    //spiWriteByte( ptrSPI, S25FL_WRR );

    //Volume->IO->PutBuffer( Volume->Port, 0x2000LL, Name, len(Name) + 1 );
   
    /*Address = 0x15000;
    for( i = 0; i < 4; i++ ){
        Volume->IO->PutWord( Volume->Port, Address + ( i * 4096 ), PYGMYFILE_ACTIVESECTOR );
    } // for

    //Volume->IO->PutWord( Volume->Port, (u64)(Address + (u64)(ii * SectorSize)), PYGMYFILE_ACTIVESECTOR );
    //fileLoadVolume( (PYGMYMEMIO *)&SST25VF, "nebula", FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );

    //fileSetCurrentPath( "/nebula" );
    //fileMount( (PYGMYMEMIO *)&SST25VF, 0, "nebula", FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
    //fileSeekVolumes( (PYGMYMEMIO *)&SST25VF, FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );

    pinConfig( LED0, OUT );
    
    /*Properties.SectorCount = 4;
    Properties.SectorGroups = 2;
    Sectors = malloc( sizeof( PYGMYFILEADDRESS ) * 2 );
    Properties.Sectors = Sectors;
    Properties.Sectors[ 0 ].BaseAddress = 0;
    Properties.Sectors[ 0 ].SectorCount = 2;
    Properties.Sectors[ 1 ].BaseAddress = 4096*2;
    Properties.Sectors[ 1 ].SectorCount = 2;
    Volume = fileGetCurrentVolume();
    Properties.MountPoint = Volume;
    
    for( i = 0; i < 4094*4; i++ ){
        fileGenerateDirectAddress( Volume, &Properties, i, &Address );
        if( Address != PYGMYFILE_INVALIDADDRESS ){
            print( COM3, "\r0x%012llX", Address );
        } else{
            print( COM3, "\rINVALID ADDRESS" );
            break;
        } // else
    } // for
    */
    /*
    fileTest = fileOpen( "/nebula/test2.txt", WRITE, 0 );
    if( fileTest ){
        print( COM3, "\rPut Chars: " );
        for( i = 0, full = FALSE; i < 741 && !full; i++ ){ // 741
            for( ii = 0; ii < 50 && !full; ii++ ){ // 50
                if( !filePutChar( fileTest, 'A' ) ){
                    print( COM3, "\rMain()->filePutChar() returned FALSE" ); 
                    full = TRUE;
                } // if
            } // for
            fileUpdateLength( fileTest->ParentFolder, &fileTest->Properties  );
        } // for
        fileClose( fileTest );
    } else{
       print( COM3, "\rFailed to open file" );
    } // else
    */
    //fileOpen( "/nebula/images", FOLDER|WRITE|READ, 0 );
    //fileOpen( "/nebula/test/test.txt", WRITE|READ, 0 );
    
    //putstr( "\rSplit: " ); putstr( splitString( "/nebula/test.txt", '/', -1 ) );
    //if( fileSeekPath( "/nebula", &pygmyEntry ) ){
    //    filePrintProperties( &pygmyEntry );
    //} // if
    /*ucLen = getAllSubStrings( "/root/test/test/test.txt", ucParams, 32, FILESEPARATORS );
    putstr( "\rParams Found: " ); putIntUSART3( ucLen );
    for( i = 0; i < ucLen; i++ ){
        putstr( "\rParam" ); putIntUSART3( i ); putstr( ": " ); putstr( ucParams[ i ] );
    } // for
    */
    //putstr( "\rPygmy Boot V2.0\rMCU " );
    
   
    //putstr( "\rFLASH " );
    //putIntUSART3( SIZEREG->Pages );
    //putcUSART3( '\r' );
    //fileMountRoot();
    // First allow 2 seconds for receipt of '+' char 
    // If received, cancel download sequence and wait for commands
    // Timeout is incremented every millisecond by SysTick
    
    //taskNewSimple( "xmodem", 1000, (void *)xmodemProcessTimer );
    //taskNewSimple( "boot", 5000, (void *)bootTimeout );
    
    /*for( globalBootTimeout = 0; globalBootTimeout < 2000; ){
        ;
    }
    if( !(globalBootStatus & BOOT_CANCEL) ){
        bootBootOS();   
    }*/
    while( 1 ){
        // Wait for commands
    }
}

u8 cmd_time( u8 *Buffer )
{
    PYGMYPARAMLIST Parameters;
    u32 MilliSeconds;
    u8 ReturnValue;

    // This function calls another command and measures the time it takes to execute
    if( getAllParameters( Buffer, &Parameters ) ){
        MilliSeconds = 0;
        ReturnValue = executeCmd( Parameters.Params[ 0 ], (PYGMYCMD *)BOOTCOMMANDS );
        
    } // if

    return( FALSE );
}

u8 cmd_date( u8 *Buffer )
{
    PYGMYPARAMLIST Parameters;
    u32 Seconds;
    u16 i, ii, len;
    u8 TmpChar, *TmpString;

    if( getAllParameters( Buffer, &Parameters ) && Parameters.SwitchCount == 1 && Parameters.ParamCount == 1  ){
        
        if( Parameters.SwitchCount == 1 && Parameters.ParamCount == 1 ){
            if( !strcmp( Parameters.Switches[ 0 ], "s" ) ){
                print( STDIO, "\rSetting time" );
                Seconds = convertDateStringToSeconds( Parameters.Params[ 0 ] );
                print( STDIO, "\rSet to %t", Seconds );
                timeSet( Seconds );
            } // if
        } // if
    } else{
        print( STDIO, "\r%t", timeGet() );
    } // else

    return( TRUE );
}

u8 cmd_fdisk( u8 *Buffer )
{
    PYGMYFILEVOLUME *Volume;
    u8 i, Len, *Params[ 2 ];

    Len = getAllSubStrings( Buffer, Params, 2, WHITESPACE|NEWLINE );
    if( Len == 0 ){
        return( FALSE );
    } // if
    if( !strcmp( Params[ 0 ], "--new" ) || !strcmp( Params[ 0 ], "-n" ) ){
        // Create a new volume/partition
        print( STDIO, "\rCreating new volume" );
        //fileNewVolume( (PYGMYMEMIO *)&S25FL, Params[ 1 ], FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
        fileNewVolume( (PYGMYMEMIO *)&SST25VF, Params[ 1 ], FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
        //fileNewVolume( (PYGMYMEMIO *)&STM32MEMIO, Params[ 1 ], FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
    } else if( !strcmp( Params[ 0 ], "--list" ) || !strcmp( Params[ 0 ], "-l" ) ){
        // List the current volumes/partitions
        print( STDIO, "\rListing volumes" );
        //fileSeekVolumes( (PYGMYMEMIO *)&S25FL, FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
        fileSeekVolumes( (PYGMYMEMIO *)&SST25VF, FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
        //fileSeekVolumes( (PYGMYMEMIO *)&STM32MEMIO, FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
    } else if( !strcmp( Params[ 0 ], "--delete" ) ){
        //fileDeleteVolume( (PYGMYMEMIO *)&S25FL, Params[ 1 ], FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
        fileDeleteVolume( (PYGMYMEMIO *)&SST25VF, Params[ 1 ], FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
        //fileDeleteVolume( (PYGMYMEMIO *)&STM32MEMIO, Params[ 1 ], FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
    } else if( !strcmp( Params[ 0 ], "--folders" ) ){
        Volume = fileFindVolume( Params[ 1 ] );
        if( !Volume ){
            return( FALSE );
        } // if
        print( STDIO, "\rVolume:" );
        if( Volume->ParentFolder->IsRoot ){
            print( STDIO, "\r\tIsRoot = TRUE" );
        } else{
            print( STDIO, "\r\tIsRoot = FALSE" );
        } // else
        print( STDIO, "\r\tName: %s", Volume->Properties.Name );
        print( STDIO, "\r\tSectorCount: %d", Volume->Properties.SectorCount );
        print( STDIO, "\r\tSectorGroups: %d", Volume->Properties.SectorGroups );
        for( i = 0; i < Volume->Properties.SectorGroups; i++ ){
            print( STDIO, "\r\t\tBaseAddress: 0x%012llX", Volume->Properties.Sectors[ i ].BaseAddress );
            print( STDIO, "\r\t\tSectorCount: %d", Volume->Properties.Sectors[ i ].SectorCount );
        } // for
        
        print( STDIO, "\r\tVolume Parent:" );
        
        print( STDIO, "\r\tSectorCount: %d", Volume->ParentFolder->Properties.SectorCount );
        print( STDIO, "\r\tSectorGroups: %d", Volume->ParentFolder->Properties.SectorGroups );
        for( i = 0; i < Volume->ParentFolder->Properties.SectorGroups; i++ ){
            print( STDIO, "\r\t\tBaseAddress: 0x%012llX", Volume->ParentFolder->Properties.Sectors[ i ].BaseAddress );
            print( STDIO, "\r\t\tSectorCount: %d", Volume->ParentFolder->Properties.Sectors[ i ].SectorCount );
        } // for
    } // else if

    return( TRUE );
}

u8 cmd_mount( u8 *Buffer )
{
    // This function takes the pins and device address for the memory interface and mounts volumes(s) from it
    u16 i;
    u8 Len, *Params[ 8 ];
    u8 Address, Pins[ 8 ], *Name;

    Len = getAllSubStrings( Buffer, Params, 8, WHITESPACE|NEWLINE );
    if( Len == 0 ){
        return( FALSE );
    } // if
    Name = Params[ 0 ];
    if( !strcmp( "--debug", Name ) ){
        print( STDIO, "\rMounting interface in debug mode" );
        Name = NULL;
    } // if
    //Address = convertStringToInt( Params[ 1 ] );
    for( i = 0; i < Len - 1; i++ ){
        Pins[ i ] = convertStringToPin( Params[ 1 + i ] );
        if( Pins[ i ] == 0xFF ){
            return( FALSE );
        } // if
        print( STDIO, "\r%s = %d", Params[ 1 + i ], Pins[ i ] );
    } // for
    //fileMount( (PYGMYMEMIO *)&SST25VF, Address, Name, Pins[0], Pins[1], Pins[2], Pins[3] );
    //fileLoadVolume( (PYGMYMEMIO *)&S25FL, Name, Pins[0], Pins[1], Pins[2], Pins[3] );
    fileLoadVolume( (PYGMYMEMIO *)&SST25VF, Name, Pins[0], Pins[1], Pins[2], Pins[3] );
    //fileLoadVolume( (PYGMYMEMIO *)&STM32MEMIO, Name, Pins[0], Pins[1], Pins[2], Pins[3] );

    return( TRUE );
}


u8 cmd_umount( u8 *Buffer )
{
    u16 i;
    u8 Len, *Params[ 2 ];
    u8 *Name, Status;

    Len = getAllSubStrings( Buffer, Params, 2, WHITESPACE|NEWLINE );
    if( Len == 0 ){
        return( FALSE );
    } // if
    Name = Params[ 0 ];

    Status = fileUnmount( Name );
    print( STDIO, "\rUnmounted %s", Name );

    return( Status );
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
    u8 ucBuffer[ 64 ], *ucSubString;//ucCalculatedSum, ;

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

/*u8 bootTestAndLoad( void )
{
    u32 i;
    u8 ucBuffer[ 64 ], ucStatus, *ucSubString;

    //putstr( "\rFlashing" );
    print( COM3, "\rFlashing" );
    //if( !fileOpen( &pygmyFile, (u8*)BOOT_filename, READ ) ){ // If file boot.hex exists then load
    //    return( 0 );
    //} // if
    
    fpecEraseProgramMemory();
    //putstr( "..." );
    print( COM3, "..." );
    for( ucStatus = 0; !( pygmyFile.Properties.Attributes & EOF ) && ucStatus != 0xFF; ){
        for( i = 0; i < 64; i++ ){
            ucBuffer[ i ] = fileGetChar( &pygmyFile );
            if( ucBuffer[ i ] == '\r' ){
                ucBuffer[ i ] = '\0';
                ucSubString = getNextSubString( (u8*)ucBuffer, WHITESPACE|NEWLINE );
                // Add 1 to pointer before passing to skip the ':' packet start char
                ucStatus = fpecProcessIHEX( (u8*)( ucSubString + 1 ) );
                break; // Time to fetch next IHEX entry
            } // if
        } // for
    } // for

    // Write Device Descriptor to last page of FLASH
    //fpecWriteLong( uiAddress, globalID );
    //fpecWriteLong( uiAddress + 2, globalXTAL );
    //fpecWriteLong( uiAddress + 4, globalFreq );
    //fpecWriteLong( uiAddress + 6, BOOT_BUILDVERSION );
    //PYGMY_WATCHDOG_REFRESH;
    //fpecWriteDescriptor( 0, globalID );
    //fpecWriteDescriptor( 1, globalXTAL );
    //fpecWriteDescriptor( 2, globalFreq );
    //fpecWriteDescriptor( 3, BOOT_BUILDVERSION );

    return( 1 );
}*/

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
/*
void putBufferUSART3( u16 uiLen, u8 *ucBuffer )
{
    u16 i;

    for( i = 0; i < uiLen ; i++ ){
        //putcUSART3( ucBuffer[ i ] ) ;
        print( COM3, "%c", ucBuffer[ i ] ) ;
    } // for   
}

void putIntUSART3( u32 ulData )
{
    s32 i, iMagnitude, iValue;
    
    if( ulData & BIT31 ){
        putstr( "ERROR" );
        return;
    }  // if  
    for( i = 0, iMagnitude = 1; ( iMagnitude * 10 ) <= ulData; i++ )
        iMagnitude *= 10;
    
    for( ; i >= 0; i-- ){
        iValue = ulData / iMagnitude;
        putcUSART3( 48 + iValue ); // 48 = '0' in ASCII
        ulData -= ( iValue * iMagnitude );
        iMagnitude /= 10;
    }
    
}

void putcUSART3( u8 ucChar )
{
    while( !( USART3->SR & USART_TXE ) );
    USART3->DR = ucChar; 
}

void putstr( u8 *ucBuffer )
{ 
    for( ; *ucBuffer ; ){
        putcUSART3( *(ucBuffer++) );
    }
}*/

void bootGetUSART3( void )
{
    static u8 ucBuffer[ 256 ], ucIndex = 0;
    u8 ucByte, CurrentStream;

    pinSet( LED1, HIGH );
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
    pinSet( LED1, HIGH );
}

void bootGetUSART2( void )
{
    static u8 COM2Buffer[ 256 ], COM2Index = 0;
    u8 ucByte, CurrentStream;

    pinSet( LED0, HIGH );
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
    pinSet( LED0, LOW );
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
            taskDelete( "boot" );
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

u8 cmd_dump( u8 *Buffer )
{
    PYGMYFILEVOLUME *pygmyVolume;
    u32 i, ulAddress, ulReadLen;
    
    u8 ucData, ucLen, *ucParams[3];

    pygmyVolume = fileGetCurrentVolume( );
    ucLen = getAllSubStrings( Buffer, ucParams, 3, WHITESPACE|NEWLINE );
    if( !pygmyVolume || ucLen < 2 ){
        return( FALSE );
    } // if
    
    if( isStringSameIgnoreCase( ucParams[ 0 ], "sector" ) ){
        ulAddress = convertStringToInt( ucParams[ 1 ] );
        ulReadLen = 4096;
    } else{
        ulAddress = convertStringToInt( ucParams[ 0 ] );
        ulReadLen = convertStringToInt( ucParams[ 1 ] );
    } // else
    print( STDIO, "\r" );
    for( i = 0; i < ulReadLen; i++ ){
        ucData = pygmyVolume->IO->GetChar( pygmyVolume->Port, ulAddress + i );
        if( !(i % 16 ) ){
            print( STDIO, "\r" );
        } // if
        if( isPrintable( ucData ) ){
            print( STDIO, "\"%c\"", ucData );
        } else{
            print( STDIO, "(%02X)", ucData );
        } // else
    } // for

    return( TRUE );
}

u8 cmd_erase( u8 *Buffer )
{
    fpecEraseProgramMemory( );// 8, SIZEREG->Pages - 2 );
    globalStatus |= BIT0; // Mark main memory as erased

    return( 1 );
}

u8 cmd_format( u8 *Buffer )
{
    //fileFormat( globalMountPoints[ 0 ], "nebula" );
    //fileMount( (PYGMYMEMIO *)&SST25VF, 0, "nebula", FLASH_CS, FLASH_SCK, FLASH_MISO, FLASH_MOSI );
    u32 Address;
    u16 i;
    u8 Len, *Params[ 8 ];
    u8 Pins[ 8 ], Force; // *Name;

    Len = getAllSubStrings( Buffer, Params, 8, WHITESPACE|NEWLINE );
    if( Len == 0 ){
        return( FALSE );
    } // if
    if( !strcmp( Params[ 0 ], "--force" ) ){
        print( STDIO, "\rForce Format" );
        Force = 1;
    } else{
        Force = 0;
    } // else
    
    for( i = 0; i < Len - Force; i++ ){
        Pins[ i ] = convertStringToPin( Params[ Force + i ] );
        if( Pins[ i ] == 0xFF ){
            return( FALSE );
        } // if
        print( STDIO, "\r%s, %d", Params[ i + Force ], Pins[ i ] );
    } // for
    //fileFormat( (PYGMYMEMIO *)&S25FL, Force, Pins[0], Pins[1], Pins[2], Pins[3] );
    fileFormat( (PYGMYMEMIO *)&SST25VF, Force, Pins[0], Pins[1], Pins[2], Pins[3] );
    //fileFormat( (PYGMYMEMIO *)&STM32MEMIO, Force, Pins[0], Pins[1], Pins[2], Pins[3] );

    return( TRUE );
}

u8 cmdRFGet( u8 *Buffer )
{
   u8 *ucParam1, *ucParam2;
    
    ucParam1 = getNextSubString( Buffer, WHITESPACE|NEWLINE );
    ucParam2 = getNextSubString( NULL, WHITESPACE|NEWLINE );
    if( ucParam1 && ucParam2 ){
        //socketRequestFile( convertStringToInt( ucParam1 ), ucParam2 );
        
        return( TRUE );
    } // if

    return( FALSE );
}

u8 cmdRFPut( u8 *Buffer )
{
    u8 *ucParam1, *ucParam2;
    
    ucParam1 = getNextSubString( Buffer, WHITESPACE|NEWLINE );
    ucParam2 = getNextSubString( NULL, WHITESPACE|NEWLINE );
    if( ucParam1 && ucParam2 ){
        //socketSendFile( convertStringToInt( ucParam1 ), ucParam2 );
        
        return( TRUE );
    } // if

    return( FALSE );
}

u8 cmd_rx( u8 *Buffer )
{
    Buffer = removeLeadingWhitespace( Buffer );
    xmodemEnable( &XModem );
    
    return( xmodemRecv( &XModem, Buffer ) );
}

u8 cmd_tx( u8 *Buffer )
{
    Buffer = removeLeadingWhitespace( Buffer );
    xmodemEnable( &XModem );

    return( xmodemSend( &XModem, Buffer ) );
}

u8 cmd_read( u8 *Buffer )
{
    u16 i;
    u8 *ucSubString, ucChar;
    
    ucSubString = getNextSubString( Buffer, WHITESPACE|NEWLINE );
    if( ucSubString ){//&& fileOpen( &pygmyFile, ucSubString, READ ) ){
        for( i = 0; !(pygmyFile.Properties.Attributes & EOF ); i++ ){
            ucChar = fileGetChar( &pygmyFile );
            if( !(pygmyFile.Properties.Attributes & EOF ) ){
                if( ucChar == '\r' ){
                    i = 0;
                } // if
                if( !( i % 80 ) ){
                    //putcUSART3( '\r' );
                    print( STDIO, "\r" );
                } // if
                if( ucChar != '\r' && ( ucChar < 32 || ucChar > 126 ) ){
                    //putcUSART3( '(' );
                    //putIntUSART3( ucChar );
                    //putcUSART3( ')' );
                    print( STDIO, "(%d)", ucChar );
                } else{
                    //putcUSART3( ucChar );
                    print( STDIO, "%c", ucChar );
                } // else
            } // if
        } // 
        return( 1 );
    } // if

    return( 0 );
}

u8 cmd_reset( u8 *Buffer )
{
    // Call for a system level reset
    PYGMY_RESET;
    return( TRUE ); // Will never actually reach this point
}

u8 cmd_echo( u8 *Buffer )
{
    PYGMYFILE *File = NULL;
    u8 *TmpBuffer, *Text, *Op, *FileName;
    
    Buffer = removeLeadingWhitespace( Buffer );
    // Allocate a dynamic string
    TmpBuffer = malloc( 1 + strlen( Buffer ) ); 
    if( !TmpBuffer ){
        return( FALSE );
    } // if
    strcpy( TmpBuffer, Buffer );
    Text = strtok( TmpBuffer, "\"" );
    //Text = strtok( NULL, "\"" );
    Op = strtok( NULL, " " );
    FileName = strtok( NULL, NULL );
    print( STDIO, "\rText: %s", Text );
    print( STDIO, "\rOp: %s", Op );
    print( STDIO, "\rFileName: %s", FileName );
    if( Op  ){
        fileOpen( FileName, WRITE, 0 );
        fileClose( File );
    } // if
    //if(  ){

    //} //if
    if( File ){
        fileClose( File );
    } // if
    free( FileName );

    return( TRUE );
}

u8 cmd_cat( u8 *Buffer )
{
    // cat fileout filein1 > filein2 (append filein2 to filein1 and create fileout)
    // cat > filename (write new)
    // cat >> filename (append)
    // cat filename (read)
    // cat -n filename (read with numbered output)
    // Example: cat >> "thisfile.txt" "This is text to append"
    //PYGMYFILEVOLUME *Volume;
    PYGMYFILE *fileCatOut, *fileCat1, *fileCat2;
    PYGMYPARAMLIST Parameters;
    u16 i, ii, len;
    u8 TmpChar, *TmpString;

   
    print( STDIO, "\rCalling cat with: %s", Buffer );
    if( getAllParameters( Buffer, &Parameters ) ){
        print( STDIO, "\rParams:" );
        for( i = 0; i < Parameters.ParamCount; i++ ){
            print( STDIO, "\r%s", Parameters.Params[ i ] );
        } // for
        print( STDIO, "\r\rSwitches:" );
        for( i = 0; i < Parameters.SwitchCount; i++ ){
            print( STDIO, "\r%s", Parameters.Switches[ i ] );
        } // for

        if( !strcmp( Parameters.Params[ 0 ], ">" ) ){
            // Write new
            print( STDIO, "\rWriting new" );
            len = 0;
            if( Parameters.ParamCount > 1 ){
                len = strlen( Parameters.Params[ 1 ] );
            } // if
            fileCatOut = fileOpen( Parameters.Params[ 1 ], WRITE, len );
            if( !fileCatOut ){
                print( STDIO, "\rCalling freeParameterList()" );
                freeParameterList( &Parameters );
                return( FALSE );
            } // if
            if( Parameters.ParamCount > 1 ){
                print( STDIO, "\rWriting: %s", Parameters.Params[ 1 ] );
                fileWriteBytes( fileCatOut, Parameters.Params[ 1 ], strlen( Parameters.Params[ 1 ] ) );
            } // if
            fileClose( fileCatOut );
        } else if( !strcmp( Parameters.Params[ 0 ], ">>" ) ){
            // Append to file, create if it doesn't exist 
            print( STDIO, "\rAppend to file" );
            fileCatOut = fileOpen( Parameters.Params[ 1 ], WRITE|APPEND, len );
            if( !fileCatOut ){
                print( STDIO, "\rCalling freeParameterList()" );
                freeParameterList( &Parameters );
                return( FALSE );
            } // if
            if( Parameters.ParamCount > 1 ){
                print( STDIO, "\rAppend: %s", Parameters.Params[ 2 ] );
                fileWriteBytes( fileCatOut, Parameters.Params[ 2 ], strlen( Parameters.Params[ 2 ] ) );
            } // if
            fileClose( fileCatOut );
        } else {
            // Concatenate, read, or read numbered
            print( STDIO, "\rConcatenate, read, read numbered" );
            //if( Parameters.ParamCount < 2 ){
            //    return( FALSE );
            //}/ // if
            //print( COM3, "\rCalling strcmp() with: %s, %s", Parameters.Params[ 1 ], ">" );
            if( Parameters.ParamCount > 1 && !strcmp( Parameters.Params[ 1 ], ">" ) ){
                print( STDIO, "\rConcatenate files" );
                // concatenate one file to another
                if( Parameters.ParamCount < 4 ){
                    print( STDIO, "\rCalling freeParameterList()" );
                    freeParameterList( &Parameters );
                    return( FALSE );
                } // if
                if( !strcmp( Parameters.Params[ 1 ], ">" ) ){
                    fileCatOut = fileOpen( Parameters.Params[ 0 ], WRITE, 0 );
                    if( !fileCatOut ){
                        print( STDIO, "\rCalling freeParameterList()" );
                        freeParameterList( &Parameters );
                        return( FALSE ); // files failed to open
                    } // if
                    for( ii = 2; ii <  Parameters.ParamCount; ii++ ){
                        print( STDIO, " %s", Parameters.Params[ ii ] ); 
                        fileCat1 = fileOpen( Parameters.Params[ ii ], READ, 0 );
                        if( !fileCat1 ){
                            print( STDIO, "\rCalling freeParameterList()" );
                            freeParameterList( &Parameters );
                            return( FALSE ); // files failed to open
                        } // if
                    
                        for( i = 0; i < fileCat1->Properties.Length; i++ ){
                            TmpChar = fileGetChar( fileCat1 );
                            filePutChar( fileCatOut, TmpChar );
                        } // for
                        fileClose( fileCat1 );
                    } // for
                    print( STDIO, " to %s", Parameters.Params[ 0 ] ); 
                    fileClose( fileCatOut );
                    freeParameterList( &Parameters );
                    return( TRUE );
                } // if
                freeParameterList( &Parameters );
                return( FALSE );
            } else{
                // read file without numbering
                print( STDIO, "\rOpening: %s", Parameters.Params[ 0 ] );
                fileCatOut = fileOpen( Parameters.Params[ 0 ], READ, 0 );
                if( !fileCatOut ){
                    print( STDIO, "\rCalling freeParameterList()" );
                    freeParameterList( &Parameters );
                    return( FALSE );
                } // if
                print( STDIO, "\rListing" );
                for( i = 0; !fileEOF( fileCatOut ); i++ ){
                    TmpString = fileGetString( fileCatOut );
                    if( !TmpString ){
                        break;
                    } // if
                    if( Parameters.SwitchCount && !strcmp( Parameters.Params[ 0 ], "n" ) ){
                        // Read with numbered lines
                        print( STDIO, "\r%08d %s", TmpString );
                    } else{
                        print( STDIO, "\r%s", TmpString );
                    } // else
                    free( TmpString );
                } // for
                print( STDIO, "\rCalling fileClose():" );
                fileClose( fileCatOut );
            } // else
        } // else
        print( STDIO, "\rCalling freeParameterList()" );
        freeParameterList( &Parameters );
        return( TRUE);
    } // if
    
    freeParameterList( &Parameters );
    return( FALSE );
}

u8 cmd_rm( u8 *Buffer )
{   
    PYGMYFOLDER *ParentFolder;
    //u8 *FullPath;

    //Buffer = removeLeadingWhitespace( Buffer );
    //FullPath = malloc( 2 + len( fileGetCurrentPath() ) + len( Buffer ) );
    //strcpy( FullPath, fileGetCurrentPath( ) );
    //strcat( FullPath, "/" );
    //strcat( FullPath, Buffer );
    //print( COM3, "\rFullPath: %s", FullPath );
    //ParentFolder = fileAllocateFoldersFromFullPath( FullPath );
    //ParentFolder->Properties.Path = FullPath;
    //fileDeleteAdvanced( ParentFolder );
    fileDelete( Buffer );

    return( TRUE );
}

u8 cmd_strings( u8 *Buffer )
{
    PYGMYFILE *pygmyFile;
    u16 i;
    u8 *ucString;

    Buffer = removeLeadingWhitespace( Buffer );
    pygmyFile = fileOpen( Buffer, READ, 0 );
    if( pygmyFile ){
        for( i = 0; i < 20; i++ ){
            ucString = fileGetString( pygmyFile );
            if( ucString ){
                print( STDIO, "\r%s", ucString );
            } else{
                break;
            } // else
        } // for

        return( TRUE );
    } // if

    return( FALSE );
}

u8 cmd_append( u8 *Buffer )
{
    PYGMYFILE *pygmyFile;
    u8 i;

    pygmyFile = fileOpen( removeLeadingWhitespace( Buffer ), WRITE|READ|APPEND, 0 );

    if( pygmyFile ){
        for( i = 0; i < 64; i++ ){
            fileWriteBytes( pygmyFile, "Testing Append String Data Len.\r", sizeof( "Testing Append String Data Len.\r" ) );//len( "Testing Append...\r" ) );
        } // for
        fileClose( pygmyFile );
        return( TRUE );
    } // if
    
    return( FALSE );
}

u8 cmd_open( u8 *ucBuffer )
{
    PYGMYFILE *pygmyFile;
    u16 i;
    u8 ucString[ 40 ], ucLen, *ucParams[ 3 ];
    
    pygmyFile = fileOpen( removeLeadingWhitespace( ucBuffer ), WRITE|READ, 0 );
    
    //
    if( pygmyFile ){
        fileWriteBytes( pygmyFile, "Testing 123...\r", len( "Testing 123...\r" ) );
        fileClose( pygmyFile );
        return( TRUE );
    } // if
    
    return( FALSE );
}

u8 cmd_new( u8 *Buffer )
{
    PYGMYFILE *File;
    u8 *Name;

    Name = removeLeadingWhitespace( Buffer );
    File = fileOpen( Name, READ|WRITE, 0 );
    if( File ){
        return( TRUE );
    } // if

    return( FALSE );
}

u8 cmd_touch( u8 *Buffer )
{
    PYGMYFILE *File;
    u8 *Name;

    Name = removeLeadingWhitespace( Buffer );
    File = fileOpen( Name, READ, 0 );
    if( File ){
        if( fileDeleteEntry( File->Properties.ID, File->ParentFolder ) ){
            File->Properties.Time = timeGet();
            if( fileWriteEntry( File->ParentFolder, &File->Properties ) ){
                // ToDo: Add code to free file
                fileClose( File );
                return( TRUE );
            } // if
        } // if
        fileClose( File );
    } // 
    
    return( FALSE );
}

u8 cmd_mkdir( u8 *ucBuffer )
{
    PYGMYFILE *pygmyFolder;

    pygmyFolder = fileOpen( ucBuffer, FOLDER|WRITE|READ, 0 );
    if( pygmyFolder ){
        free( pygmyFolder ); // memory is unused unless folder is endpoint of path
        return( TRUE );
    } 
    
    return( FALSE );
}

u8 cmd_rmdir( u8 *Buffer )
{
    PYGMYFOLDER *ParentFolder;
    u8 *FullPath;

    Buffer = removeLeadingWhitespace( Buffer );
    FullPath = malloc( 2 + len( fileGetCurrentPath() ) + len( Buffer ) );
    strcpy( FullPath, fileGetCurrentPath( ) );
    strcat( FullPath, "/" );
    strcat( FullPath, Buffer );
    ParentFolder = fileAllocateFoldersFromFullPath( FullPath );
    ParentFolder->Properties.Path = FullPath;
    
    return( fileDeleteAdvanced( ParentFolder ) );
}

u8 cmd_cd( u8 *ucParams )
{
    u8 *ucParam;
    
    ucParam = removeLeadingWhitespace( ucParams );
    print( STDIO, "\rChanging active directory to: %s", ucParam );
    return( fileChangeCurrentPath( ucParam ) );
}

u8 cmd_ff( u8 *Buffer )
{
    //ToDo: Add code to find
}

u8 cmd_pwd( u8 *Buffer )
{
    // Print Working Directory

    print( COM3, "\r%s", fileGetCurrentPath() );

    return( TRUE );
}

u8 cmd_ls( u8 *Buffer )
{
    PYGMYFILEVOLUME *Volume;
    PYGMYFILELIST List;
    PYGMYFILEPROPERTIES Properties;
    PYGMYFOLDER *ParentFolder;
    u64 i, Len, Index, MaxFiles, StartSector, EndSector, EntryLen;
    u16 MaxVolumes;
    u8 Data, *Params[3];

    Len = getAllSubStrings( Buffer, Params, 3, WHITESPACE|NEWLINE );
    if( Len > 0 && isStringSame( Params[ 0 ], "--debug" ) ){
        // print raw output in hex format for debugging purposes
        StartSector = 0;
        EndSector = 0;
        if( Len == 2 ){
            StartSector = convertStringToInt( Params[ 1 ] );
        } else if( Len == 3 ){
            StartSector = convertStringToInt( Params[ 1 ] );
            EndSector = convertStringToInt( Params[ 2 ] );
        } // else if
        print( STDIO, "\r" );
        Volume = fileGetCurrentVolume();
        if( !Volume ){
            return( FALSE );
        } // if
        for( i = StartSector * Volume->Desc.SectorSize; i < EndSector * Volume->Desc.SectorSize; i++ ){
            Data = Volume->IO->GetChar( Volume->Port, i );
            if( !( i % 16 ) ){
                print( STDIO, "\r0x%012llX ", (u64)i );
            } // if
            if( isPrintable( Data ) ){
                print( STDIO, "  %c", Data );
            } else{
                print( STDIO, " %02X", (u8)Data );
            } //else
        } // for
    } else{
        Volume = fileGetCurrentVolume();
        if( !Volume ){
            print( STDIO, "\rVolumes:" );
            MaxVolumes = fileGetVolumeCount( );
            for( i = 0; i < MaxVolumes; i++ ){
                Volume = fileGetVolume( i );
                print( STDIO, "\r<MNT> %s", Volume->Properties.Name );
            } // for
            return( TRUE );
        } // if
        ParentFolder = fileAllocateFoldersFromFullPath( fileGetCurrentPath() );
        Index = 0;
        MaxFiles = ( ParentFolder->Properties.SectorCount * ParentFolder->Volume->Desc.SectorSize ) / PYGMYFILE_CHUNKLEN;
        for( i = 0; i < MaxFiles; i++ ){
            Index = fileFindNextID( ParentFolder, Index, &Properties );
            if( !Index ){
                break;
            } // if
            if( Properties.Attributes & FOLDER ){
                print( STDIO, "\r<DIR>\t%s %d", Properties.Name, Properties.Length );
            } else{
                // print filename, file length, and size of file on media
                print( STDIO, "\r%t %10d %10d %s", Properties.Time, Properties.Length, Properties.SectorCount * Volume->Desc.SectorSize, Properties.Name );
                //print( COM3, "\r% 10d % 10d %s", Properties.Length, Properties.SectorCount * Volume->Desc.SectorSize, "name" );// Properties.Name );

            } // else
        } // for
    } // else

    return( TRUE );
}

u8 cmd_cp( u8 *Buffer )
{
    PYGMYPARAMLIST Parameters;

    if( getAllParameters( Buffer, &Parameters ) ){
        fileCopy( Parameters.Params[ 0 ], Parameters.Params[ 1 ] );
        return( TRUE );
    } // if

    return( FALSE );
}

u8 cmd_mv( u8 *Buffer )
{
    PYGMYPARAMLIST Parameters;

    if( getAllParameters( Buffer, &Parameters ) ){
        if( Parameters.ParamCount == 2 ){
             fileRename( Parameters.Params[ 0 ], Parameters.Params[ 1 ] );

             return( TRUE );
        } // if
    } // if

    return( FALSE );
}

u8 cmd_boot( u8 *Params )
{
    u32 Address;
    u8 *Name, *AddressString;

    Params = removeLeadingWhitespace( Params );
    Name = getNextSubString( Params, WHITESPACE );
    AddressString = getNextSubString( NULL, NEWLINE );
    Address = convertBufferToU32( AddressString, len( AddressString ) );
    
    print( STDIO, "\rFlashing %s @ 0x%08X", Name, Address );

    bootTestAndLoad( Address, Name );

    PYGMY_RESET;
    
    // No return after reset
}

u8 executeCmd( u8 *ucBuffer, PYGMYCMD *pygmyCommands )
{
    u16 i;
    u8 *ucCommand;
    
    ucCommand = getNextSubString( ucBuffer, WHITESPACE | PUNCT );
        
    for( i = 0; 1; i++ ){ 
        if( isStringSame( "", pygmyCommands[ i ].Name ) ){
            return( FALSE );
        } // if
        if( isStringSame( ucCommand, pygmyCommands[ i ].Name ) ){
            if( pygmyCommands[ i ].Call( ucBuffer + len(ucCommand) ) ){
                return( TRUE );
            } else{
                return( FALSE );
            } // else
        }// if
    } // for

    return( 0 );
}

u8 cmd_find( u8 *Buffer )
{
    Buffer = removeLeadingWhitespace( Buffer );

    fileFind( fileGetCurrentVolume(), Buffer );

    return( TRUE );
}

u8 cmd_df( u8 *Buffer )
{
    // Display free disk space

    return( FALSE );
}

u8 cmd_du( u8 *Buffer )
{
    // Disk Usage - report the amount of disk space used by the specified files and for each subdirectory.

    return( FALSE );
}

u8 cmd_tail( u8 *Buffer )
{
    // Print a specified number of lines from the end of the specified file

    return( FALSE );
}

u8 cmd_cksum( u8 *Buffer )
{
    // generate and print a checksum for the string or file specified

    return( FALSE );
}

u8 cmd_if( u8 *Buffer )
{
    // call a command based on the condition of a variable

    return( FALSE );
}

u8 cmd_sleep( u8 *Buffer )
{
    // sleep for specified number of milliseconds

    return( FALSE );
}

u8 cmd_lsof( u8 *Buffer )
{
    // List open files

    return( FALSE );
}

u8 cmd_gawk( u8 *Buffer )
{
    // Find and Replace text within file(s)
    PYGMYPARAMLIST Parameters;
    PYGMYFILE *FileIn, *FileOut;
    u32 i, SearchLen, ReplaceLen, SearchStringIndex;
    u8 TmpChar, *SearchString, *FileName, *FileNameTo, *FileNameFrom;

    // This function calls another command and measures the time it takes to execute
    if( getAllParameters( Buffer, &Parameters )  ){
        print( STDIO, "\rParameters:" );
        for( i = 0; i < Parameters.ParamCount; i++ ){
            print( STDIO, "\r%d: %s", i, Parameters.Params[ i ] );
        } // for
        if( Parameters.ParamCount != 3 ){
            return( FALSE );
        } // if
        ReplaceLen = len( Parameters.Params[ 2 ] );
        SearchLen = len( Parameters.Params[ 1 ] );  // Length of string we are searching for, including null
        print( STDIO, "\rSearchLen: %d\rReplaceLen: %d", SearchLen, ReplaceLen );
        SearchString = malloc( SearchLen + 1 );             // Buffer to store string from file for comparison
        //TmpBuffer = malloc( SearchLen );              
        if( !SearchString ){
            return( FALSE );
        } // if
        FileName = malloc( len( Parameters.Params[ 0 ] ) + 2 );
        if( !FileName ){
            return( FALSE );
        } // if
        *FileName = '\0';
        strcpy( FileName, "%" );
        strcat( FileName, Parameters.Params[ 0 ] );
        FileIn = fileOpen( Parameters.Params[ 0 ], READ, 0 );
        FileOut = fileOpen( FileName, WRITE, FileIn->Properties.Length );
        if( !FileIn || !FileOut ){
            return( FALSE );
        } // if
       
        // match one char at a time and copy the chars into a buffer
        // As soon as one char matches, write the buffer
        print( STDIO, "\rTesting: %d", FileIn->Properties.Length );
        for( SearchStringIndex = 0; FileIn->Index < FileIn->Properties.Length; ){
            TmpChar = fileGetChar( FileIn );
            print( STDIO, "\rCompare: %c to %c", TmpChar, Parameters.Params[ 1 ][ SearchStringIndex ] );
            if( TmpChar == Parameters.Params[ 1 ][ SearchStringIndex ] ){
                // We have a match
                SearchString[ SearchStringIndex++ ] = TmpChar;
                print( STDIO, "\rFound char match: %c", TmpChar );
                if( SearchStringIndex == SearchLen ){
                    // The entire buffer is a match, output replacement and reset index
                    print( STDIO, "\rReplacing with %s", Parameters.Params[ 2 ] );
                    fileWriteBytes( FileOut, Parameters.Params[ 2 ], ReplaceLen );
                    SearchStringIndex = 0;
                } // if
            } else{
                // write the buffer and restart the search
                SearchString[ SearchStringIndex++ ] = TmpChar;
                fileWriteBytes( FileOut, SearchString, SearchStringIndex );
                SearchStringIndex = 0;
            } // else

        } // for
    } // if
    fileClose( FileIn );
    fileClose( FileOut );
    //free( FileName );
    //FileNameTo = fileCreateNameWithPath( Parameters.Params[ 0 ] );
    //FileNameFrom = fileCreateNameWithPath( FileName );
    fileDelete( Parameters.Params[ 0 ] );
    print( STDIO, "\rRenaming: %s to %s", FileName, Parameters.Params[ 0 ] );
    fileRename( FileName, Parameters.Params[ 0 ] );
    free( FileName );
    //free( FileNameTo );
    //free( FileNameFrom );

    return( TRUE );
}

u8 cmd_declare( u8 *Buffer )
{
    // Declare variables

    return( FALSE );
}

u8 cmd_dc( u8 *Buffer )
{
    // desktop calculator, solves equations from string or specified file
    
    return( FALSE );
}

u8 cmd_kill( u8 *Buffer )
{
    // kill the specified process

    return( FALSE );
}

u8 cmd_killall( u8 *Buffer )
{
    // kill all running processes
  
    return( FALSE );
}

u8 cmd_wait( u8 *Buffer )
{
    // wait until a process terminates and call the specified command

    return( FALSE );
} 

u8 cmd_watch( u8 *Buffer )
{
    // print the status of the specified process or file every specified number of milliseconds 

    return( FALSE );
}

u8 cmd_while( u8 *Buffer )
{
    // 

    return( FALSE );
}

u8 cmd_true( u8 *Buffer )
{
    // return true
    print( STDIO, "TRUE" );

    return( TRUE );
}

u8 cmd_false( u8 *Buffer )
{
    // return false
    print( STDIO, "FALSE" );

    return( FALSE );
}

u8 cmd_test( u8 *Buffer )
{
    // evaluate an expression and return true or false

    return( FALSE );
}

u8 cmd_man( u8 *Buffer )
{
    
}

/*void SysTick_Handler( void )
{
    PYGMY_WATCHDOG_REFRESH; 
    
    xmodemProcessTimer( &XModem );
}*/