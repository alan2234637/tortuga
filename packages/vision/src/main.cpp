/*
 * Copyright (C) 2007 Robotics at Maryland
 * Copyright (C) 2007 Daniel Hakim
 * All rights reserved.
 *
 * Author: Daniel Hakim <dhakim@umd.edu>
 * File:  packages/vision/src/main.cpp
 */

#define _CRT_SECURE_NO_WARNINGS

// STD Includes
#include <math.h>
#include <cstdlib>
#include <cassert>
#include <stdio.h>
#include <time.h>

#include <list>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Library Includes
#include "cv.h"
#include "highgui.h"

// Project Includes
#include "vision/include/main.h"
#include "vision/include/OpenCVImage.h"
#include "vision/include/BlobDetector.h"


/* 
	Daniel Hakim
	VISION CODE!!! 
	PLEASE TOUCH ME.
        
        I am all over it.
          -JHL 02-09-2008
*/
using namespace std;
using namespace ram::vision;

//int pixelCountsDiamond[] = {
//0, 2, 4, 6, 8, 10, 12, 12, 14, 16, 18, 20, 22, 24, 26, 28,
//30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 52, 54, 56, 58, 60, 62,
//62, 60, 58, 56, 54, 52, 50, 48, 44, 42, 40, 38, 36, 34, 32, 30,
//28, 26, 24, 22, 20, 18, 16, 14, 14, 12, 10, 8, 6, 4, 2, 2};
//
//int pixelCountsClub[] = {
//8, 13, 16, 20, 22, 23, 24, 26, 26, 28, 28, 28, 28, 28, 28, 28,
//28, 28, 28, 26, 26, 40, 49, 54, 56, 58, 59, 60, 62, 62, 62, 64,
//64, 64, 64, 64, 64, 64, 64, 60, 60, 58, 54, 53, 49, 45, 42, 38,
//30, 18, 6, 8, 8, 10, 12, 14, 16, 18, 20, 24, 30, 36, 44, 44};
//
//int pixelCountsSpade[] = {
//0, 2, 2, 2, 4, 4, 6, 8, 8, 10, 12, 14, 16, 18, 20, 24,
//26, 28, 32, 34, 36, 40, 42, 46, 48, 50, 52, 54, 56, 58, 60, 62,
//62, 62, 64, 64, 64, 64, 64, 62, 60, 58, 54, 50, 46, 40, 33, 18,
//6, 6, 8, 8, 10, 12, 14, 16, 18, 22, 26, 30, 36, 46, 56, 56};
//
//int pixelCountsHeart[] = {
//17, 28, 36, 42, 46, 50, 54, 56, 58, 60, 62, 63, 64, 64, 64, 64,
//64, 62, 62, 62, 60, 60, 58, 58, 56, 55, 54, 52, 50, 49, 47, 45,
//44, 42, 40, 38, 37, 35, 33, 32, 30, 28, 26, 24, 23, 22, 20, 18,
//17, 15, 14, 12, 11, 10, 8, 8, 6, 6, 4, 4, 2, 2, 1, 0};

 
int suitDifference(int array1[], int array2[], int len)
{
    int i = 0;
    int diff = 0;
    for (i = 0; i < len; i++)
        diff += (array1[i]-array2[i]) * (array1[i]-array2[i]);
    return diff;
}

//Allow testRecord to be called easily from within a dynamically linked library
extern "C"{
  int testRecord();
}

//A test function for writing to an avi file
int testRecord()
{
  CvCapture* camCapture=cvCaptureFromFile("starcraft.avi");
  
  CvVideoWriter *writer = 0;
//  int isColor = 1;
  int fps     = 10;  // or 30
  int frameW  = 1024; // 744 for firewire cameras
  int frameH  = 436; // 480 for firewire cameras
  FILE* video=fopen("out.avi","w");
  fclose(video),
  writer=cvCreateVideoWriter("out.avi",CV_FOURCC('D','I','V','X'),
                             fps,cvSize(frameW,frameH),1);
  IplImage* frame=NULL;
			     
  char key=' ';
  int count=0;
  while (key!=(char)'q' && count<500)
    {
      key=(char)cvWaitKey((char)25);
//      int okay=cvGrabFrame(camCapture);
      cvGrabFrame(camCapture);
      frame=cvRetrieveFrame(camCapture);
      cvWriteFrame(writer,frame);
      printf("Frame %d\n",++count);
    }
  
  cvReleaseVideoWriter(&writer);

  return 7;
}

//Rotate counterclockwise 90 degrees, hopefully unnecessary if our cameras aren't on sideways
void rotate90Deg(IplImage* src, IplImage* dest)
{
	char* data=src->imageData;
	char* data2=dest->imageData;
	int width=src->width;
	int height=src->height;
	if (width!=dest->height || height!=dest->width)
	{
        assert(false && "Wrong dimensions of destination image in rotation, should be transpose of src image");
	}
	int count=0;
	int count2=0;
	for (int y=0; y<height;y++)
	{
		count2=3*height*width+y*3;
		for (int x=0; x<width;x++)
		{
			count2-=(3*height);
			data2[count2]=data[count];
			data2[count2+1]=data[count+1];
			data2[count2+2]=data[count+2];
			count+=3;
		}
	}
}

//This goes clockwise, rotate90Deg goes counterclockwise.
void rotate90DegClockwise(IplImage* src, IplImage* dest)
{
	char* data=src->imageData;
	char* data2=dest->imageData;
	int width=src->width;
	int height=src->height;
	if (width!=dest->height || height!=dest->width)
	{
        assert(false && "Wrong dimensions of destination image in rotation, should be transpose of src image");
	}
	int count=0;
	int count2=0;
	for (int y=0; y<height;y++)
	{
		count2=-3-3*y;
		for (int x=0; x<width;x++)
		{
			count2+=(3*height);
			data2[count2]=data[count];
			data2[count2+1]=data[count+1];
			data2[count2+2]=data[count+2];
			count+=3;
		}
	}
}	

