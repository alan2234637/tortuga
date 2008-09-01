/*
 * Copyright (C) 2008 Robotics at Maryland
 * Copyright (C) 2008 Joseph Lisee <jlisee@umd.edu>
 * All rights reserved.
 *
 * Author: Joseph Lisee <jlisee@umd.edu>
 * File:  packages/control/include/IRotationalController.h
 */

#ifndef RAM_CONTROL_IROTATIONALCONTROLLER_08_31_2008
#define RAM_CONTROL_IROTATIONALCONTROLLER_08_31_2008

// Library Includes
#include <boost/shared_ptr.hpp>

// Must Be Included last
#include "control/include/Export.h"

namespace ram {
namespace control {

class IRotationalController;
typedef boost::shared_ptr<IRotationalController> IRotationalControllerPtr;
    
class RAM_EXPORT IRotationalController
{
public:
    virtual ~IRotationalController() {}
    
    /** Yaws the desired vehicle state by the desired number of degrees */
    virtual void yawVehicle(double degrees) = 0;

    /** Pitches the desired vehicle state by the desired number of degrees */
    virtual void pitchVehicle(double degrees) = 0;

    /** Rolls the desired vehicle state by the desired number of degrees */
    virtual void rollVehicle(double degrees) = 0;

    /** Gets the current desired orientation */
    virtual math::Quaternion getDesiredOrientation() = 0;
    
    /** Sets the current desired orientation */
    virtual void setDesiredOrientation(math::Quaternion) = 0;
    
    /** Returns true if the vehicle is at the desired orientation */
    virtual bool atOrientation() = 0;

    /** Loads current orientation into desired (fixes offset in roll and pitch) */
    virtual void holdCurrentHeading() = 0;
};
    
} // namespace control
} // namespace ram

#endif // RAM_CONTROL_IROTATIONALCONTROLLER_08_31_2008