
#ifndef LOGITECH_H
#define LOGITECH_H

#define BTN_INCSPEED -1
#define BTN_DECSPEED -2

#define BTN_TURNLEFT -3
#define BTN_TURNRIGHT -4

#define BTN_ASCEND 3
#define BTN_DESCEND 2

#define BTN_EMERGSTOP -7
#define BTN_ZEROSPEED -8

#define BTN_ARM_TORPEDO 10
#define BTN_ARM_MARKER 11
#define BTN_FIRE 0

#define AXIS_TSPEED 0
#define AXIS_SPEED 1
#define AXIS_YAW 2
#define AXIS_THROTTLE -1 /* to enable, use 3 */

int scaleAxis(int val, int negRange, int posRange, int offset, int outRange)
{
    val += offset;
    if (val > 0) { /* Positive */
        val = outRange * val / posRange;
    } else {
        val = -outRange * val / negRange;
    }

    if (val > outRange)
        val = outRange;

    if (val < -outRange)
        val = -outRange;

    return val;
}

#define SCALE_SPEED(val)                        \
    val = scaleAxis(val, 32767, -32767, 0, SPEED_RANGE)
#define SCALE_YAW(val)                          \
    val = scaleAxis(val, -32767, 32767, 0, YAW_RANGE)
#define SCALE_TSPEED(val)                       \
    val = scaleAxis(val, -32767, 32767, 0, TSPEED_RANGE)
#define SCALE_THROTTLE(val)                     \
    val = scaleAxis(val, 32767, -32767, 0, SPEED_RANGE)

#include "joystick.h"

#define INFO_MESSAGE                            \
    "Using Logitech mapping\n"

#endif // LOGITECH_H
