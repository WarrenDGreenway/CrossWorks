/**************************************************************************
    PygmyOS ( Pygmy Operating System )
    Copyright (C) 2011  Warren D Greenway

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

// This file is intended to be used as a template
// To use, copy into your project folder and:
// Add the board specific header ( if needed )
// Add any defines for your specific project 
// Include at the top of main.c

#pragma once

//#define __PYGMYNEBULA
#define __PYGMYSTREAMS
    #define __PYGMYSTREAMCOM4
    #define __PYGMYSTREAMCOM3
    #define __PYGMYSTREAMCOM2
    #define __PYGMYSTREAMCOM1
//#define __PYGMYCOMMANDS
//#define __PYGMYMESSAGES
#define __PYGMYTASKS
//#define __PYGMYSOCKETS
    //#define __PYGMYSOCKETS_CONTROL
//    #define __PYGMYSOCKETS_FILE
    //#define __PYGMYSOCKETS_CMDLINE
    //#define __PYGMYSOCKETS_AV
#define __PYGMYFILES
#define __PYGMYANALOG
//#define __PYGMY_DEBUG_FILE
//#include "profiles/nebula/pygmy_nebula_hp.h"
#include "gomex_v1.h"

#include "pygmy_sys.h"
#include "pygmy_type.h"
#include "pygmy_port.h"
#include "pygmy_com.h"
#include "pygmy_rf.h"
#include "pygmy_adc.h"
#include "pygmy_pfat.h"
//#include "pygmy_file.h"
//#include "pygmy_lcd.h"
//#include "pygmy_gui.h"
#include "pygmy_nvic.h"
#include "pygmy_rtc.h"
#include "pygmy_string.h"
#include "pygmy_fpec.h"
//#include "pygmy_socket.h"
#include "pygmy_xmodem.h"
//#include "profiles/memory/stm32.h"
#include "profiles/memory/sst25vf.h"
//#include "profiles/memory/s25flxxx.h"


//-------------------------------------------------------------------------------------
// Place any user defines below--------------------------------------------------------
//-------------------------------------------------------------------------------------


