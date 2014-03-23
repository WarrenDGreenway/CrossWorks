#include "pygmy_profile.h"
#include "eastrisingERC1602.h"

#define LCD_CLEAR                   0x01
#define LCD_HOME                    0x02
#define LCD_ENTRYMODE               0x04
#define LCD_ONOFF                   0x08
#define LCD_ONOFF_DISPLAY           0x04
#define LCD_ONOFF_CURSOR            0x02
#define LCD_ONOFF_CURSORPOS         0x01
#define LCD_SHIFT                   0x10
#define LCD_SHIFT_CURSOR            0x08
#define LCD_SHIFT_DIRECTION         0x04
#define LCD_FUNCTION                0x20
#define LCD_FUNCTION_BITS           0x10
#define LCD_FUNCTION_LINE           0x08
#define LCD_CGRAM                   0x40
#define LCD_DDRAM                   0x80
#define LCD_READBUSY                0x80
#define LCD_MAXCOLUMNS              0x10
#define LCD_MAXROWS                 0x02

void lcdWriteCommand( u8 cmd )
{
    u8 i;

    pinSet( LCD_SCL, LOW );
    pinSet( LCD_CS, LOW ); 
    pinSet( LCD_RS, LOW );

    for( i = 0; i < 8; i++ ){
        if ( cmd & 0x80 ){
            pinSet( LCD_SDA, HIGH );
        } else{
            pinSet( LCD_SDA, LOW ); 
        } // else
        pinSet( LCD_SCL, LOW );
        cmd <<= 1;
        pinSet( LCD_SCL, HIGH );      
    }
    pinSet( LCD_CS, HIGH ); 
}

void lcdWriteData( u8  dat )
{
    u8 i;
     
    pinSet( LCD_SCL, LOW );
    pinSet( LCD_CS, LOW );
    pinSet( LCD_RS, HIGH );
    for( i = 0; i < 8; i++ ){
        if ( dat & 0x80 ){
            pinSet( LCD_SDA, HIGH );
        } else{
            pinSet( LCD_SDA, LOW );
        } //else
        pinSet( LCD_SCL, LOW );
        dat <<= 1;
        pinSet( LCD_SCL, HIGH );      
    } // for
    pinSet( LCD_CS, HIGH );
}

void lcdWriteAddress( u8 column, u8 row )
{
    if( column >= LCD_MAXCOLUMNS ){
        column = 0;
        ++row;
    } // if
    if( row >= LCD_MAXROWS ){
        row = 0;
    } // if
    if( row == 0 ){
        lcdWriteCommand( LCD_DDRAM + column );
    } else{
        lcdWriteCommand( 0xC0 + column );
    } // else
}

void lcdInit( void )
{	
    pinConfig( LCD_SDA, OUT );
    pinConfig( LCD_SCL, OUT );
    pinConfig( LCD_RS, OUT );
    pinConfig( LCD_CS, OUT );
    pinConfig( LCD_PSB, OUT );
    pinConfig( LCD_PSI2B, OUT );
    pinConfig( LCD_BL, OUT );
    pinSet( LCD_BL, HIGH );
    pinSet( LCD_RS, HIGH );
    pinSet( LCD_CS, HIGH );
    // SPI
    pinSet( LCD_PSB, LOW );
    pinSet( LCD_PSI2B, HIGH ); 

    lcdWriteCommand( 0x38 );   //FUNCTION SET 8 bit,N=1 2-line display mode,5*7dot	
    delay( 100 );	
    lcdWriteCommand( 0x39 );  //FUNCTION SET 8 bit,N=1 2-line display mode,5*7dot IS=1
    delay( 100 );	
    lcdWriteCommand( 0x1c );   //Internal OSC frequency adjustment 183HZ    bias will be 1/4 
    delay( 100 );	
    lcdWriteCommand( 0x73 );    //Contrast control  low byte
    delay( 100 );	
    lcdWriteCommand( 0x57 );    //booster circuit is turn on.    /ICON display off. /Contrast control   high byte
    delay( 100 );	
    lcdWriteCommand( 0x6c );  //Follower control
    delay( 1000 );	
    lcdWriteCommand( 0x0c );    //DISPLAY ON
    delay( 100 );	
    lcdWriteCommand( 0x01 );   //CLEAR DISPLAY
    delay( 10000 );	
    lcdWriteCommand( 0x06 );   //ENTRY MODE SET  CURSOR MOVES TO RIGHT
    delay( 100 );	
}

u8 putsLCD( u8 *Buffer )
{
    for( ; *Buffer; ){
        lcdWriteData( *(Buffer++) );
    } // for
}

u8 putcLCD( u8 c )
{
    lcdWriteData( c );
}
