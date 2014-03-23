#include "pygmy_type.h"
#pragma once

void si7020Init( void );
float si7020GetTemperature( void );
float si7020GetRH( void );
void si7020Reset( void );
void si7020GetRegister( void );