//Used for camera calibration, this finds corners on a chessboard
//Currently expecting a 6 by 6 chessboard
int findCorners(IplImage* image, CvPoint2D32f* array/*size 36*/)
{
    cout<<"Starting findCorners"<<endl;
//	int i, j;
	
	// 8 bit image:
	IplImage* image8 = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
	cvCvtColor(image,image8,CV_BGR2GRAY);
	
	int cornerCount;

    cout<<"Guessing chessboard corners"<<endl;
	//The cvSize(6,6) refers to the size of the chessboard
    int okay = cvFindChessboardCorners(image8, cvSize(6,6), array, &cornerCount);
	cvDrawChessboardCorners( image, cvSize(6,6),
							array, cornerCount, okay);

    cout<<"Was it okay?:"<<okay<<endl;
    cout<<"Finished guessing chessboard corners"<<endl;
    if (okay)
      {
			cout<<"calling subPix"<<endl;
			cvFindCornerSubPix(image8, array, cornerCount, 
				cvSize(4,4), cvSize(5,5), cvTermCriteria(CV_TERMCRIT_ITER, 20, 0.025));  
			cvDrawChessboardCorners( image, cvSize(6,6),
						array, cornerCount, okay);

			cout<<"Finished subPix"<<endl;
      }
    else
      {
		cornerCount=-1;
      }
	cvReleaseImage(&image8);
	cout<<"CornerCount:"<<cornerCount<<endl;
    return cornerCount;
}


//  Calibrate the camera, see the openCV manual, this just hands it off
//	float distortion[4];
//	float cameraMatrix[9];
//	float transVects[3];
//	float rotMat[9];
void calibrateCamera(int width, int height, int* cornerCountsArray, float* distortion, float* cameraMatrix, float* transVects, float* rotMat, int numImages, CvPoint2D32f* array, CvPoint3D32f* buffer)
{
	cvCalibrateCamera(numImages, cornerCountsArray,cvSize(width,height),
		array, buffer,
		distortion, cameraMatrix,
		transVects, rotMat, 0);
}

//  Undistorts the image using undistort, in theory we should use cvUndistort to calculate the distortion matrix
//  This calculates the distortion matrix every time we want to undistort an image.
void undistort(IplImage* image, IplImage* dest, float* cameraMatrix, float* distortion)
{
	cvUnDistortOnce( image, dest,
                      cameraMatrix,
                      distortion,
                      1);
}

//Allow giveMeFive to be called from within dynamically linked library more easily
extern "C"{
  int giveMeFive();
}
//A simple function to test out dynamically linked libraries.
int giveMeFive(){
  return 5;
}

//A way to kill the vision loop, at least in theory, in fact this only controls the visionStart function
static int goVision=1;
extern "C"{
  void killVision();
}
void killVision(){
  goVision=0;
}

//Detect a gate, cut off the top half of the image, then go through the image counting up
//the number of white pixels in each column of the image.  If there are a set of consecutive columns with high pixel counts
//This implies one pipe of the gate is in the image, if there is an area with a pipe on each side, it means we're facing the center of the gate.
int gateDetect(IplImage* percents, IplImage* base, int* gatex, int* gatey)
{
	unsigned char* data=(unsigned char*)percents->imageData;
	unsigned char* data2=(unsigned char*)base->imageData;
	int width=percents->width;
	int height=percents->height;

	cout<<"width="<<width<<"height="<<height<<endl;

	int* columnCounts=(int*) calloc(sizeof(int),width);
	int count=0;
//	int pixel_count=0;
	int r=0;
	int g=0;
	int b=0;
	int r2=0;
	int g2=0;
	int b2=0;
	int total=0;
//	int total2=0;
	
	int whitex=0;
	int whitey=0;
	
//	int xdist=0;
//	int ydist=0;
	int minx=999999;
	int maxx=0;
	int miny=999999;
	int maxy=0;

//Not necessary to black out the image, just ignore this piece of it
//	count=0;
//	for (int y=0; y<height/4; y++)
//		for (int x=0; x<width;x++)
//		{
//			data2[count+2]=data2[count+1]=data2[count]=0;
//			count+=3;
//		}
//	

	for (int y=height/2; y<height-2; y++)
	{
		count=3*2+3*width*y;
		for (int x=2; x<width-2; x++)
		{
			b=(data[count]+256)%256;
			g=(data[count+1]+256)%256;
			r=(data[count+2]+256)%256;
			b2=(data2[count]+256)%256;
			g2=(data2[count+1]+256)%256;
			r2=(data2[count+2]+256)%256;
			if (b>20 && g>20 && r>15)
			{
				if (b2>100 && g2>100 && r2>50)
				{
					data2[count]=255;
					data2[count+1]=255;
					data2[count+2]=255;
					whitex+=x;
					whitey+=y;
					total++;
					minx=min(x,minx);
					maxx=max(x,maxx);
					miny=min(y,miny);
					maxy=max(y,maxy);
					++columnCounts[x];
				}
				else
					data2[count]=data2[count+1]=data2[count+2]=0;
			}
			else
				data2[count]=data2[count+1]=data2[count+2]=0;
			count+=3;
		}
	}
	if (total>500)
	  {
	        whitex/=total;
	        whitey/=total;       
		int indexR = 0;
		int indexL  =0;
		*gatex=whitex;
		*gatey=whitey;
		int testCol=columnCounts[whitex];
		int state=0;
		for (indexR=whitex;indexR<width;indexR++)
		{
			if (columnCounts[indexR]>2*testCol)
				state++;
			else
				state=0;
			if (state==5)
				break;
		}
		if (state==5)
		{
			for (indexL=whitex;indexL>0;indexL--)
			{
				if (columnCounts[indexL]>2*testCol)
					state++;
				else
					state=0;
				if (state==5)
					break;
			}
		}
		if (state!=5)
		{
			//No gate here
			whitex=whitey=-1;
		}
		else
		{
			count=3*indexL+3*width*whitey;
			for (int x=indexL; x<indexR;x++)
			{
				data2[count]=data2[count+2]=0;
				data2[count+1]=255;
				count+=3;
			}
			whitex=(indexL+indexR)/2;
		}
	}
	else
	{
		whitex=whitey=-1;
	}
	free(columnCounts);
	return whitex!=-1;
}
	
	//subtract two images, display the result
	void diff(IplImage* img, IplImage* oldImg, IplImage* destination)
	{
		char* data=img->imageData;
		char* data2=oldImg->imageData;
		char* dest=destination->imageData;
		int width=img->width;
		
		if (width!=oldImg->width || width!=destination->width)
		{
			//cout<<"Error, width changed"<<endl;
			return;
		}
		int height=img->height;
		if (height!=oldImg->height || height!=destination->height)
		{
			//cout<<"error, height changed"<<endl;
			return;
		}
		
		int	count=0;
		int r;
		int g;
		int b;
		int r2;
		int g2;
		int b2;
		for (int y=0; y<height; y++)
			for (int x=0; x<width; x++)
			{
				b=(data[count]+256)%256;
				g=(data[count+1]+256)%256;
				r=(data[count+2]+256)%256;
				b2=(data2[count]+256)%256;
				g2=(data2[count+1]+256)%256;
				r2=(data2[count+2]+256)%256;
				dest[count]=(char)abs(b-b2);
				dest[count+1]=(char)abs(g-g2);
				dest[count+2]=(char)abs(r-r2);
//				dest[count]=data[count]-data2[count];
//				dest[count+1]=data[count+1]-data2[count+1];
//				dest[count+2]=data[count+2]-data2[count+2];
				count+=3;
			}
//		count=0;
//		int numPerEight=0;
//			for (int x=0;x<width*height/8;x++)
//			{
//				numPerEight=0;
//				for (int z=0; z<8;z++)
//				{
//				
//					if (dest[count]>5 && dest[count+1]>5 && dest[count+2]>5)
//					{
//						numPerEight++;
//					}
//					count+=3;
//				}
//				if (numPerEight>3)
//				
//					dest[count-24]=dest[count-23]=dest[count-22]=dest[count-21]=dest[count-20]=dest[count-19]=
//					dest[count-18]=dest[count-17]=dest[count-16]=dest[count-15]=dest[count-14]=dest[count-13]=
//					dest[count-12]=dest[count-11]=dest[count-10]=dest[count-9] =dest[count-8] =dest[count-7]=
//					dest[count-6] =dest[count-5] =dest[count-4 ]=dest[count-3] =dest[count-2] =dest[count-1]=255;
//				else
//					dest[count-24]=dest[count-23]=dest[count-22]=dest[count-21]=dest[count-20]=dest[count-19]=
//					dest[count-18]=dest[count-17]=dest[count-16]=dest[count-15]=dest[count-14]=dest[count-13]=
//					dest[count-12]=dest[count-11]=dest[count-10]=dest[count-9] =dest[count-8] =dest[count-7]=
//					dest[count-6] =dest[count-5] =dest[count-4 ]=dest[count-3] =dest[count-2] =dest[count-1]=0;
//			}
	}
	
	//cvHough
	//this returns an angle, or -10 (HOUGH_ERROR) on error
