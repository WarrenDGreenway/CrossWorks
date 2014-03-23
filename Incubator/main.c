//#include <__cross_studio_io.h>

#include "pygmy_profile.h"
//#include "eastrisingERC1602.h"

volatile u32 globalPLL, globalID, globalXTAL, globalFreq;

void main(void)
{
    float rh, temp;

    RCC->APB2ENR |= (RCC_IOPBEN|RCC_IOPCEN|RCC_IOPAEN);

    PYGMY_RCC_HSI_ENABLE;
    PYGMY_RCC_HSE_ENABLE;
    while( !PYGMY_RCC_HSE_READY );
    RCC->CFGR2 = 0;
    RCC->CFGR = BIT16|BIT1;
    PYGMY_RCC_PLL_ENABLE;
    while( !PYGMY_RCC_PLL_READY );

    PYGMY_RCC_GPIOA_ENABLE;
    PYGMY_RCC_GPIOB_ENABLE; 
    PYGMY_RCC_GPIOC_ENABLE;
    PYGMY_RCC_TIMER1_ENABLE;
    //ulClock = globalFreq;  
    NVIC->ISER[ 1 ] = 0x00000001 << 7;
    NVIC->ISER[ 1 ] = 0x00000001 << 6;
    NVIC->ISER[ 1 ] = 0x00000001 << 5;
    SYSTICK->VAL = PYGMY_MAX_XTAL / 1000;
    SYSTICK->LOAD = PYGMY_MAX_XTAL / 1000; // Based on  ( 2X the System Clock ) / 1000
    SYSTICK->CTRL = 0x07;   // Enable system timer
    // End Configure Interrupts
    WATCHDOG_UNLOCK;
    WATCHDOG_PRESCALER( IWDT_PREDIV128 );
    WATCHDOG_TIMER( 0x0FFF );
    WATCHDOG_START;
    WATCHDOG_REFRESH;

    pinConfig( LIGHT, OUT );
    
    lcdInit(  );
    si7020Init( );
    si7020GetRegister( );
    //rh = si7020GetRH( );
    //temp = si7020GetTemperature( );
    
    //lcdWriteAddress( 0, 0 );
    //delay( 60 );
    //print( "temp: %f", temp );
    //lcdWriteAddress( 0, 1 );
    //print( "rh: %f", rh );
    //putsLCD( "Hello" );
    //lcdWriteAddress( 0, 1 );
    //putsLCD( "World" );
    
}

void delayms( u32 Delay )
{
    delay( Delay * 1000 );
}

void delay( u32 Delay )
{
    // This function uses a general purpose timer to provide an accurate microsecond delay
    // MainClock must be set to 1MHz min, 8MHz or higher recommended
    
    //TIMER *pygmyTimer;
    u32 i;
    u32 Prescaler;
    
    WATCHDOG_REFRESH;
    Delay *= ( PYGMY_MAX_XTAL / 1000000 );
    if( Delay > 0x0000FFFF ){
        Prescaler = ( Delay >> 16 ) + 1;
        Delay = ( Delay / Prescaler );
    } // 
    if( Delay < 60 ){ 
        return; // Delay of under 60 microseconds not supported
    } // if 
    //if( pygmyGlobalData.DelayTimer == PYGMY_TIMER1 ){
        // F103LD, F103MD, F103HD
        // Warning! F103 devies with less than 768KB Flash do not have extra
        // multipurpose timers and must share Timer1. In this case, Timer1
        // should not be used for PWM output.
        PYGMY_RCC_TIMER1_ENABLE;
        TIM1->CR1 = 0;                          // Disable before configuring timer
        TIM1->CR2 = 0;                          //
        TIM1->SMCR = 0;                         //
        TIM1->DIER = 0;                         // DMA and interrupt enable register
        TIM1->CNT = 0;                          // Count Register
        TIM1->PSC = Prescaler;
        TIM1->ARR =  Delay; // Auto Reload Register
        TIM1->SR = 0;
        TIM1->CR1 = ( TIM_ARPE | TIM_OPM | TIM_CEN );      // Enable single shot count
        while( (TIM1->CR1 & TIM_CEN) );         // Wait for count to complete 
    /*} else {
        //
        pygmyTimer = sysGetTimer( pygmyGlobalData.DelayTimer );
        PYGMY_RCC_TIMER9_ENABLE;
        pygmyTimer->CR1 = 0;                          // Disable before configuring timer
        pygmyTimer->CR2 = 0;                          //
        pygmyTimer->SMCR = 0;                         //
        pygmyTimer->DIER = 0;                         // DMA and interrupt enable register
        pygmyTimer->CNT = 0;                          // Count Register
        pygmyTimer->ARR =  Delay - 60; // Auto Reload Register
        pygmyTimer->PSC = Prescaler;
        pygmyTimer->SR = 0;
        pygmyTimer->CR1 = ( TIM_ARPE | TIM_OPM | TIM_CEN );      // Enable single shot count
        while(( pygmyTimer->CR1 & TIM_CEN ) );         // Wait for count to complete
           
    } // else
    */
    WATCHDOG_REFRESH;
}

