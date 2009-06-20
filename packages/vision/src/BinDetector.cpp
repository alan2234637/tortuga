/*
 * Copyright (C) 2007 Robotics at Maryland
 * Copyright (C) 2007 Daniel Hakim
 * All rights reserved.
 *
 * Author: Daniel Hakim <dhakim@umd.edu>
 * File:  packages/vision/src/BinDetector.cpp
 */

// STD Includes
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
//#include <cassert>

// Library Includes
#include "cv.h"
#include "highgui.h"
#include <log4cpp/Category.hh>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

// Project Includes
#include "vision/include/main.h"
#include "vision/include/OpenCVImage.h"
#include "vision/include/BinDetector.h"
#include "vision/include/Camera.h"
#include "vision/include/Events.h"

#include "math/include/Vector2.h"

#include "core/include/Logging.h"
#include "core/include/PropertySet.h"

//static log4cpp::Category& LOGGER(log4cpp::Category::getInstance("Vision"));



namespace ram {
namespace vision {

static bool binToCenterComparer(BinDetector::Bin b1, BinDetector::Bin b2)
{
    return b1.distanceTo(0,0) < b2.distanceTo(0,0);
}   
    
BinDetector::Bin::Bin() :
    TrackedBlob(),
    m_symbol(Symbol::NONEFOUND)
{
}
    
BinDetector::Bin::Bin(BlobDetector::Blob blob, double x, double y,
                      math::Degree angle, int id, Symbol::SymbolType symbol) :
    TrackedBlob(blob, x, y, angle, id),
    m_symbol(symbol)
{
}
    
void BinDetector::Bin::draw(Image* image)
{
    IplImage* out = image->asIplImage();
    // Draw green rectangle around the blob
    CvPoint tl,tr,bl,br;
    tl.x = bl.x = getMinX();
    tr.x = br.x = getMaxX();
    tl.y = tr.y = getMinY();
    bl.y = br.y = getMaxY();
    cvLine(out, tl, tr, CV_RGB(0,255,0), 3, CV_AA, 0);
    cvLine(out, tl, bl, CV_RGB(0,255,0), 3, CV_AA, 0);
    cvLine(out, tr, br, CV_RGB(0,255,0), 3, CV_AA, 0);
    cvLine(out, bl, br, CV_RGB(0,255,0), 3, CV_AA, 0);

    // Now draw my id
    std::stringstream ss;
    ss << getId();
    Image::writeText(image, ss.str(), tl.x, tl.y);

    std::stringstream ss2;
    ss2 << getAngle().valueDegrees();
    Image::writeText(image, ss2.str(), br.x-30, br.y-15);

    // Now do the symbol
    if (Symbol::NONEFOUND == m_symbol)
        Image::writeText(image, "None", bl.x, bl.y - 15);
    else if (Symbol::UNKNOWN == m_symbol)
        Image::writeText(image, "Unknown", bl.x, bl.y - 15);
    else if (Symbol::HEART == m_symbol)
        Image::writeText(image, "Heart", bl.x, bl.y - 15);
    else if (Symbol::DIAMOND == m_symbol)
        Image::writeText(image, "Diamond", bl.x, bl.y - 15);
    else if (Symbol::SPADE == m_symbol)
        Image::writeText(image, "Spade", bl.x, bl.y - 15);
    else if (Symbol::CLUB == m_symbol)
        Image::writeText(image, "Club", bl.x, bl.y - 15);
}
    
    
BinDetector::BinDetector(core::ConfigNode config,
                         core::EventHubPtr eventHub) :
    Detector(eventHub),
    symbolDetector(config,eventHub),
    blobDetector(config,eventHub),
    m_centered(false)
{
    init(config);
    setSymbolDetectionOn(true);
}
    
void BinDetector::init(core::ConfigNode config)
{
    rotated = cvCreateImage(cvSize(640,480), 8, 3);
    // Its only 480 by 640 if the cameras on sideways
    binFrame =cvCreateImage(cvGetSize(rotated),8,3);
    bufferFrame = cvCreateImage(cvGetSize(rotated),8,3);
    memset(bufferFrame->imageData, 0,
           bufferFrame->width * bufferFrame->height * 3);
    whiteMaskedFrame = cvCreateImage(cvGetSize(rotated),8,3);
    blackMaskedFrame = cvCreateImage(cvGetSize(rotated),8,3);
    m_found=0;
/*    foundHeart = false;
    foundSpade = false;
    foundDiamond = false;
    foundClub = false;
    foundEmpty = false;*/
    m_runSymbolDetector = false;
    m_centeredLimit = config["centeredLimit"].asDouble(0.1);
    m_sameBinThreshold = config["sameBinThreshold"].asDouble(0.2);
    m_maxAspectRatio = config["maxAspectRatio"].asDouble(3);
    

    // NOTE: The property set automatically loads the value from the given
    //       config if its present, if not it uses the default value presented.
    core::PropertySetPtr propSet(getPropertySet());

    propSet->addProperty(config, false, "blackMaskMinimumPercent",
        "% of for the black mask minimum",
        10, &m_blackMaskMinimumPercent, 0, 100);
    propSet->addProperty(config, false, "blackMaskMaxTotalIntensity",
        "Maximum value of RGB pixels added together for black",
        350, &m_blackMaskMaxTotalIntensity, 0, 765);

    propSet->addProperty(config, false, "whiteMaskMinimumPercent",
        "% of for the white mask minimum",
        30, &m_whiteMaskMinimumPercent, 0, 100);
    propSet->addProperty(config, false, "whiteMaskMinimumIntensity",
        "Minimum value of RGB pixels added together for white",
        190, &m_whiteMaskMinimumIntensity, 0, 765);

    m_incrediblyWashedOutImages = (bool)(config["incrediblyWashedOut"].asInt(0));
    m_logSymbolImages = (bool)(config["logSymbolImages"].asInt(0));
    
    m_binID = 0;
    for (int i = 0; i < 4; i++)
    {
        binImages[i] = cvCreateImage(cvSize(128,128),8,3);
    }
}
    
BinDetector::~BinDetector()
{
    cvReleaseImage(&binFrame);
    cvReleaseImage(&rotated);
    cvReleaseImage(&bufferFrame);
    for (int i = 0; i < 4; i++)
    {
        cvReleaseImage((&binImages[i]));
    }
}

void BinDetector::processImage(Image* input, Image* out)
{
    /*First argument to white_detect is a ratios frame, then a regular one*/
    IplImage* image =(IplImage*)(*input);
    IplImage* output = NULL;
    if (out != NULL)
    {
        output = out->asIplImage();
    }
//std::cout<<"startup"<<std::endl;
    
    //This is only right if the camera is on sideways... again.
    //rotate90Deg(image,rotated);

    //Else just copy
    cvCopyImage(image,rotated);
    image=rotated;//rotated is poorly named when camera is on correctly... oh well.
        
    // Set image to a newly copied space so we don't write over opencv's 
    // private memory space... since opencv has a bad habit of making
    // assumptions about what I want to do. :)
    cvCopyImage(image,binFrame);
    
    // Fill in output image.
    if (out)
    {
        OpenCVImage temp(binFrame, false);
        out->copyFrom(&temp);
    }
    
    // Make image into each pixel being the percentage of its total value
    to_ratios(image);
    
    // Image is now in percents, binFrame is now the base image.
    /*int totalWhiteCount = */white_mask(image,binFrame, whiteMaskedFrame, m_whiteMaskMinimumPercent, m_whiteMaskMinimumIntensity);

if (!m_incrediblyWashedOutImages)
{
    black_mask(image,
               binFrame,
               blackMaskedFrame,
               m_blackMaskMinimumPercent,
               m_blackMaskMaxTotalIntensity);
}
else
{    
    //unsigned char* imgData = (unsigned char*)(image->imageData);
    unsigned char* blackMaskData = (unsigned char*)(blackMaskedFrame->imageData);
    unsigned char* binFrameData = (unsigned char*)(binFrame->imageData);
    for (int count = 0; count < binFrame->width * binFrame->height * 3; count+=3)
    {
	//unsigned char b2 = imgData[count];
	//unsigned char g2 = imgData[count+1];
	//unsigned char r2 = imgData[count+2];

        unsigned char b = binFrameData[count];
	//unsigned char g = binFrameData[count+1];
        unsigned char r = binFrameData[count+2];

        if (r < b/2 )//||(b2 >= m_blackMaskMinimumPercent && g2 >= m_blackMaskMinimumPercent && r2 >= m_blackMaskMinimumPercent && r + b + g < m_blackMaskMaxTotalIntensity))
        {
            blackMaskData[count]=255;
            blackMaskData[count+1]=255;
            blackMaskData[count+2]=255;
        }
        else
        {
            blackMaskData[count]=0;
            blackMaskData[count+1]=0;
            blackMaskData[count+2]=255;
        }
    }
}

    if (output)
    {
        // Make all white, solid white in the output
        int size = binFrame->width* binFrame->height * 3;
        for (int count = 0; count < size; count++)
        {
            if (whiteMaskedFrame->imageData[count] != 0)
            {
//            binFrame->imageData[count] = 255;
                output->imageData[count] = 255;
            }
        }

        for (int count = 0; count < size; count+=3)
        {
            if (blackMaskedFrame->imageData[count] != 0)
            {
//               binFrame->imageData[count] = 0;
                output->imageData[count] = 0;
                output->imageData[count+1] = 0;
                output->imageData[count+2] = 0;
            }
            else if (blackMaskedFrame->imageData[count+2] != 0 && whiteMaskedFrame->imageData[count] == 0)//if r > b/2 and its not white...
            {
                binFrame->imageData[count] = 0;
                binFrame->imageData[count+1] = 0;
                binFrame->imageData[count+2] = 255;
                output->imageData[count] = 0;
                output->imageData[count+1] = 0;
                output->imageData[count+2] = 255;
            }
        }
    }
//    if (output)
//    {
//    suitMask(image,output);
//    }

    // Find all the white blobs
    blobDetector.setMinimumBlobSize(2500);
    OpenCVImage whiteMaskWrapper(whiteMaskedFrame,false);
    blobDetector.processImage(&whiteMaskWrapper);
    BlobDetector::BlobList whiteBlobs = blobDetector.getBlobs();
    
    // Find all the black blobs
    blobDetector.setMinimumBlobSize(1500);
    OpenCVImage blackMaskWrapper(blackMaskedFrame,false);
    blobDetector.processImage(&blackMaskWrapper);
    BlobDetector::BlobList blackBlobs = blobDetector.getBlobs();

    // List of found blobs which are bins
    BlobDetector::BlobList binBlobs;

//    BOOST_FOREACH(BlobDetector::Blob whiteBlob, whiteBlobs)
//    {
//        if (output)
//            whiteBlob.draw(out);
//    }
//    
//    BOOST_FOREACH(BlobDetector::Blob blackBlob, blackBlobs)
//    {
//        if (output)
//            blackBlob.draw(out);
//    }

    // NOTE: all blobs sorted largest to smallest
    BOOST_FOREACH(BlobDetector::Blob blackBlob, blackBlobs)
    {
        BOOST_FOREACH(BlobDetector::Blob whiteBlob, whiteBlobs)
        {
            // Sadly, this totally won't work at the edges of the screen...
            // crap damn.
            if (whiteBlob.containsInclusive(blackBlob,2) &&
                blackBlob.getAspectRatio() < m_maxAspectRatio)
            {
                // blackBlobs[blackBlobIndex] is the black rectangle of a bin
                binBlobs.push_back(blackBlob);
                
                // Don't add it once for each white blob containing it,
                // thatd be dumb.
                break;
            }
            else
            {
                //Not a bin.
            }
        }
    }
    
    if (out)
    {
        std::stringstream ss;
        ss << binBlobs.size();
        Image::writeText(out, ss.str(),0,200);
    }
    
//    std::cout<<"Made set of bins"<<std::endl;
//    std::cout<<"Num bins found: " << binBlobs.size() <<std::endl;
    
/*    bool seeHeart=false;
    bool seeDiamond=false;
    bool seeClub=false;
    bool seeSpade=false;
    bool seeEmpty=false;*/

    // Publish lost event
    if (m_found && (binBlobs.size() == 0))
    {
        m_found = false;
        m_centered = false;
        publish(EventType::BIN_LOST, core::EventPtr(new core::Event()));
    }
    
    if (binBlobs.size() > 0)
    {
        // For each black blob inside a white blob (ie, for each bin blob) 
        // take the rectangular portion containing the black blob and pass it
        // off to SymbolDetector
        BinList candidateBins;
        
        int binNumber = 0;
        BOOST_FOREACH(BlobDetector::Blob binBlob, binBlobs)
        {
            processBin(binBlob, m_runSymbolDetector, candidateBins, binNumber,
                       out);
            binNumber++;
        }

        // Determine the angle of the bin array
        if (candidateBins.size() > 1 && candidateBins.size() <= 4)
        {
            int curX = -1;
            int curY = -1;
            int prevX = -1;
            int prevY = -1;
            int binsCenterX = 0;
            int binsCenterY = 0;
            double innerAngles[3];//If you change this from a 3, also change the loops below
            int angleCounter = 0;
            BOOST_FOREACH(Bin bin, candidateBins)
            {
                binsCenterX += bin.getCenterX();
                binsCenterY += bin.getCenterY();
                prevX = curX;
                prevY = curY;
                curX = bin.getCenterX();
                curY = bin.getCenterY();
                
                if (prevX == -1 && prevY == -1)
                {
                    // the first one
                }
                else
                {
                    CvPoint prev;
                    CvPoint cur;
                    prev.x = prevX;
                    prev.y = prevY;
                    cur.x = curX;
                    cur.y = curY;
                    
                    //Swap so we always get answers mod 180.
                    if (prev.x > cur.x || (prev.x == cur.x && prev.y > cur.y))
                    {
                        CvPoint swap = prev;
                        prev = cur;
                        cur = swap;
                    }
                    
                    double innerAng = atan2(cur.y - prev.y,cur.x - prev.x);
                    
                    if (out)
                    {
                        cvLine(output, prev, cur, CV_RGB(255,0,0), 5, CV_AA, 0 );                    
                    }
                    innerAngles[angleCounter] = innerAng;
                    angleCounter++;
                }
            }
            
            double sinTotal = 0;
            double cosTotal = 0;
            for (int i = 0; i < angleCounter && i < 3; i++)
            {
                sinTotal+=sin(innerAngles[i]);
                cosTotal+=cos(innerAngles[i]);
            }
            
            double finalAngleAcrossBins = atan2(sinTotal,cosTotal);
            
            CvPoint drawStart, drawEnd;
            drawStart.x = binsCenterX/(angleCounter+1);
            drawStart.y = binsCenterY/(angleCounter+1);
            
            drawEnd.x = (int)(drawStart.x + cosTotal / (angleCounter) * 250);
            drawEnd.y = (int)(drawStart.y + sinTotal / (angleCounter) * 250);
            if (out)
            {
                cvLine(output, drawStart, drawEnd, CV_RGB(255,255,0),5, CV_AA,0); 
            }
    //        printf("final angle across bins %f:\n", finalAngleAcrossBins);
            
            math::Radian angleAcrossBins(finalAngleAcrossBins);
            
            math::Degree finalInnerAngleForJoe(-angleAcrossBins.valueDegrees());
//            printf("Final Inner Angle For Joe: %f\n", finalInnerAngleForJoe.valueDegrees());
            BinEventPtr event(new BinEvent(finalInnerAngleForJoe));
            publish(EventType::MULTI_BIN_ANGLE, event);
        } // angle of bin array
        
        // Sort candidate bins on distance from center
        candidateBins.sort(binToCenterComparer);

        // Sort through our candidate bins and match them to the old ones
        TrackedBlob::updateIds(&m_bins, &candidateBins, m_sameBinThreshold);

        // List of new bins
        BinList newBins = candidateBins;

        // Anybody left we didn't find this iteration, so its been dropped
        BOOST_FOREACH(Bin bin, m_bins)
        {
            BinEventPtr event(new BinEvent(bin.getX(), bin.getY(), 
                                           bin.getSymbol(), bin.getAngle()));
            event->id = bin.getId();
            publish(EventType::BIN_DROPPED, event);
        }

        // Track whether we lose the bin which is centered
        //if (m_centered)
        //{
        //    int centeredID;
        //}
        
        // Sort list by distance from center and copy it over the old one
        newBins.sort(binToCenterComparer);
        m_bins = newBins;
                
        // Now publish found & centered events
        math::Vector2 toCenter(getX(), getY());
        if (toCenter.normalise() < m_centeredLimit)
        {
            if(!m_centered)
            {
                m_centered = true;
                BinEventPtr event(new BinEvent(getX(), getY(), getSymbol(), getAngle()));
                publish(EventType::BIN_CENTERED, event);
            }
        }
        else
        {
            m_centered = false;
        }
        
        BOOST_FOREACH(Bin bin, m_bins)
        {
            if (out)
                bin.draw(out);
            BinEventPtr event(new BinEvent(bin.getX(), bin.getY(), 
                                           bin.getSymbol(), bin.getAngle()));
            event->id = bin.getId();
            publish(EventType::BIN_FOUND, event);
        }
    }
}

void BinDetector::drawBinImage(Image* imgToShow, int binNumber,
			       Image* output)
{
    Image::drawImage(imgToShow, 
		     binNumber * 128, 
		     0,
		     output, 
		     output);
}

void BinDetector::processBin(BlobDetector::Blob bin, bool detectSymbol,
                             BinList& newBins, int binNum, Image* output)
{
    if (binNum > 3)
        return;

    // Get the bin center in AI coordinates
    OpenCVImage temp(binFrame, false);
    double binX = 0;
    double binY = 0;
    Detector::imageToAICoordinates(&temp, bin.getCenterX(), bin.getCenterY(),
				   binX, binY);
//        std::cout<<"Finished writing to output image"<<std::endl;


    // Create the image to hold the bin blob, give a little bit of extra space
    // around the black.  (10 pixels)
    int width = (bin.getMaxX()-bin.getMinX()+31)/4*4;
    int height = (bin.getMaxY()-bin.getMinY()+31)/4*4;


    //Regular sizing
    // Create the image to hold the bin blob
//    int width = (bin.getMaxX()-bin.getMinX()+1)/4*4;
//    int height = (bin.getMaxY()-bin.getMinY()+1)/4*4;
    OpenCVImage binImage(width, height);
    // Extra bin blob into image
    CvPoint2D32f binCenter = cvPoint2D32f(bin.getCenterX(), bin.getCenterY());
    
    if ((binCenter.x - width/2 - 1 < 0) || 
        (binCenter.x + width/2 + 1 >= binFrame->width) ||
        (binCenter.y - height/2 -1 < 0) ||
        (binCenter.y + height/2 +1 >= binFrame->height))
    {
        newBins.push_back(Bin(bin, binX, binY, math::Degree(0), m_binID++, Symbol::UNKNOWN));
        return;
    }
    
    cvGetRectSubPix(binFrame, binImage.asIplImage(), binCenter);
    math::Radian angle = calculateAngleOfBin(bin, &binImage, &binImage);
    //Image::showImage(&binImage);
    // Find symbol if desired
    Symbol::SymbolType symbol = Symbol::NONEFOUND;
    
    // Rotate image to straight
    OpenCVImage rotatedRedSymbolWrapper(binImage.getWidth(),
                                          binImage.getHeight());
    vision::Image::transform(&binImage, &rotatedRedSymbolWrapper, -angle);
    
    // Set fields as percent of colors
    OpenCVImage percentsRotatedRedWrapper(
        rotatedRedSymbolWrapper.getWidth(),
        rotatedRedSymbolWrapper.getHeight());
        
    cvCopyImage(rotatedRedSymbolWrapper.asIplImage(),
                percentsRotatedRedWrapper.asIplImage());
    to_ratios(percentsRotatedRedWrapper.asIplImage());
        
    if (detectSymbol)
    {

//        drawBinImage(&maskedRotatedRed, binNum);

        // Now mask just red (ie. make it white)
        suitMask(percentsRotatedRedWrapper.asIplImage(), rotatedRedSymbolWrapper.asIplImage());
        
        if (cropImage(rotatedRedSymbolWrapper.asIplImage(), binNum))
        {
            OpenCVImage wrapper(binImages[binNum],false);
//            Image::showImage(&wrapper);
            symbol = determineSymbol(&wrapper);
	    if (output)
            {
               drawBinImage(&wrapper, binNum, output);

               if (m_logSymbolImages)
                   logSymbolImage(&wrapper, symbol);
            }
        }
        else
        {
            symbol = Symbol::UNKNOWN;
//            printf("No symbol found\n");
        }
    }
    
    double angInDegrees = angle.valueDegrees();
    double angleToReturn = 90-angInDegrees;
    
    if (angleToReturn >= 90)
        angleToReturn -= 180;
    else if (angleToReturn < -90)
        angleToReturn += 180;
    
    math::Degree finalJoeAngle(angleToReturn);
    // Create bin add it to the list (and incremet the binID)
    newBins.push_back(Bin(bin, binX, binY, finalJoeAngle, m_binID++, symbol));
    
    m_found = true;
}

void BinDetector::unrotateBin(math::Radian angleOfBin, Image* redSymbol, 
			      Image* rotatedRedSymbol)
{
    float m[6];
    CvMat M = cvMat( 2, 3, CV_32F, m );

    double factor = -angleOfBin.valueRadians();
    m[0] = (float)(cos(factor));
    m[1] = (float)(sin(factor));
    m[2] = redSymbol->getWidth() * 0.5f;
    m[3] = -m[1];
    m[4] = m[0];
    m[5] = redSymbol->getWidth() * 0.5f;
    
    cvGetQuadrangleSubPix(redSymbol->asIplImage(), rotatedRedSymbol->asIplImage(),&M);
}

math::Radian BinDetector::calculateAngleOfBin(BlobDetector::Blob bin, 
					      Image* input, Image* output)
{
    IplImage* redSymbol = (IplImage*)(*input);
    IplImage* redSymbolGrayScale = cvCreateImage(cvGetSize(redSymbol),IPL_DEPTH_8U,1);

    cvCvtColor(redSymbol,redSymbolGrayScale,CV_BGR2GRAY);
    IplImage* cannied = cvCreateImage(cvGetSize(redSymbolGrayScale), 8, 1 );

    cvCanny( redSymbolGrayScale, cannied, 50, 200, 3 );
    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* lines = 0;
    
//    OpenCVImage wrapper(cannied, false);
//    Image::showImage(&wrapper);
    lines = cvHoughLines2( cannied, storage, CV_HOUGH_PROBABILISTIC, 3, CV_PI/180, 150, 5, 50);
        
    float longestLineLength = -1;
    float angle = 0;
    for(int i = 0; i < lines->total; i++ )
    {
        CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
        float lineX = line[1].x - line[0].x; 
        float lineY = line[1].y - line[0].y;
        
        if (output)
        {
            cvLine(output->asIplImage(), line[0], line[1], CV_RGB(255,255,0), 5, CV_AA, 0 );
        }
//        printf("Line dimensions: %f, %f\n", lineX, lineY);

        if (longestLineLength < (lineX * lineX + lineY * lineY))
        {
            angle = atan2(lineY,lineX);
            longestLineLength = lineX * lineX + lineY * lineY;
        }
    }
    
    if (lines->total == 0)
        return math::Degree(-180);
    cvReleaseImage(&cannied);
    cvReleaseMemStorage(&storage);
    cvReleaseImage(&redSymbolGrayScale);
    return math::Radian(angle);
}

Symbol::SymbolType BinDetector::determineSymbol(Image* input,
                                          Image* output)
{
//        std::cout<<"finished symbol detection"<<std::endl;
//        std::cout<<"Symbol: " << symbolDetector.getSymbol()<<std::endl;
    symbolDetector.processImage(input,output);
	
    // Filter symbol type
    Symbol::SymbolType symbolFound = symbolDetector.getSymbol(); //In case we ever want to use the symbol detector...
    Symbol::SymbolType symbol = Symbol::NONEFOUND;

        
    if (symbolFound == Symbol::CLUB || symbolFound == Symbol::CLUBR90 || symbolFound == Symbol::CLUBR180 || symbolFound == Symbol::CLUBR270)
    {
        //seeClub = true;
        symbol = Symbol::CLUB;
//        std::cout<<"Found Club Bin"<<std::endl;
        
    }
    else if (symbolFound == Symbol::SPADE || symbolFound == Symbol::SPADER90 || symbolFound == Symbol::SPADER180 || symbolFound == Symbol::SPADER270)
    {
        //seeSpade = true;
        symbol = Symbol::SPADE;
//        std::cout<<"Found Spade Bin"<<std::endl;
        
    }
    else if (symbolFound == Symbol::HEART || symbolFound == Symbol::HEARTR90 || symbolFound == Symbol::HEARTR180 || symbolFound == Symbol::HEARTR270)
    {
        //seeHeart = true;
        symbol = Symbol::HEART;
//        std::cout<<"Found Heart Bin"<<std::endl;
    }
    else if (symbolFound == Symbol::DIAMOND || symbolFound == Symbol::DIAMONDR90 || symbolFound == Symbol::DIAMONDR180 || symbolFound == Symbol::DIAMONDR270)
    {
        //seeDiamond = true;
        symbol = Symbol::DIAMOND;
//        std::cout<<"Found Diamond Bin"<<std::endl;
    }
    else if (symbolFound == Symbol::UNKNOWN)
    {
        symbol = Symbol::UNKNOWN;
//        std::cout<<"Found an unknown bin, rotate above it until we figure out what it is!"<<std::endl;
    }
    else if (symbolFound == Symbol::NONEFOUND)
    {
        //seeEmpty = true;
        symbol = Symbol::NONEFOUND;
//        std::cout<<"Found empty Bin"<<std::endl;
    }
    return symbol;
}
    
float BinDetector::getX()
{
    if (m_bins.size() > 0)
        return m_bins.front().getX();
    else
        return 0;
}
float BinDetector::getY()
{
    if (m_bins.size() > 0)
        return m_bins.front().getY();
    else
        return 0;
}
math::Degree BinDetector::getAngle()
{
    if (m_bins.size() > 0)
        return m_bins.front().getAngle();
    else
        return math::Degree(0);
}

Symbol::SymbolType BinDetector::getSymbol()
{
    if (m_bins.size() > 0)
        return m_bins.front().getSymbol();
    else
        return Symbol::NONEFOUND;
}

BinDetector::BinList BinDetector::getBins()
{
    return m_bins;
}

bool BinDetector::found()
{
    return m_found;
}

//Returns false on failure, puts symbol into scaledRedSymbol.
bool BinDetector::cropImage(IplImage* rotatedRedSymbol, int binNum)
{   
    int minSymbolX = 999999;
    int minSymbolY = 999999;
    int maxSymbolX = 0;
    int maxSymbolY = 0;
    //            int redCX, redCY;
//    cvDilate(rotatedRedSymbol,rotatedRedSymbol,NULL, 1);
    OpenCVImage mySymbol(rotatedRedSymbol,false);
    //   int size = 0;
    blobDetector.setMinimumBlobSize(100);
    blobDetector.processImage(&mySymbol);
    if (!blobDetector.found())
    {
        return false;
        //no symbol found, don't make a histogram
        //                printf("Oops, we fucked up, no symbol found :(\n");
    }
    else
    {
        //find biggest two blobs (hopefully should be just one, but if spade or club split..)
        std::vector<ram::vision::BlobDetector::Blob> blobs = blobDetector.getBlobs();
        ram::vision::BlobDetector::Blob biggest(-1,0,0,0,0,0,0);
        ram::vision::BlobDetector::Blob secondBiggest(0,0,0,0,0,0,0);
        ram::vision::BlobDetector::Blob swapper(-1,0,0,0,0,0,0);
        for (unsigned int blobIndex = 0; blobIndex < blobs.size(); blobIndex++)
        {
            if (blobs[blobIndex].getSize() > secondBiggest.getSize())
            {
                secondBiggest = blobs[blobIndex];
                if (secondBiggest.getSize() > biggest.getSize())
                {
                    swapper = secondBiggest;
                    secondBiggest = biggest;
                    biggest = swapper;
                }
            }
        }
        minSymbolX = biggest.getMinX();
        minSymbolY = biggest.getMinY();
        maxSymbolX = biggest.getMaxX();
        maxSymbolY = biggest.getMaxY();

        if (!m_incrediblyWashedOutImages)//A fancy way to say that at transdec, the symbols don't get split.
        {
        if (blobs.size() > 1)
        {
            if (minSymbolX > secondBiggest.getMinX())
                minSymbolX = secondBiggest.getMinX();
            if (minSymbolY > secondBiggest.getMinY())
                minSymbolY = secondBiggest.getMinY();
            if (maxSymbolX < secondBiggest.getMaxX())
                maxSymbolX = secondBiggest.getMaxX();
            if (maxSymbolY < secondBiggest.getMaxY())
                maxSymbolY = secondBiggest.getMaxY();

        }
        }
    }

    int onlyRedSymbolRows = (maxSymbolX - minSymbolX + 1);// / 4 * 4;
    int onlyRedSymbolCols = (maxSymbolY - minSymbolY + 1);// / 4 * 4;

    if (onlyRedSymbolRows == 0 || onlyRedSymbolCols == 0)
    {
        return false;
//        binImages[binNum] = NULL;
    }
        
    onlyRedSymbolRows = onlyRedSymbolCols = (onlyRedSymbolRows > onlyRedSymbolCols ? onlyRedSymbolRows : onlyRedSymbolCols);


    if (onlyRedSymbolRows >= rotatedRedSymbol->width || onlyRedSymbolCols >= rotatedRedSymbol->height)
    {
        return false;
//        binImages[binNum] = NULL;
    }
        
    IplImage* onlyRedSymbol = cvCreateImage(
        cvSize(onlyRedSymbolRows,
               onlyRedSymbolCols),
        IPL_DEPTH_8U,
        3);
    
    cvGetRectSubPix(rotatedRedSymbol,
                    onlyRedSymbol,
                    cvPoint2D32f((maxSymbolX+minSymbolX)/2,
                                 (maxSymbolY+minSymbolY)/2));
    
    cvResize(onlyRedSymbol, binImages[binNum], CV_INTER_LINEAR);

//    for (int redIndex = 2; redIndex < 128 * 128 * 3; redIndex+=3)
//    {
//        if (binImages[binNum]->imageData[redIndex] != 0)
//            size++;
//    }

//    printf("Bin Num: %d, Size: %d", binNum, size);
    
    cvReleaseImage(&onlyRedSymbol);
    return true;
}

void BinDetector::setSymbolDetectionOn(bool on)
{
    m_runSymbolDetector = on;
}

void BinDetector::setSymbolImageLogging(bool value)
{
    m_logSymbolImages = value;
}

void BinDetector::logSymbolImage(Image* image, Symbol::SymbolType symbol)
{
    static int saveCount = 1;
    
    if (saveCount == 1)
    {
        // First run, make sure all the directories are created
        std::string names[] = {"heart", "spade", "diamond", "club", "unknown"};
        boost::filesystem::path base = core::Logging::getLogDir();

        for (int i = 0; i < 5; ++i)
        {
            boost::filesystem::path symbolDir = base / names[i];
            if (!boost::filesystem::exists(symbolDir))
                boost::filesystem::create_directories(symbolDir);
        }
    }

    // Determine the directory to place the image based on symbol
    boost::filesystem::path base = core::Logging::getLogDir();
    boost::filesystem::path baseDir;
    switch (symbol)
    {
        case Symbol::HEART: baseDir = base / "heart"; break;
        case Symbol::SPADE: baseDir = base / "spade"; break;
        case Symbol::DIAMOND: baseDir = base / "diamond"; break;
        case Symbol::CLUB: baseDir = base / "club"; break;
        case Symbol::UNKNOWN: baseDir = base / "unknown"; break;
            
        default: baseDir = base / "error"; break;
    }

    // Determine the final path
    std::stringstream ss;
    ss << saveCount << ".png";
    boost::filesystem::path fullPath =  baseDir / ss.str();
    
    // Save the image and increment our counter
    Image::saveToFile(image, fullPath.string());
    saveCount++;
}

} // namespace vision
} // namespace ram