double hough(IplImage* img, int* linex, int* liney)
{
	IplImage* color_dst = img;//cvLoadImage("DSC00099.jpg", 0 );
	IplImage* src = cvCreateImage(cvGetSize(img), 8, 1);
	cvCvtColor(img,src,CV_BGR2GRAY);
	
	IplImage* dst;
	
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* lines = 0;
	int i;
	
	//if( !src )
	//	return;
	
	dst = cvCreateImage( cvGetSize(src), 8, 1 );
	//		color_dst = cvCreateImage( cvGetSize(src), 8, 3 );
	
	//		IplConvKernel* element = cvCreateStructuringElementEx(2*2+1, 2*2+1, 2, 2, CV_SHAPE_RECT, 0 );
	//		cvDilate(dst,dst,element);
	//		cvCvtColor( dst, color_dst, CV_GRAY2BGR );	
	cvCanny( src, dst, 50, 200, 3 );
	//		cvShowImage("Hough",dst);
	
#if 0
	lines = cvHoughLines2( dst, storage, CV_HOUGH_STANDARD, 1, CV_PI/180, 10, 0, 0 );
	
	for( i = 0; i < MIN(lines->total,100); i++ )
	{
		float* line = (float*)cvGetSeqElem(lines,i);
		float rho = line[0];
		float theta = line[1];
		CvPoint pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 4000*(-b));
		pt1.y = cvRound(y0 + 4000*(a));
		pt2.x = cvRound(x0 - 4000*(-b));
		pt2.y = cvRound(y0 - 4000*(a));
		cvLine( color_dst, pt1, pt2, CV_RGB(255,0,0), 3, CV_AA, 0 );
	}
#else
	lines = cvHoughLines2( dst, storage, CV_HOUGH_PROBABILISTIC, 3, CV_PI/180, 50, 70, 10 );
	CvPoint start,end;
	
	start.x=start.y=end.x=end.y=0;
	int missed = 0;
	int temp;
	double angle;
	for( i = 0; i < lines->total; i++ )
	{
		CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
		if (((line[0].x < 5)&&(line[1].x < 5)) || ((line[0].x > color_dst->width-5)&&(line[1].x > color_dst->width-5)) 
			|| ((line[0].y < 5)&&(line[1].y < 5)) || ((line[0].y > color_dst->height-5)&&(line[1].y > color_dst->height-5))) 
		{
			missed++;
			//cout<<"Missed++"<<endl;
		}
		else 
		{
			if(line[0].y < line[1].y)
			{
				temp = line[0].y;
				line[0].y = line[1].y;
				line[1].y = temp;
				temp = line[0].x;
				line[0].x = line[1].x;
				line[1].x = temp;
			}
			start.x += line[0].x;
			start.y += line[0].y;
			end.x += line[1].x;
			end.y += line[1].y;
		}
		
		cvLine( color_dst, line[0], line[1], CV_RGB(255,0,0), 3, CV_AA, 0 );
	}
	int total = lines->total - missed;
	if (total>0)
	{
		start.x/=total;
		start.y/=total;
		end.x/=total;
		end.y/=total;
		
		int xdiff = (end.x - start.x);		//deal with y's being flipped
		int ydiff = -1*(end.y - start.y);
		angle = atan2((double)(ydiff/(double)img->height),(double)(xdiff/(double)img->height));
		//cout<<"Xdiff: "<<xdiff<<endl;
		//cout<<"Ydiff: "<<ydiff<<endl;
		//cout<<"Angle: "<<angle<<endl; 
		//cout<<"startx: "<<start.x<<" endx: "<<end.x<<endl;
		
		*linex=(start.x+end.x)/2;
		*liney=(start.y+end.y)/2;
		
		cvLine(color_dst,start,end,CV_RGB(255,0,255), 3, CV_AA, 0);
		cvReleaseImage(&src);
                cvReleaseImage(&dst);
                cvReleaseMemStorage(&storage);

		return angle;
	}
