#include "FaceDetect.h"

FaceDetect::FaceDetect()
{
	face_cascade.load("assets/data/lbpcascade_frontalface_improved.xml");
	//facemark = cv::face::createFacemarkKazemi();
	//facemark->loadModel("assets/data/face_landmark_model.dat");
	printf("Loaded model.");

	mask.create(cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT), CV_8UC1);
	if (!mask.empty())
		mask.setTo(cv::Scalar::all(cv::GC_BGD));
	markers.create(cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT), CV_8UC1);
	markers.setTo(cv::Scalar(0.0));	//cv::Scalar(255)	cv::GC_BGD

	//face_detector = dlib::get_frontal_face_detector();
	dlib::deserialize("assets/data/shape_predictor_81_face_landmarks.dat") >> shape_predictor;
}

//TODO::We will be using YUV image which will isolate gray channel image, so we don't have to conver to gray image
std::vector<std::vector<cv::Point>> FaceDetect::getFaceLandmarks(unsigned char* image, int width, int height)
{
	cv::Mat imageMat = cv::Mat(height, width, CV_8UC3, image);

	std::vector<std::vector<cv::Point>> shapes;

	cv::resize(imageMat, imageMat, cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT), 0, 0, cv::INTER_LINEAR_EXACT);
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

	std::vector < dlib::full_object_detection > dlibshapes;
	for (unsigned long j = 0; j < faces.size(); ++j)
	{
		dlib::rectangle rect = dlib::rectangle((long)faces[j].tl().x, (long)faces[j].tl().y, (long)faces[j].br().x - 1, (long)faces[j].br().y - 1);
		dlib::array2d<dlib::rgb_pixel> img_dlib;
		dlib::assign_image(img_dlib, dlib::cv_image<unsigned char>(gray));
		dlib::full_object_detection shape = shape_predictor(img_dlib, rect);
		std::cout << "number of parts: " << shape.num_parts() << std::endl;
		std::cout << "pixel position of first part:  " << shape.part(0) << std::endl;
		std::cout << "pixel position of second part: " << shape.part(1) << std::endl;
		// You get the idea, you can get all the face part locations if
		// you want them.  Here we just store them in shapes so we can
		// put them on the screen.
		dlibshapes.push_back(shape);
		for (size_t i = 0; i < faces.size(); i++)
		{
			cv::rectangle(imageMat, faces[i], cv::Scalar(0, 255, 0));
		}
		for (size_t i = 0; i < faces.size(); i++)
		{
			cv::Rect rc(cv::Point2i(shape.get_rect().left(), shape.get_rect().top()), cv::Point2i(shape.get_rect().right() + 1, shape.get_rect().bottom() + 1));
			cv::rectangle(imageMat, rc, cv::Scalar(255, 0, 0));
		}
		std::vector<cv::Point> pts;
		for (unsigned long k = 0; k < shape.num_parts(); k++) {
			cv::Point p;
			p.x = shape.part(k).x();
			p.y = shape.part(k).y();
			pts.push_back(p);
			cv::circle(imageMat, p, 5, cv::Scalar(0, 0, 255), cv::FILLED);
		}
		std::vector<cv::Point> contour_hull;
		cv::convexHull(pts, contour_hull);
		std::vector<std::vector<cv::Point>> hulls;
		hulls.push_back(contour_hull);
		cv::drawContours(markers, hulls, 0, cv::Scalar(255),-1);
		cv::Mat result;

		//cv::bitwise_and(imageMat, markers, result);
		//cv::namedWindow("Detected_shape");
		//cv::imshow("Detected_shape", markers);
		//cv::waitKey(0);
		cv::resize(markers, markers, cv::Size(width, height), 0, 0, cv::INTER_LINEAR_EXACT);

		face_mask_image = markers.data;

		shapes.push_back(pts);
	}

//
//	if (faces.size() > 0) {
//#if ENABLE_DEBUG
//		if (facemark->fit(imageMat, faces, shapes))
//		{
//			std::vector<cv::Point> pts;
//			int lowestY =0, lowestX =0, highestY = RESIZED_IMAGE_HEIGHT, highestX = RESIZED_IMAGE_WIDTH;
//			for (auto shape : shapes) {
//				for (int i = 0; i < shape.size();i++) {
//					if (shape[i].x > lowestX) {
//						lowestX = shape[i].x;
//					}
//					if (shape[i].y > lowestY) {
//						lowestY = shape[i].y;
//					}
//					if (shape[i].x < highestX) {
//						highestX = shape[i].x;
//					}
//					if (shape[i].y < highestY) {
//						highestY = shape[i].y;
//					}
//					pts.push_back(cv::Point(shape[i].x, shape[i].y));
//				}
//			}
//
//			for (size_t i = 0; i < faces.size(); i++)
//			{
//				//float incX = float(faces[i].width) * 0.5;
//				//float incY = float(faces[i].height);
//				//faces[i].x -= int(incX / 2);
//				//faces[i].y -= int(incY / 2);
//				//faces[i].width += incX;
//				//faces[i].height += incY;
//				//cv::rectangle(imageMat, cv::Rect(cv::Point2f(lowestX,lowestY),cv::Point2f(highestX,highestY)), cv::Scalar(255, 0, 0));
//				cv::rectangle(imageMat, faces[i], cv::Scalar(255, 0, 0));
//			}
//			for (unsigned long i = 0; i < faces.size(); i++) {
//				for (unsigned long k = 0; k < shapes[i].size(); k++)
//					cv::circle(imageMat, shapes[i][k], 5, cv::Scalar(0, 0, 255), cv::FILLED);
//			}
//			//std::vector<cv::Point> contour_hull;
//			//cv::convexHull(pts, contour_hull);
//			//std::vector<std::vector<cv::Point>> hulls;
//			//hulls.push_back(contour_hull);
//			//cv::drawContours(markers, hulls, 0, cv::Scalar(255),-1);
//			//cv::fillConvexPoly(markers, pts, cv::Scalar(255));
//
//			//cv::Mat backtorgb = cv::Mat(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT, CV_8UC3);
//			//cv::cvtColor(gray, backtorgb, cv::COLOR_GRAY2RGB);
//			//cv::grabCut(backtorgb, markers, cv::Rect(cv::Point2f(lowestX, lowestY), cv::Point2f(highestX, highestY)), bgdModel, fgdModel, 1, cv::GC_INIT_WITH_MASK);
//			//cv::Mat1b mask_fgpf = (markers == cv::GC_FGD) | (markers == cv::GC_PR_FGD);
//			//cv::Mat3b tmp = cv::Mat3b::zeros(imageMat.rows, imageMat.cols);
//			//imageMat.copyTo(tmp, mask_fgpf);
//			/*cv::Mat result;
//			cv::bitwise_and(gray, markers, result);*/
//
//			cv::namedWindow("Detected_shape");
//			//cv::resize(gray, gray, cv::Size(width, height), 0, 0, cv::INTER_LINEAR_EXACT);
//			cv::imshow("Detected_shape", imageMat);
//			cv::waitKey(0);
//		}
//#else
//		facemark->fit(imageMat, faces, shapes);
//#endif
//	}
	return shapes;
}