void print( u8 *ucBuffer, ... )
{
    PYGMYTIME pygmyTime;
    u8 i, ii, ucValue[ 3 ], *sValue, sFormat[ 16 ], ucIntBuffer[ 40 ], iType;
    u16 uiLen;
    va_list ap;
     
    if( ucBuffer == NULL ){
        return;
    } // if
    ucValue[ 2 ] = 0;
    va_start( ap, ucBuffer );
    for( ; *ucBuffer; ucBuffer++ ){
        
        if( *ucBuffer == '%' ){ // Found format specifier
            // first collect precision, if any
            for( i = 0; i < 12 && ( isNumeric( *(++ucBuffer) ) || *ucBuffer == '-' 
                || *ucBuffer == '+' || *ucBuffer == '.' ); i++ ){
                sFormat[ i ] = *ucBuffer;
            } // for
            sFormat[ i ] = 0; // Terminate format string
            if( *ucBuffer == '%' ){ // Format specifier was only escaping '%' char, print
                ucValue[ 0 ] = '%';
                putsLCD( ucValue );
            } else if( *ucBuffer == 'c' ){ 
                ucValue[ 0 ] = (u8)va_arg( ap, int );
                putsLCD( ucValue );
            } else if( *ucBuffer == 's' ){
                sValue = va_arg( ap, u8 * );
                uiLen = len( sValue );
                
                ucValue[ 0 ] = ' ';
                if( sFormat[ 0 ] == '-' ){
                    putsLCD( sValue );
                    ii = convertStringToInt( (u8*)(sFormat+1) );
                    if( ii > uiLen ){
                        ii -= uiLen;
                    } else{
                        ii = 0;
                    } // else
                    for( i = 0; i < ii; i++ ){
                         putsLCD( ucValue );
                    } // for
                } else{
                    ii = convertStringToInt( sFormat );
                    if( ii > uiLen ){
                        ii -= uiLen;
                    } else{
                        ii = 0;
                    } // if
                    for( i = 0; i < ii; i++ ){
                         putsLCD( ucValue );
                    } // for
                    putsLCD( sValue );
                } // else
            } else if( *ucBuffer == 'i' || *ucBuffer == 'd' ||
                *ucBuffer == 'x' || *ucBuffer == 'X' || *ucBuffer == 'o' || *ucBuffer == 'f' || *ucBuffer == 'l' ){
                iType = 16;
                if( *ucBuffer == 'l' ){
                    ++ucBuffer;
                    if( *ucBuffer == 'l' ){
                        ++ucBuffer;
                        iType = 64;
                    } // if
                } // if
                sFormat[ i++ ] = *ucBuffer; // PrintInteger requires format char
                sFormat[ i ] = 0; // terminate at new index
                
                if( iType == 64 ){
                    convertIntToString( va_arg( ap, u64 ), sFormat, ucIntBuffer ); 
                } else if( *ucBuffer == 'f' ){
                    convertFloatToString( va_arg( ap, double ), sFormat, ucIntBuffer );
                } else{
                    convertIntToString( va_arg( ap, u32 ), sFormat, ucIntBuffer ); 
                } // else
                putsLCD( ucIntBuffer );
            //} else if( *ucBuffer == 'f' ){
                
            } /*else if( *ucBuffer == 't' ){
                convertSecondsToSystemTime( va_arg( ap, s32 ), &pygmyTime );
                print( "%04d-%02d-%02d %02d:%02d:%02d", pygmyTime.Year,pygmyTime.Month,pygmyTime.Day,
                    pygmyTime.Hour,pygmyTime.Minute,pygmyTime.Second );
            }*/
        } else{
            ucValue[ 0 ] = *ucBuffer;
            ucValue[ 1 ] = 0;
            putsLCD( ucValue );
        }
    } // for
    va_end( ap );
}

void SysTick_Handler( void )
{
    WATCHDOG_REFRESH;  
}