#endif
    cvReleaseImage(&src);
    cvReleaseImage(&dst);
    cvReleaseMemStorage(&storage);
	return HOUGH_ERROR;
	//		cvShowImage( "Hough", color_dst );
}


//Only meant for use on images after correct has been called on them.
int mask_red(IplImage* img, bool alter_img, int threshold)
{
	unsigned char* data=(unsigned char*)img->imageData;
	int width=img->width;
	int height=img->height;
	int count=0;
	int pixel_count=0;
	int r=0;
	int g=0;
	int b=0;

	for (int y=0; y<height; y++)
		for (int x=0; x<width; x++)
		{
			b=(data[count]+256)%256;
			g=(data[count+1]+256)%256;
			r=(data[count+2]+256)%256;
			
			if (r>g && r>b && r>threshold)
			{
				pixel_count++;
				if (alter_img)
				{
					data[count]=255;
					data[count+1]=255;
					data[count+2]=255;
				}
			}
			else
			{
				if (alter_img)
				{
					data[count]=0;
					data[count+1]=0;
					data[count+2]=0;
				}
			}
			
			count+=3;
		}
	return pixel_count;
}

#define OUTSIZE 5
typedef struct
{
	int x;
	int y;
}Pos;
typedef std::list<Pos> PosList;


void safeMask(IplImage* base, double r_over_g_min, double r_over_g_max, double b_over_r_max, int minTotal)
{
	unsigned char* data=(unsigned char*)base->imageData;
//    unsigned char* dataPercents = (unsigned char*) percents->imageData;
	int width=base->width;
	int height=base->height;
	int count=0;
	unsigned char r=0;
	unsigned char g=0;
	unsigned char b=0;
    
//    r_over_g_min=.5f;
//    r_over_g_max=1.8f;
//    b_over_r_max=1.25f;

	for (int y=0; y<height; y++)
		for (int x=0; x<width; x++)
		{
			b=data[count];
			g=data[count+1];
			r=data[count+2];
			
			if (r>r_over_g_min*g && r_over_g_max*g>r && b_over_r_max*r>b && r+g+b >= minTotal)
                                data[count]=data[count+1]=data[count+2]=255;
			else
				data[count]=data[count+1]=data[count+2]=0;
			count+=3;
		}
}

//For crappy bluish camera (not on robot)
//Use "if (data[count+2]>34 && data2[count+2]>75)// && data2[count]<75 && data2[count+1]<75)"
void suitMask(IplImage* percents, IplImage* base)
{
	unsigned char* data=(unsigned char*)percents->imageData;
	unsigned char* data2=(unsigned char*)base->imageData;
	int width=percents->width;
	int height=percents->height;
    int count=0;
    for (int y =0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (data[count+2]>35 && data2[count+2] > 125 && data2[count]<170 && data2[count+1]<170)
            {
                data2[count]=data2[count+1]=data2[count+2]=255;
            }
            else
            {
                data2[count]=data2[count+1]=data2[count+2]=0;
            }
            count+=3;
        }
    }
}

void redMask(IplImage* percents, IplImage* base,
             int redPercent, int redIntensity)
{
	unsigned char* data=(unsigned char*)percents->imageData;
	unsigned char* data2=(unsigned char*)base->imageData;
	int width=percents->width;
	int height=percents->height;
//    printf("width : %d height : %d vs width : %d height : %d", width, height, base->width, base->height);
	int r;
	int r2;
	int count=0;
	
//To remove top half of image, insert this, and start y at height/2 in the next set of loops
//	for (int y=0; y<height/4;y++)
//		for (int x=0;x<width;x++)
//		{
//			data2[count]=data2[count+1]=data2[count+2]=0;
//			count+=3;
//		}
//			
	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
//            data[count]=data2[count]=data2[count+1]=data2[count+2]=0;
            r = data[count+2];//r = percent red in the image
			r2 = data2[count+2];//r2 = intensity of red in the image
			if (r > redPercent && r2 > redIntensity) 
			{
				data2[count]=255;
				data2[count+1]=255;
				data2[count+2]=255;
			}
			else
          {
				data2[count]=data2[count+1]=data2[count+2]=0;
          }
		count+=3;
		}
	}
}

//returns size and fills redx and redy.
int redMaskAndHistogram(IplImage* percents, IplImage* base, int* redx, int* redy)
{
	unsigned char* data=(unsigned char*)percents->imageData;
	unsigned char* data2=(unsigned char*)base->imageData;
	int width=percents->width;
	int height=percents->height;
	int r,g,b;
	int r2,g2,b2;
	int count=0;
	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			b=(data[count]+256)%256;
			g=(data[count+1]+256)%256;
			r=(data[count+2]+256)%256;
			b2=(data2[count]+256)%256;
			g2=(data2[count+1]+256)%256;
			r2=(data2[count+2]+256)%256;
			if (r>35 && r2>100)
			{
				data2[count]=255;
				data2[count+1]=255;
				data2[count+2]=255;
			}
			else
				data2[count]=data2[count+1]=data2[count+2]=0;

			count+=3;
		}
	}

        int minX, minY, maxX, maxY;
	return histogram(base, redx, redy, &minX, &minY, &maxX, &maxY);
}

