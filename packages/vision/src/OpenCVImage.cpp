/*
 * Copyright (C) 2007 Robotics at Maryland
 * Copyright (C) 2007 Daniel Hakim
 * All rights reserved.
 *
 * Author: Daniel Hakim
 * File:  packages/vision/include/CvCamera.h
 */

// STD Includes
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cmath>

// Library Includes
/// TODO: Limit this to just the needed headers if possible
#include "cv.h" // OpenCV Functions
#include "highgui.h"

// Project incldues
#include "vision/include/Convert.h"
#include "vision/include/Exception.h"
#include "vision/include/OpenCVImage.h"

#define RGB2LCHUV -3

/** Error handler to send an abort signal if OpenCV throws an error */
int cvErrorHandler(int status, char const* func_name,
                   char const* err_msg, char const* file_name,
                   int line, void*)
{
    std::cerr << "ERROR: in " << func_name << "(" << file_name
              << ":" << line << ") Message: " << err_msg << std::endl;
    abort();
}

namespace ram {
namespace vision {

OpenCVImage::OpenCVImage(int width, int height, Image::PixelFormat fmt) :
    m_own(true),
    m_data(0),
    m_img(0),
    m_fmt(fmt)
{
    assert(width >= 1 && "Image can't have a negative or 0 width");
    assert(height >= 1 && "Image can't have a negative or 0 height");

#ifdef RAM_POSIX
    // Assign the error handler if it hasn't been done
    static bool errHandlerInitialized = false;
    if (!errHandlerInitialized) {
        cvRedirectError(::cvErrorHandler);
        errHandlerInitialized = true;
    }
#endif
    
    int depth, channels;
    getFormatParameters(fmt, depth, channels);
    m_img = cvCreateImage(cvSize(width, height), depth, channels);

    assert(m_img && "Error creating OpenCV Image");
}
    
OpenCVImage::OpenCVImage(IplImage* image, bool ownership,
                         Image::PixelFormat fmt) :
    m_own(ownership),
    m_data(0),
    m_img(image),
    m_fmt(fmt)
{
    // If a pixel format is given, sanity check it
    if (fmt != PF_START) {
        int depth, channels;
        getFormatParameters(fmt, depth, channels);
        assert(image->nChannels == channels &&
               "The given image has a different number of channels"
               " than the specified format");
        assert(image->depth == depth &&
               "The given image has a different depth than"
               " the specified format");
    }
}

/**
 * Creates an OpenCVImage using the data.
 *
 * Note: OpenCVImage should only take ownership of data that is allocated
 *       using new. Unknown behavior will happen when using malloc or
 *       passing data allocated on the stack.
 */
OpenCVImage::OpenCVImage(unsigned char* data, int width, int height,
                         bool ownership, Image::PixelFormat fmt) :
    m_own(ownership),
    m_data(data),
    m_img(0),
    m_fmt(fmt)
{
    assert(data && "Image data can't be null");
    assert(width >= 1 && "Image can't have a negative or 0 width");
    assert(height >= 1 && "Image can't have a negative or 0 height");

    int depth, channels;
    getFormatParameters(fmt, depth, channels);
    m_img = cvCreateImageHeader(cvSize(width, height), depth, channels);
    cvSetData(m_img, data, width * channels);
}
    
OpenCVImage::OpenCVImage(std::string fileName, Image::PixelFormat fmt) :
    m_own(true),
    m_data(0),
    m_img(cvLoadImage(fileName.c_str())),
    m_fmt(fmt)
{
    assert(m_img && "Image could not be loaded from file");
}

void OpenCVImage::getFormatParameters(const Image::PixelFormat& fmt,
                                      int& depth, int &channels)
{
    static const std::pair<int, int> lookupTable[11] = {
        std::make_pair(8, 3), // PF_START
        std::make_pair(8, 3), // PF_RGB_8
        std::make_pair(8, 3), // PF_BGR_8
        std::make_pair(8, 3), // PF_YUV444_8
        std::make_pair(8, 1), // PF_GRAY_8
        std::make_pair(8, 3), // PF_HSV_8
        std::make_pair(8, 3), // PF_LUV_8
        std::make_pair(8, 3), // PF_LCHUV_8
        std::make_pair(8, 3), // PF_LAB_8
        std::make_pair(8, 3), // PF_LCHAB_8
        std::make_pair(8, 3)  // PF_END
    };

    depth = lookupTable[fmt].first;
    channels = lookupTable[fmt].second;
}
    
void OpenCVImage::copyFrom (const Image* src)
{
    // Handle self copy
    if (this == src)
        return;
    
    PixelFormat src_fmt = src->getPixelFormat();
    // Create temporaty OpenCV image to smooth the copy process
    int depth, channels;
    getFormatParameters(src_fmt, depth, channels);
    IplImage* tmp_img = cvCreateImageHeader(cvSize(src->getWidth(),
                                                   src->getHeight()),
                                            depth, channels);
    cvSetData(tmp_img, src->getData(), src->getWidth() * channels);

    // Resize this image to match the source needed (also copy)
    if ((getWidth() != src->getWidth()) ||
        (getHeight() != src->getHeight()) ||
        (getNumChannels() != src->getNumChannels()) ||
        (getDepth() != src->getDepth()))
    {
        assert(m_own && "Cannot perform resize unless I own the image");
        cvReleaseImage(&m_img);
        m_img = cvCreateImage(cvSize(src->getWidth(), src->getHeight()),
                              depth, channels);
    }
    // Copy the internal image data over
    cvCopy(tmp_img, m_img);

    cvReleaseImageHeader(&tmp_img);

    // Set the pixel format
    m_fmt = src_fmt;
    
    // Copy Other members
    //m_own = src->getOwnership();
    // copying this makes no sense, we now have a totally new copy, so we own it
    //m_own = true;
}
    
OpenCVImage::~OpenCVImage()
{
    if (m_own)
    {
        assert(m_img && "Error can't free empty OpenCV image");
        if (m_data)
        {
            cvReleaseImageHeader(&m_img);
            delete[] m_data;
        }
        else
        {
            cvReleaseImage(&m_img);
        }
    }
    else if (m_data)
    {
        // If data is not null, then we need to release the created image header
        cvReleaseImageHeader(&m_img);
    }
}

unsigned char* OpenCVImage::getData() const
{
    return (unsigned char*)(m_img->imageData);
}
    
size_t OpenCVImage::getWidth() const
{
    return cvGetSize(m_img).width;
}

size_t OpenCVImage::getHeight() const
{
    return cvGetSize(m_img).height;
}

size_t OpenCVImage::getDepth() const
{
    return m_img->depth;
}

size_t OpenCVImage::getNumChannels() const
{
    return m_img->nChannels;
}

Image::PixelFormat OpenCVImage::getPixelFormat() const
{
    ///TODO:Make sure this format is actually the format that the image is in.
    return m_fmt;
}

bool OpenCVImage::getOwnership() const
{
    return m_own;
}
    
unsigned char* OpenCVImage::setData(unsigned char* data, bool ownership)
{
    m_own = ownership;
    unsigned char* tmp = getData();
    int depth, channels;
    getFormatParameters(m_fmt, depth, channels);
    cvSetImageData(m_img, data, cvGetSize(m_img).width * channels);
    return tmp;
}

void  OpenCVImage::setSize(int width, int height)
{
    // Leave early if we really aren't changing the image size
    if ((cvGetSize(m_img).width == width) && (cvGetSize(m_img).height == height))
        return;

    assert(m_own && "Can't change size of an image we don't own");
    assert(width >= 1 && "Image can't have a negative or 0 width");
    assert(height >= 1 && "Image can't have a negative or 0 height");
    
    IplImage* old = m_img;
    
    int depth, channels;
    getFormatParameters(m_fmt, depth, channels);
    m_img = cvCreateImage(cvSize(width, height), depth, channels);
    //cvCopy(old, m_img);
    // this doesn't work, so let's use the right function instead
    cvResize (old, m_img);
    
    cvReleaseImage(&old);
}

void OpenCVImage::setPixelFormat(Image::PixelFormat format)
{
    assert(format != PF_START && "No format specified");
    assert(format != PF_END   && "No format specified");
    assert(m_own && "Must have ownership of the image to change its format");

    /**
     * Lookup table for conversion codes
     * Invalid conversions labeled with -1
     * Conversions to the same colorspace labeled with -2
     * Conversions on a sentinal value are ALWAYS invalid
     * Macros are defined in cv.h
     *
     * Table is constructed so the row is the current colorspace,
     * column is the colorspace to convert to.
     */
    static const int lookupTable[11][11] = {
        /* Sentinal value */
        {-1,         -1,         -1,-1,         -1,        -1,        -1,        -1, -1, -1, -1},
        /* RGB */
        {-1,         -2, CV_RGB2BGR,-1,CV_RGB2GRAY,CV_RGB2HSV,CV_RGB2Luv, RGB2LCHUV, -1, -1, -1},
        /* BGR */
        {-1, CV_BGR2RGB,         -2,-1,CV_BGR2GRAY,CV_BGR2HSV,CV_BGR2Luv,        -1, -1, -1, -1},
        /* YUV */
        {-1,         -1,         -1,-2,         -1,        -1,        -1,        -1, -1, -1, -1},
        /* Grayscale */
        {-1,CV_GRAY2RGB,CV_GRAY2BGR,-1,         -2,        -1,        -1,        -1, -1, -1, -1},
        /* HSV */ 
        {-1, CV_HSV2RGB, CV_HSV2BGR,-1,         -1,        -2,        -1,        -1, -1, -1, -1},
        /* CIELUV */
        {-1, CV_Luv2RGB, CV_Luv2BGR,-1,         -1,        -1,        -2,        -1, -1, -1, -1},
        /* CIELCh_uv */
        {-1,         -1,         -1,-1,         -1,        -1,        -1,        -2, -1, -1, -1},
        /* CIELAB */
        {-1,         -1,         -1,-1,         -1,        -1,        -1,        -1, -1, -1, -1},
        /* CIELCh_ab */
        {-1,         -1,         -1,-1,         -1,        -1,        -1,        -1, -1, -1, -1},
        /* Sentinal value */
        {-1,         -1,         -1,-1,         -1,        -1,        -1,        -1, -1, -1, -1}
    };

    int code = lookupTable[m_fmt][format];
    if(code == RGB2LCHUV) {
        Convert::RGB2LCHuv(this);
        m_fmt = Image::PF_LCHUV_8;
    } else if (code == -1) {
        throw ImageConversionException(m_fmt, format);
    } else if (code != -2) {
        // If the number of channels or depth change, we need a new image
        int depth, channels;
        getFormatParameters(m_fmt, depth, channels);
        int newDepth, newChannels;
        getFormatParameters(format, newDepth, newChannels);

        if (depth != newDepth || channels != newChannels) {
            // Create a new image with the new depth/channels
            IplImage* newImg = cvCreateImage(cvGetSize(m_img),
                                             newDepth, newChannels);
            cvCvtColor(m_img, newImg, code);

            // Delete old image data
            if (m_data) {
                cvReleaseImageHeader(&m_img);
                delete[] m_data;
                m_data = NULL;
            } else {
                cvReleaseImage(&m_img);
            }

            // Assign modified image as current image
            m_img = newImg;
        } else {
            cvCvtColor(m_img, m_img, code);
        }

        // Change the format flag
        m_fmt = format;
    }
}

OpenCVImage::operator IplImage* ()
{
    return m_img;
}

IplImage* OpenCVImage::asIplImage() const
{
    return m_img;
}

} // namespace vision
} // namespace ram
