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
/**************************************************************************
This version of the Pygmy xmodem interface has been tailored for pboot,
Pygmy's light-weight boot loader for systems without a file system and
limited RAM. Some of the limitations are that it only supports RECV operations
and writes to flash (or a buffer) through calls to the Pygmy Hex utility.


***************************************************************************/

void xmodemInit( PYGMYXMODEM *XModem )
{
    XModem->Status = 0;
    XModem->Timeout = 0;
    XModem->Transaction = 0;
    XModem->Failures = 0;
    XModem->Count = 0;
    XModem->Index = 0;
    XModem->Port = 0;
    XModem->Enabled = FALSE;

}

void xmodemEnable( PYGMYXMODEM *XModem )
{
    XModem->Enabled = TRUE;
}

void xmodemDisable( PYGMYXMODEM *XModem )
{
    XModem->Enabled = FALSE;
}

void xmodemRecv( PYGMYXMODEM *XModem, u8 *FileName )
{
    // This version of xmodem is designed for the light-weight pboot bootloader
    // As such, there is no filesystem support in this version
    
    XModem->Status |= XMODEM_ACTIVE;       // BIT1 used to mark In XModem Status
    XModem->Timeout = 1000; // 10 seconds
    XModem->Transaction = 60;
    XModem->Count = 1;
    // Call init for the interface handling the data processing
    XModem->Init();
}

u8 xmodemProcess( PYGMYXMODEM *XModem, u8 Byte )
{
    // returns TRUE if xmodem active and processing char, else FALSE
    if( XModem->Status & XMODEM_RECV ){ // RECV      
        if( XModem->Status & XMODEM_MODE_SOH ){ // Recieving Payload, after <SOH>
            XModem->Buffer[ XModem->Index++ ] = Byte;
            if( XModem->Index == 131 ){
                XModem->Status &= ~XMODEM_MODE_SOH; // Reset Packet Marker
                if( XModem->Count == XModem->Buffer[ 0 ] && xmodemWritePacket( XModem ) ) { // Returns 1 if check sum correct, 0 if not
                    if( XModem->Count + 1 == 256 ){
                        XModem->Count = 0;
                    } else{
                        ++XModem->Count;
                    } // else
                    XModem->Failures = 0; // Reset failure counter, counter starts with each ACK
                    XModem->Putc( XMODEM_ACK );
                    //comPrint( print( STDIO, "%c", XMODEM_ACK );
                } else{
                    print( STDIO, "%c", XMODEM_NACK );
                } // else  
            } 
        } else{
            if( Byte == XMODEM_SOH ){ // This is XModem <SOH> // was 0x01
                XModem->Status |= XMODEM_MODE_SOH; // Set Packet Marker
                XModem->Timeout = 1000;
                XModem->Transaction = 10;
            } else if( Byte == XMODEM_EOT || Byte == XMODEM_CAN ){ // 0x04 Marks End Of Transmission   
                // Call xmodemInit() to reset the XModem status bits
                xmodemInit( XModem );
                // Push the last of the data to the Pygmy hex utility
                hexProcessBuffer( XModem->Buffer, 128 );
                //The entire binary has been loaded, send ACK and exit
                XModem->Putc( XMODEM_ACK );
            } // else if
            XModem->Index = 0;
        } // else

        return( TRUE );
    } 
    
    return( FALSE );
}

void xmodemProcessTimer( PYGMYXMODEM *XModem )
{
    if( XModem->Status & XMODEM_RECV ){
        if( XModem->Timeout ){
            --XModem->Timeout;
        } else{
            ++XModem->Failures;
            if( XModem->Failures >= XMODEM_MAXFAILURES ){
                // Call xmodemInit() to reset the XModem status bits
                xmodemInit( XModem );
                // There are too many failures to continue, send the cancellation byte
                XModem->Putc( XMODEM_CAN );
                //print( STDIO, "%c", XMODEM_CAN );
                return;
            } // if
            // We have had a failure, but not 
            //print( STDIO, "%c", XMODEM_NACK );
            XModem->Timeout = 1000;
            if( XModem->Transaction ){
                --XModem->Transaction;
            } else{
                XModem->Status &= ~(XMODEM_RECV|XMODEM_MODE_SOH);
                // Timeout
            } // else
        } // else
    } else if( XModem->Status & XMODEM_SEND ){
        if( XModem->Timeout )
            --XModem->Timeout;
        else{
            ++XModem->Failures;
            if( XModem->Failures >= XMODEM_MAXFAILURES ){
                // Call xmodemInit() to reset the XModem status bits
                xmodemInit( XModem );
                //XModem->Status = 0;
                XModem->Putc( XMODEM_CAN );
                //print( STDIO, "%c", XMODEM_CAN );
                return;
            } // if
            xmodemSendPacket( XModem, XMODEM_LAST );
            XModem->Timeout = 1000;
            if( XModem->Transaction )
                --XModem->Transaction;
            else{
                XModem->Status &= ~(XMODEM_SEND|XMODEM_SEND_EOT);
                //putstr( (u8*)BOOT_ERROR );
            } // else
        } // else
    } // else if
}

u8 xmodemWritePacket( PYGMYXMODEM *XModem )
{
    u8 i, Sum, *Buffer;

    // count ( 0-1 before calling this function ) must be validated by calling function 
    // 0-127 are data bytes, 128 is checksum 
    Buffer = XModem->Buffer+2;
    Sum = Buffer[ 0 ];
    for( i = 1; i < 128; i++ ){
        Sum += Buffer[ i ]; 
    } // for  
    if( Sum == Buffer[ 128 ] ){
        //filePutBuffer( XModem->File, Buffer, 128 );
        XModem->ProcessPacket( Buffer, 128 );
        return( TRUE );
    } // if
	
    return( FALSE );
}