int redDetect(IplImage* percents, IplImage* base, int* redx, int* redy)
{
	unsigned char* data=(unsigned char*)percents->imageData;
	unsigned char* data2=(unsigned char*)base->imageData;
	int width=percents->width;
	int height=percents->height;

	int count=0;
//	int pixel_count=0;
	int r=0;
	int g=0;
	int b=0;
	int r2=0;
	int g2=0;
	int b2=0;
	int total=0;
//	int total2=0;
	
//	int xdist=0;
//	int ydist=0;
	int minx=999999;
	int maxx=0;
	int miny=999999;
	int maxy=0;
	int rx = 0;
	int ry = 0;
	
	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			b=(data[count]+256)%256;
			g=(data[count+1]+256)%256;
			r=(data[count+2]+256)%256;
			b2=(data2[count]+256)%256;
			g2=(data2[count+1]+256)%256;
			r2=(data2[count+2]+256)%256;
			if (b>15 && g>15 && r>35)
			{
				if (b2>70 && g2>70 && r2>150)
				{
					data2[count]=255;
					data2[count+1]=255;
					data2[count+2]=255;
					rx+=x;
					ry+=y;
					total++;
					minx=min(x,minx);
					maxx=max(x,maxx);
					miny=min(y,miny);
					maxy=max(y,maxy);
				}
				else
					data2[count]=data2[count+1]=data2[count+2]=0;
			}
			else if (b>20 && g>20 && r>30)
			{
				//These pixels are whiter
				if (b2>150 && g2>150 && r2>150)
				{
					data2[count]=1;
					data2[count+1]=1;
					data2[count+2]=1;
					//Dont change anything else, just show that its white and ignore it
					//this change will allow the center of red to be whitish, as on a really bright light.
				}
				else
					data2[count]=data2[count+1]=data2[count+2]=0;
			}
			else
				data2[count]=data2[count+1]=data2[count+2]=0;
			count+=3;
		}
	}
	if (total>250)
	{
		rx/=total;
		ry/=total;
		int right=0;
		int top=0;
		int bottom=0;
		count=3*(rx)+3*width*(ry);
		//State represents count of pixels that were not found to be red
		int state=0;
		int left=0;
		for (left=rx;left>minx;left--)
		{
			if (data2[count]==0)
				state++;
			else
				state=0;
				
			if (state==3)
				break;
			count-=3;
		}
		state=0;
		count=3*(rx)+3*width*(ry);
		for (right=(rx);right<maxx;right++)
		{
			if (data2[count]==0)
				state++;
			else
				state=0;
								
			if (state==3)
				break;
			count+=3;
		}
		state=0;
		count=3*(rx)+3*width*(ry);
		for (bottom=(ry);bottom<maxy;bottom++)
		{
			if (data2[count]==0)
				state++;
			else
				state=0;
				
			if (state==3)
				break;
			count+=3*width;
		}
		state=0;
		count=3*(rx)+3*width*(ry);
		for (top=(ry);top>miny;top--)
		{
			if (data2[count]==0)
				state++;
			else
				state=0;
				
			if (state==3)
				break;
			count-=3*width;
		}
		
		//Remember, bottom > top, because y increases as you move downwards on the image
		
		if (bottom-top>3 && right-left>3)
		{
			CvPoint tl;
			CvPoint tr;
			CvPoint bl;
			CvPoint br;
			bl.x=tl.x=left;
			br.x=tr.x=right;
			bl.y=br.y=bottom;
			tl.y=tr.y=top;
			cvLine(base, tl, tr, CV_RGB(0,0,255), 3, CV_AA, 0 );
			cvLine(base, tl, bl, CV_RGB(0,0,255), 3, CV_AA, 0 );
			cvLine(base, tr, br, CV_RGB(0,0,255), 3, CV_AA, 0 );
			cvLine(base, bl, br, CV_RGB(0,0,255), 3, CV_AA, 0 );
			rx=(right+left)/2;
			ry=(top+bottom)/2;
			*redx=rx;
			*redy=ry;
			return total;
		}
		else
		{
			// Its not big enough to be the light, its nto even 10 by 10.
			// Either that or theres lots of other red found in the image.
			rx=ry=-1;
			*redx=rx;
			*redy=ry;
			return -1;
		}
	}
	else//total is less than 250 pixels
	{
		rx=ry=-1;
		*redx=rx;
		*redy=ry;
		return -1;
	}
}

