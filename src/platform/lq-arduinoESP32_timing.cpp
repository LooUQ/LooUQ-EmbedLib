/** ***************************************************************************
  @file lq-arduinoESP32_timing.cpp
  @brief LooUQ embeded timing abstraction for ESP32 under Arduino framework.

  @author Greg Terrell, LooUQ Incorporated

  \loouq

  @warning Internal dependencies, changes only as directed by LooUQ staff.

-------------------------------------------------------------------------------

LooUQ-LTEmC // Software driver for the LooUQ LTEm series cellular modems.
Copyright (C) 2017-2023 LooUQ Incorporated

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
Also add information on how to contact you by electronic and paper mail.

**************************************************************************** */


#ifdef ARDUINO_ARCH_ESP32

#include "lq-platform_timing.h"
#include <Arduino.h>

platform_yieldCB_func_t platform_yieldCB_func;


uint32_t lqMillis()
{
    return millis();
}


void lqYield()
{
    if (platform_yieldCB_func)          // allow for device application yield processing
        platform_yieldCB_func();
    else
        vTaskDelay(1);                  // ESP32: give core to next task
}


void lqDelay(uint32_t delay_ms)
{
    lqYield();                          // perform yield to allow for application processing       
    vTaskDelay(delay_ms);
}







uint32_t pMillis()
{
    return millis();
}


void pYield()
{
    if (platform_yieldCB_func)          // allow for device application yield processing
        platform_yieldCB_func();
    else
        vTaskDelay(1);                  // ESP32: give core to next task
}


void pDelay(uint32_t delay_ms)
{
    for (size_t i = 0; i < delay_ms; i++)
    {
        pYield();
        delay(1);
    }
}


bool pElapsed(uint32_t timerStart, uint32_t timerTimeout)
{
    return (timerStart == 0) ? 0 : millis() - timerStart > timerTimeout;
}

#endif