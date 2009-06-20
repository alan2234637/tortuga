/*
 * Copyright (C) 2009 Robotics at Maryland
 * Copyright (C) 2009 David Love
 * All rights reserved.
 *
 * Author: David Love <loved@umd.edu>
 * File:  packages/vision/include/FANNSuitDetector.hpp
 */

#ifndef RAM_VISION_FANN_SUITE_DETECTOR_HPP
#define RAM_VISION_FANN_SUITE_DETECTOR_HPP

// Project Includes
#include "vision/include/Symbol.h"
#include "vision/include/ImageDetector.hpp"

// Must be included last
#include "vision/include/Export.h"

namespace ram {
namespace vision {

/** Wraps ImageDetector to create a card-suite specific interface
    identical to that of SuitDetector */
class RAM_EXPORT FANNSuitDetector : public Detector
{
  public:
    FANNSuitDetector(core::ConfigNode config,
                     core::EventHubPtr eventHub = core::EventHubPtr());
    
    void processImage(Image* input, Image* output= 0);
    
    /** Stub don't do nuttin' **/
    void update() {};
    
    /** Get the analyzed image - just returns the input **/
    IplImage* getAnalyzedImage();
    
    /** Get the suite type discovered **/
    Symbol::SymbolType getSuit();
  private:
    ImageDetector m_imageDetector;
    IplImage* m_analyzed;

};
	
} // namespace vision
} // namespace ram

#endif // RAM_SUIT_DETECTOR_H_04_29_2008