//-1 on failure from too many distinct pieces, 0 if nothing at all was found,
//otherwise returns number of pixels in the largest connected white splotch in the image
//and fills centerX and centerY with its center.
int histogram(IplImage* img, int* centerX, int* centerY, int* minX, int* minY,
              int* maxX, int* maxY)
{
//	cout<<"starting histogram, beware."<<endl;
	int width=img->width;
	int height=img->height;
	unsigned char* data=(unsigned char*)img->imageData;
	int* pixelCounts=(int*) calloc(254,sizeof(int));
	//Might need longs for these two if we increase image size above 1024x1024, otherwise a full white image might overflow
	int* totalX=(int*) calloc(254,sizeof(int));
	int* totalY=(int*) calloc(254,sizeof(int));

        // To keep track of mins and maxes of the bounds of blobs
        int* totalMaxX=(int*) calloc(254, sizeof(int));
        int* totalMaxY=(int*) calloc(254, sizeof(int));
        int* totalMinX=(int*) malloc(254 * sizeof(int));
        int* totalMinY=(int*) malloc(254 * sizeof(int));
        
	unsigned char* joins=(unsigned char*) malloc(254*sizeof(unsigned char));
	for (unsigned char i=0; i<254; i++)
        {
		joins[i]=i;
                totalMinX[i] = img->width;
                totalMinY[i] = img->height;
        }
	joins[0]=255;
	
	int index=1;
	int count=0;
	for (int x=0;x<width;x++)
	{
		data[count]=data[count+1]=data[count+2]=0;
		count+=3;
	}
	count=0;
	for (int y=0;y<height;y++)
	{
		data[count]=data[count+1]=data[count+2]=0;
		count+=3*width;
	}
	count=0;
	for (int y=0; y<height;y++)
	{
		for (int x=0; x<width;x++)
		{
			if (data[count]>0)
			{
				unsigned char above=data[count-3*width];
				unsigned char left=data[count-3];
				if (above==0 && left==0)
				{
					pixelCounts[index]=1;
					totalX[index]=x;
					totalY[index]=y;
					data[count]=(unsigned char)(index++);
//					cout<<index<<endl;				
					if (index==254)
					{
						free(pixelCounts);
						free(totalX);
						free(totalY);
						free(joins);
						return -1;
					}
				}
				else 
				{
//					cout<<"I'm in part 2"<<endl;
					unsigned char above2=above;
					unsigned char left2=left;
					if (above2==0)
						above2=255;
					else
					{
					while (above2!=joins[above2])
						above2=joins[above2];
					}
					if (left2==0)
						left2=255;
					else
					{
						while (left2!=joins[left2])
							left2=joins[left2];
					}
					
					data[count]=joins[above]=joins[left]=min(left2,above2);
					totalX[data[count]]+=x;
					totalY[data[count]]+=y;
					++pixelCounts[data[count]];
//					cout<<"end of part 2"<<endl;
                                        
                                        // Min/Max
                                        if (x < totalMinX[data[count]])
                                            totalMinX[data[count]] = x;
                                        else if (x > totalMaxX[data[count]])
                                            totalMaxX[data[count]] = x;
                                        if (y < totalMinY[data[count]])
                                            totalMinY[data[count]] = y;
                                        else if (y > totalMaxY[data[count]])
                                            totalMaxY[data[count]] = y;
				}
			}
			count+=3;
		}
	}
//	cout<<"Made it through image, now creating useful data from arrays"<<endl;
	int maxCount=0;

        // Work from the top to bottom, collapsing the pixel clusters together
	for (int i=index-1;i>0;i--)
	{
                
		if (joins[i]!=i)
		{
                    // "Unfinished" cluster of pixels, ie part of bigger cluster
                    // So add all of its information to the parents information
			totalX[joins[i]]+=totalX[i];
			totalY[joins[i]]+=totalY[i];
			pixelCounts[joins[i]]+=pixelCounts[i];
			pixelCounts[i]=0;
		}
		else
		{
                    // Found a final cluster
			if (maxCount<pixelCounts[i])
			{
                            // Transfer data from the correct blob back to caller
				maxCount=pixelCounts[i];
				(*centerX)=totalX[i]/maxCount;
				(*centerY)=totalY[i]/maxCount;
                                *minX = totalMinX[i];
                                *maxX = totalMaxX[i];
                                *minY = totalMinY[i];
                                *maxY = totalMaxY[i];
			}
		}
	}

	//Deallocate arrays
	free(pixelCounts);
	free(totalX);
	free(totalY);
	free(joins);
        free(totalMinX);
        free(totalMaxX);
        free(totalMinY);
        free(totalMaxY);
//	cout<<"Happily reaching the end of histogram"<<endl;
	return maxCount;
}

void explore(IplImage* img, int x, int y, int* out, int color)
{	
	PosList toExplore;
	int width=img->width;
	int height=img->height;
	unsigned char* data=(unsigned char*)img->imageData;
	out[0]=0;
	out[1]=999999;
	out[2]=999999;
	out[3]=-999999;
	out[4]=-999999;
	
	Pos start,temp;
	start.x=x;
	start.y=y;
	toExplore.push_back(start);
	while (!(toExplore.empty()))
	{
		Pos &p=toExplore.back();
		x=p.x;
		y=p.y;
		toExplore.pop_back();
		int count=3*x+3*width*y;
		if ((data[count]>100 && data[count+1]>100 && data[count+2]>100))
		{
			out[0]++;
			out[1]=min(out[1],x);
			out[2]=min(out[2],y);
			out[3]=max(out[3],x);
			out[4]=max(out[4],y);
			
			data[count]=0;
			data[count+1]=0;
			data[count+2]=(unsigned char)color;
			
			if (x>0)
			{
				temp.x=x-1;
				temp.y=y;
				toExplore.push_back(temp);
			}
			if (x<width-1)
			{
				temp.x=x+1;
				temp.y=y;
				toExplore.push_back(temp);
			}
			if (y>0)
			{
				temp.x=x;
				temp.y=y-1;
				toExplore.push_back(temp);
			}
			if (y<height-1)
			{
				temp.x=x;
				temp.y=y+1;
				toExplore.push_back(temp);
			}
		}
	}
}
CvPoint find_flash(IplImage* img, bool display)
{
	int width=img->width;
	int height=img->height;
	unsigned char* data=(unsigned char*)img->imageData;
	CvPoint center;
	int out[5];
	int count=0;
	int color=175;
	for (int y=0; y<height; y++)
		for (int x=0; x<width;x++)
		{
			out[0]=0;
			out[1]=999999;
			out[2]=-999999;
			out[3]=999999;
			out[4]=-999999;
			

			if (data[count]>0&&data[count+1]>0&&data[count+2]>0)
			{
				explore(img,x,y,out,color);
				color+=20;
				CvPoint bl,br,tl,tr;
				tl.x=bl.x=out[1];
				tr.x=br.x=out[3];
				tl.y=tr.y=out[4];
				bl.y=br.y=out[2];
				int w=out[3]-out[1];
				int h=out[4]-out[2];
				center.x=(out[3]+out[1])/2;
				center.y=(out[4]+out[2])/2;
				
				if (display && out[0]>25)
				{
					//cout<<"Data from explore"<<endl<<"Count: "<<out[0]<<"min x: "<<out[1]<<"min y: "<<out[2]<<"max x: "<<out[3]<<"max y: "<<out[4]<<endl; 
				}
				if (out[0]>25 && w>5 && w<200 && h>5 && h<200)
				{
					cvLine(img, tl, tr, CV_RGB(0,0,255), 3, CV_AA, 0 );
					cvLine(img, tl, bl, CV_RGB(0,0,255), 3, CV_AA, 0 );
					cvLine(img, tr, br, CV_RGB(0,0,255), 3, CV_AA, 0 );
					cvLine(img, bl, br, CV_RGB(0,0,255), 3, CV_AA, 0 );
					return center;
				}
			}
			count+=3;
		}
	//Return a -1,-1 point if not found
	center.x=-1;
	center.y=-1;
	return center;
}

