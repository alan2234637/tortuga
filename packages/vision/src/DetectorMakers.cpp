/*
 * Copyright (C) 2008 Robotics at Maryland
 * Copyright (C) 2008 Joseph Lisee <jlisee@umd.edu>
 * All rights reserved.
 *
 * Author: Joseph Lisee <jlisee@umd.edu>
 * File:  packages/vision/src/DetectorMakers.cpp
 */

// All registration are done in one file to keep the compilation dependency on 
// the factory system to a minimum

// Project Includes
#include "vision/include/DetectorMaker.h"

#include "vision/include/RedLightDetector.h"
#include "vision/include/BuoyDetector.h"
#include "vision/include/BinDetector.h"
#include "vision/include/OrangePipeDetector.h"
#include "vision/include/GateDetector.h"
#include "vision/include/FeatureDetector.h"
#include "vision/include/SuitDetector.h"
#include "vision/include/BasicWW2Detector.h"
#include "vision/include/FANNSuitDetector.h"
#include "vision/include/FANNWW2Detector.h"
#include "vision/include/BasicWeaponDetector.h"
#include "vision/include/FANNWeaponDetector.h"
#include "vision/include/DuctDetector.h"
#include "vision/include/DownwardDuctDetector.h"
#include "vision/include/SafeDetector.h"
#include "vision/include/AdaptiveThresher.h"
#include "vision/include/TargetDetector.h"
#include "vision/include/BarbedWireDetector.h"
#include "vision/include/HedgeDetector.h"
#include "vision/include/WindowDetector.h"
#include "vision/include/VelocityDetector.h"

namespace ram {
namespace vision {

static DetectorMakerTemplate<RedLightDetector>
registerRedLightDetector("RedLightDetector");

static DetectorMakerTemplate<BuoyDetector>
registerBuoyDetector("BuoyDetector");

static DetectorMakerTemplate<BinDetector>
registerBinDetector("BinDetector");

static DetectorMakerTemplate<OrangePipeDetector>
registerOrangePipeDetector("OrangePipeDetector");
    
static DetectorMakerTemplate<GateDetector>
registerGateDetector("GateDetector");

static DetectorMakerTemplate<FeatureDetector>
registerFeatureDetector("FeatureDetector");

static DetectorMakerTemplate<SuitDetector>
registerSuitDetector("SuitDetector");

static DetectorMakerTemplate<BasicWW2Detector>
registerBasicWW2Detector("BasicWW2Detector");
    
static DetectorMakerTemplate<FANNSuitDetector>
registerFANNSuitDetector("FANNSuitDetector");

static DetectorMakerTemplate<FANNWW2Detector>
registerFANNWW2Detector("FANNWW2Detector");

static DetectorMakerTemplate<BasicWeaponDetector>
registerBasicWeaponDetector("BasicWeaponDetector");

static DetectorMakerTemplate<FANNWeaponDetector>
registerFANNWeaponDetector("FANNWeaponDetector");
    
static DetectorMakerTemplate<DuctDetector>
registerDuctDetector("DuctDetector");

static DetectorMakerTemplate<DownwardDuctDetector>
registerDownwardDuctDetector("DownwardDuctDetector");

static DetectorMakerTemplate<SafeDetector>
registerSafeDetector("SafeDetector");

static DetectorMakerTemplate<AdaptiveThresher>
registerAdaptiveThresherDetector("AdaptiveThresher");

static DetectorMakerTemplate<TargetDetector>
registerTargetDetector("TargetDetector");

static DetectorMakerTemplate<BarbedWireDetector>
registerBarbedWireDetector("BarbedWireDetector");

static DetectorMakerTemplate<HedgeDetector>
registerHedgeDetector("HedgeDetector");

static DetectorMakerTemplate<WindowDetector>
registerWindowDetector("WindowDetector");

static DetectorMakerTemplate<VelocityDetector>
registerVelocityDetector("VelocityDetector");
    
} // namespace vision
} // namespace ram
    
// In this case the static variable is safe
#if RAM_COMPILER == RAM_COMPILER_MSVC
#  pragma warning( disable : 4640 )
#endif

namespace ram {
namespace pattern {

template<>
vision::DetectorMaker::MakerMap*
vision::DetectorMaker::getRegistry()
{
    // This line is run only once, avoids static initialization order issue
    static vision::DetectorMaker::MakerMap* reg =
        new vision::DetectorMaker::MakerMap();
    return reg;
}

} // namespace pattern
} // namespace ram
