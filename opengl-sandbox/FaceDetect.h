#pragma once
#ifndef FACE_DETECT_H
#define FACE_DETECT_H

#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/face/facemark.hpp>

class FaceDetect
{
public:
	FaceDetect();

	void getFaceLandmarks(unsigned char* image, int width, int height);

	cv::CascadeClassifier face_cascade;
	cv::Ptr<cv::face::Facemark> facemark;
};

#endif	//FACE_DETECT_H