int guess_line(IplImage* img)
{
	char* data=img->imageData;
	int width=img->width;
	int height=img->height;

	//What??
	//int avgxs[height];
	int *avgxs = new int[height];

//	int r=0;
//	int g=0;
	int b=0;
	int count=0;
//	int minx=0;
//	int maxx=0;
//	int miny=0;
	int avgx=0;
	int countOfx=0;
	int temp;
	int goRight=0;
	bool on=false;
	for (int y=0; y<height; y++)
	{
		countOfx=0;
		int sumx=0;
		for (int x=0; x<width; x++)
		{
		
			b=(data[count]+256)%256;

			if (b==255 && on==false)
			{
				sumx+=x;
				countOfx++;
			}
			count+=3;
		}			
		avgx=sumx/countOfx;
		avgxs[y]=avgx;
	}	
	
	for (int j=0;j<5;j++) {
		for (int y=0;y<height-1;y++) {
			if ((avgxs[y] != 0) && (avgxs[y+1] != 0)) {
				temp = (avgxs[y] + avgxs[y+1]) / 2;
				avgxs[y] = temp;
				avgxs[y+1] = temp;
			}
		}
	}
	for (int y=0; y<height; y++)
	{
		avgx=avgxs[y];
		if (avgx==0)
			continue;
		goRight-=(width/2)-avgx;
		data[3*avgx+3*width*y]=0;
		data[3*avgx+3*width*y+1]=0;
	}
	
	distance_from_line(avgxs,img);
	angle_from_center(avgxs, img);
	goRight/=countOfx;
	//cout<<endl<<goRight<<endl;
	delete avgxs;
	return goRight;
}
		

int mask_orange(IplImage* img, bool alter_img, int brightness, bool strict,
		double r_over_g_min, double r_over_g_max, double b_over_r_max)
{
	strict=true;
	unsigned char* data=(unsigned char*)img->imageData;
	int width=img->width;
	int height=img->height;
	int count=0;
	int pixel_count=0;
	int r=0;
	int g=0;
	int b=0;
//	float r_over_g_min;
//	float r_over_g_max;
//	float b_over_r_max;
	int acceptable=0;

//	if (!strict)
//	{
//		r_over_g_min=1.0f;
//		r_over_g_max=2.1f;
//		b_over_r_max=0.6f;
//	}
//	else//(strict)
//	{
//		r_over_g_min=1.0f;
//		r_over_g_max=2.0f;
//		b_over_r_max=0.4f;
//	}

//Competition Values	
//	if (!strict)
//	{
//		r_over_g_min=1.0;
//		r_over_g_max=2.5;
//		b_over_r_max=.5;
//	}
//	else
//	{
//		r_over_g_min=1.2;
//		r_over_g_max=2.0;
//		b_over_r_max=.4;
//	}
	for (int y=0; y<height; y++)
		for (int x=0; x<width; x++)
		{
			b=(data[count]+256)%256;
			g=(data[count+1]+256)%256;
			r=(data[count+2]+256)%256;
			
			//if (b<0 || g<0 || r<0 || b>255 ||g>255||r>255)
				//cout<<"WTF?!?!?!?!?!"<<g <<endl;
			acceptable=0;
			if (r>r_over_g_min*g)
				++acceptable;
			if (r_over_g_max*g>r)
				++acceptable;
			if ( b_over_r_max*r>b)
				++acceptable;
			// A new test because dark black squares shouldn't be
                        // orange. (default should be 100)
			if (r+g+b < brightness)
				acceptable=0;
			if (acceptable>=2+(int)strict)
			{
				if (alter_img)
					data[count]=data[count+1]=data[count+2]=255;
				pixel_count++;
			}
			else if (alter_img)
				data[count]=data[count+1]=data[count+2]=0;
			count+=3;
		}
		
	return pixel_count;
}

void thin_blue_line(IplImage* img)
{
	unsigned char* data=(unsigned char*)img->imageData;
	int width=img->width;
	int height=img->height;
	int half_width=width/2;
	
	for (int y=0; y<height; y++)
	{
		data[3*half_width+3*width*y]=255;
		data[3*half_width+3*width*y+1]=0;
		data[3*half_width+3*width*y+2]=0;
	}
}


int distance_from_line(int avgxs[], IplImage* img) {
	int height = img->height;
	int *half_avgxs = new int[height/2];
	int count = 0;
	int sum_of_abs = 0;
	for(int i=height/2;i<height;i++) {
		half_avgxs[count] = avgxs[i];
	}
	for(int i=0;i<height/2;i++) {
		sum_of_abs += abs(half_avgxs[i]);
	}
	//cout<<sum_of_abs<<endl;
	delete half_avgxs;
	return sum_of_abs;
}

int angle_from_center(int argxs[], IplImage* img) {
	double startx,starty,endx,endy;
	int height = img->height;
	starty = 0;
	startx = argxs[(int)starty];
	endy = height/2;
	endx = argxs[(int)endy];
	//cout<<atan((endx-endy)/(endy-starty));
	return static_cast<int>(atan((endx-endy)/(endy-starty)));
}
	
void correct(IplImage* img)
{
	unsigned char* data=(unsigned char*)img->imageData;
	int width=img->width;
	int height=img->height;
	
	srand ( (unsigned int)time(NULL) );
	
	int red_sum=0;
	int green_sum=0;
	int blue_sum=0;
	int r;
	int g;
	int b;
	int total=0;
	
	for (int to_avg=0; to_avg<100;to_avg++)
	{
		int x = rand() % width;
		int y = rand() % height;
		int count=3*x+3*width*y;
	
		b=(data[count]+256)%256;
		g=(data[count+1]+256)%256;
		r=(data[count+2]+256)%256;
		
		if (b>r && g>r)//its a watery color
		{
			++total;
			blue_sum+=b;
			green_sum+=g;
			red_sum+=r;
		}
	}	
	
	r=red_sum/total;
	g=green_sum/total;
	b=blue_sum/total;

	g=g-r;
	b=b-r;	
	r=0;

	//cout<<"Color of water:" <<r << " "<<g<<" "<< b<<endl;
	int r2;
	int g2;
	int b2;
	int	diffr;
	int diffg;
	int diffb;
	int count=0;

	
	for (int y=0; y<height; y++)
		for (int x=0; x<width; x++)
		{
			b2=(data[count]+256)%256;
			g2=(data[count+1]+256)%256;
			r2=(data[count+2]+256)%256;
			diffr=r2-r;
			diffg=g2-g;
			diffb=b2-b;
			
			//Remove Grays
			int max=MAX(MAX(diffr,diffg),diffb);
			int min=MIN(MIN(diffr,diffg),diffb);
			
			if (max-min < .250 * (max + min) )
			{
				data[count]=0;
				data[count+1]=0;
				data[count+2]=0;
			}
			else
			{
				if (diffb<=0)
					data[count]=0;
				else
					data[count]=(char)diffb;
			
				if (diffg<=0)
					data[count+1]=0;
				else
					data[count+1]=(char)diffg;
			
				if (diffr<=0)
					data[count+2]=0;
				else
					data[count+2]=(char)diffr;
			}
			count+=3;
		}
}

