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

void xmodemInit( PYGMYXMODEM *XModem );
void xmodemEnable( PYGMYXMODEM *XModem );
void xmodemDisable( PYGMYXMODEM *XModem );
u8 xmodemRecv( PYGMYXMODEM *XModem, u8 *FileName );
u8 xmodemSend( PYGMYXMODEM *XModem, u8 *FileName );
u8 xmodemProcess( PYGMYXMODEM *XModem, u8 Byte );
void xmodemProcessTimer( PYGMYXMODEM *XModem );
u8 xmodemWritePacket( PYGMYXMODEM *XModem );
void xmodemSendPacket( PYGMYXMODEM *XModem, u8 ucLast );