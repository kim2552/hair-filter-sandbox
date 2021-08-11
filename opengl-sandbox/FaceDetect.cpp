#include "FaceDetect.h"

FaceDetect::FaceDetect()
{
	face_cascade.load("assets/data/lbpcascade_frontalface_improved.xml");
	facemark = cv::face::createFacemarkKazemi();
	facemark->loadModel("assets/data/face_landmark_model.dat");
	printf("Loaded model.");
}

//TODO::We will be using YUV image which will isolate gray channel image, so we don't have to conver to gray image
void FaceDetect::getFaceLandmarks(unsigned char* image, int width, int height)
{
	cv::Mat imageMat = cv::Mat(height, width, CV_8UC3, image);

	float aspect_ratio = (float)height / (float)width;

	cv::resize(imageMat, imageMat, cv::Size(460, (int)(460*aspect_ratio)), 0, 0, cv::INTER_LINEAR_EXACT);
	cv::Mat gray;
	std::vector<cv::Rect> faces;
	if (imageMat.channels() > 1) {
		cvtColor(imageMat, gray, cv::COLOR_BGR2GRAY);
	}
	else {
		gray = imageMat.clone();
	}
	cv::equalizeHist(gray, gray);
	face_cascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(30, 30));

	if (faces.size() > 0) {

		std::vector<std::vector<cv::Point2f>> shapes;
		if (facemark->fit(imageMat, faces, shapes))	//TODO::transfer shapes coordinates to openGL!
		{
			//for (size_t i = 0; i < faces.size(); i++)
			//{
			//	cv::rectangle(imageMat, faces[i], cv::Scalar(255, 0, 0));
			//}
			for (unsigned long i = 0; i < faces.size(); i++) {
				for (unsigned long k = 0; k < shapes[i].size(); k++)
					cv::circle(imageMat, shapes[i][k], 5, cv::Scalar(0, 0, 255), cv::FILLED);
			}
			cv::namedWindow("Detected_shape");
			cv::imshow("Detected_shape", imageMat);
			cv::waitKey(0);
		}
	}
}