void filter(IplImage* img, bool red_flag, bool green_flag, bool blue_flag)
{
	char* data=img->imageData;
	int width=img->width;
	int height=img->height;
	int count=0;
	
	for (int y=0; y<height; y++)
		for (int x=0; x<width; x++)
		{
			if (!blue_flag)
				data[count]=0;
			if (!green_flag)
				data[count+1]=0;
			if (!red_flag)
				data[count+2]=0;
			count+=3;
		}
	
}

void to_ratios(IplImage* img)
{
    unsigned char* data = (unsigned char*)img->imageData;
    int length = img->width * img->height * 3;
    int r = 0;
    int g = 0;
    int b = 0;
    int sum = 0;
    
    for (int i = 0; i < length; i += 3)
    {
        b = data[i];
        g = data[i+1];
        r = data[i+2];
        sum = r + b + g;

        // Fixed point version (conditional needed for divede by zero)
        if (sum)
        {
            data[i] = (100 * b) / sum;
            data[i+1] = (100 * g) / sum;
            data[i+2] = (100 * r) / sum;
        }
        else
        {
            data[i] = 33;
            data[i+1] = 33;
            data[i+2] = 33;
        }
    }
}

int red_blue(IplImage* img, float ratio)
{
	char* data=img->imageData;
	int width=img->width;
	int height=img->height;
	int count=0;
//	int pixel_count=0;
	int r=0;
	int g=0;
	int b=0;
	int total=0;
	for (int y=0; y<height; y++)
		for (int x=0; x<width; x++)
		{
			b=(data[count]+256)%256;
			g=(data[count+1]+256)%256;
			r=(data[count+2]+256)%256;
			for (float z=1.0f;z>=0.7f;z-=0.1f)
				if (r>ratio*b*z)
				{
                                    data[count]=static_cast<char>(255*z);
                                    data[count+1]=static_cast<char>(255*z);
                                    data[count+2]=static_cast<char>(255*z);
                                    total+=static_cast<char>(10*z);
					break;
				}
			count+=3;
		}
		
	return total;
}



//Good ones are 30 and 190 for test images at least
/* Masks the image for white, all parameters must be the same size,
   output is filled with either 0's or 255s*/
int white_mask(IplImage* percents, IplImage* base, IplImage* output, unsigned char minPercentIntensity, unsigned char minIntensity)
{
	unsigned char* data=(unsigned char*)percents->imageData;
	unsigned char* data2=(unsigned char*)base->imageData;
	unsigned char* data3=(unsigned char*)output->imageData;
	int width=percents->width;
	int height=percents->height;
	int count=0;
	int r=0;
	int g=0;
	int b=0;
	int r2=0;
	int g2=0;
	int b2=0;
	int pixelCount = 0;
	if (percents->width != base->width || percents->width != output->width ||
		percents->height != base->height || percents->height != output->height)
		{
			assert(false && "Unmatched image width/height in white_mask, all parameters should have same width and all parameters should have same height");
		}

	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			b=data[count];
			g=data[count+1];
			r=data[count+2];
			b2=data2[count];
			g2=data2[count+1];
			r2=data2[count+2];
			if (b>minPercentIntensity && g>minPercentIntensity && r>minPercentIntensity)
			{
				if (b2>minIntensity && g2>minIntensity && r2>minIntensity)
				{
					data3[count]=data3[count+1]=data3[count+2]=255;
					pixelCount++;
				}
				else
				{
                    data3[count]=data3[count+1]=data3[count+2]=0;
				}
			}
			else
			{
                data3[count]=data3[count+1]=data3[count+2]=0;
            }
			count+=3;
		}
	}
	return pixelCount;
}

//Good parameters are 15 and 350, except for stupid cam, for which drop it to 15 150
/* masks the images for black. output is filled with 255s wherever base and percents were in black thresholds,
filled with 0s elsewhere*/
int black_mask(IplImage* percents, IplImage* base, IplImage* output, unsigned char minPercentIntensity, int maxTotalIntensity)
{
	unsigned char* data=(unsigned char*)percents->imageData;
	unsigned char* data2=(unsigned char*)base->imageData;
	unsigned char* data3=(unsigned char*)output->imageData;
	int width=percents->width;
	int height=percents->height;
	int count=0;
	int r=0;
	int g=0;
	int b=0;
	int r2=0;
	int g2=0;
	int b2=0;
	int pixelCount = 0;
	if (percents->width != base->width || percents->width != output->width ||
		percents->height != base->height || percents->height != output->height)
		{
			assert(false && "Unmatched image width/height in white_mask, all parameters should have same width and all parameters should have same height");
		}

	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			b=data[count];
			g=data[count+1];
			r=data[count+2];
			b2=data2[count];
			g2=data2[count+1];
			r2=data2[count+2];
			if (b>minPercentIntensity && g>minPercentIntensity && r>minPercentIntensity)
			{
				if (b2 + g2 + r2 <= maxTotalIntensity)
				{
					data3[count]=data3[count+1]=data3[count+2]=255;
					pixelCount++;
				}
				else
				{
                    data3[count]=data3[count+1]=data3[count+2]=0;
				}
			}
			else
			{
                data3[count]=data3[count+1]=data3[count+2]=0;
            }
			count+=3;
		}
	}
	return pixelCount;
}
