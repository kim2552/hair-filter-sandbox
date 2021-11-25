#include "FaceDetect.h"


//Intrisics can be calculated using opencv sample code under opencv/sources/samples/cpp/tutorial_code/calib3d
//Normally, you can also apprximate fx and fy by image width, cx by half image width, cy by half image height instead
double K[9] = { 6.5308391993466671e+002, 0.0, 3.1950000000000000e+002, 0.0, 6.5308391993466671e+002, 2.3950000000000000e+002, 0.0, 0.0, 1.0 };
double D[5] = { 7.0834633684407095e-002, 6.9140193737175351e-002, 0.0, 0.0, -1.3073460323689292e+000 };

FaceDetect::FaceDetect()
{
	m_faceCascade.load("assets/data/lbpcascade_frontalface_improved.xml");
	//facemark = cv::face::createFacemarkKazemi();
	//facemark->loadModel("assets/data/face_landmark_model.dat");
	printf("Loaded model.");

	markers.create(cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT), CV_8UC1);
	markers.setTo(cv::Scalar(0.0));	//cv::Scalar(255)	cv::GC_BGD

	//face_detector = dlib::get_frontal_face_detector();
	dlib::deserialize("assets/data/shape_predictor_81_face_landmarks.dat") >> m_shapePredictor;
}

//TODO::We will be using YUV image which will isolate gray channel image, so we don't have to conver to gray image
std::vector<FaceDetectObj> FaceDetect::getFaceLandmarks(unsigned char* image, int width, int height)
{
	std::vector<FaceDetectObj> fdObjs;

	cv::Mat imageMat = cv::Mat(height, width, CV_8UC3, image);
	cv::resize(imageMat, imageMat, cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT), 0, 0, cv::INTER_LINEAR_EXACT);
	cv::Mat bgray;
	if (imageMat.channels() > 1) {
		cvtColor(imageMat, bgray, cv::COLOR_BGR2GRAY);
	} else {
		bgray = imageMat.clone();
	}
	cv::equalizeHist(bgray, bgray);
	cv::Mat gray;
	cv::GaussianBlur(bgray, gray, cv::Size(0, 0), 3);
	cv::addWeighted(bgray, 1.5, gray, -0.5, 0, gray);

	//cv::namedWindow("Detected_shape");
	//cv::imshow("Detected_shape", gray);
	//cv::waitKey(0);

	//std::vector<cv::Rect> faces;
	//m_faceCascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(30, 30));

	if (!m_foundFace)
		detectFaceAllSizes(gray); // Detect using cascades over whole image
	else {
		detectFaceAroundRoi(gray); // Detect using cascades only in ROI
		if (m_templateMatchingRunning) {
			detectFacesTemplateMatching(gray); // Detect using template matching
		}
	}

	for (unsigned long j = 0; j < 1; ++j)
	{
		FaceDetectObj obj;

		dlib::rectangle rect = dlib::rectangle((long)m_trackedFace.tl().x, (long)m_trackedFace.tl().y, (long)m_trackedFace.br().x - 1, (long)m_trackedFace.br().y - 1);
		dlib::array2d<dlib::rgb_pixel> img_dlib;
		dlib::assign_image(img_dlib, dlib::cv_image<unsigned char>(gray));
		dlib::full_object_detection shape = m_shapePredictor(img_dlib, rect);

#if ENABLE_DEBUG
		std::cout << "number of parts: " << shape.num_parts() << std::endl;
		std::cout << "pixel position of first part:  " << shape.part(0) << std::endl;
		std::cout << "pixel position of second part: " << shape.part(1) << std::endl;
#endif
		for (size_t i = 0; i < 1; i++)
		{
			cv::rectangle(imageMat, m_trackedFace, cv::Scalar(0, 255, 0));
		}
		//for (size_t i = 0; i < faces.size(); i++)
		//{
		//	cv::Rect rc(cv::Point2i(shape.get_rect().left(), shape.get_rect().top()), cv::Point2i(shape.get_rect().right() + 1, shape.get_rect().bottom() + 1));
		//	cv::rectangle(imageMat, rc, cv::Scalar(255, 0, 0));
		//}

		//result
		cv::Mat rotation_vec;                           //3 x 1
		cv::Mat rotation_mat;                           //3 x 3 R
		cv::Mat translation_vec;                        //3 x 1 T
		cv::Mat pose_mat = cv::Mat(3, 4, CV_64FC1);     //3 x 4 R | T
		cv::Mat euler_angle = cv::Mat(3, 1, CV_64FC1);

		std::vector<cv::Point> pts;
		std::vector<cv::Point> neck_pts;
		std::vector<cv::Point2d> pose_image_points;
		cv::Point2d left_brow_left_corner;
		cv::Point2d left_brow_right_corner;
		cv::Point2d right_brow_left_corner;
		cv::Point2d right_brow_right_corner;
		cv::Point2d left_eye_left_corner;
		cv::Point2d left_eye_right_corner;
		cv::Point2d right_eye_left_corner;
		cv::Point2d right_eye_right_corner;
		cv::Point2d nose_left_corner;
		cv::Point2d nose_right_corner;
		cv::Point2d mouth_left_corner;
		cv::Point2d mouth_right_corner;
		cv::Point2d mouth_central_bottom_corner;
		cv::Point2d chin_corner;

		for (unsigned long k = 0; k < shape.num_parts(); k++) {
			cv::Point p;
			p.x = shape.part(k).x();
			p.y = shape.part(k).y();
			pts.push_back(p);
			if (k == 8) {
				std::cout << "bot: " << p.x << "," << p.y << std::endl;
				cv::Point p_transformed;
				p_transformed.x = p.x;
				p_transformed.y = RESIZED_IMAGE_HEIGHT - p.y;
				obj.botHeadPoint = p_transformed;
			}
			if (k == 71) {
				std::cout << "top: " << p.x << "," << p.y << std::endl;
				cv::Point p_transformed;
				p_transformed.x = p.x;
				p_transformed.y = RESIZED_IMAGE_HEIGHT - p.y;
				obj.topHeadPoint = p_transformed;
			}
			if (k == 0) {
				std::cout << "right: " << p.x << "," << p.y << std::endl;
				cv::Point p_transformed;
				p_transformed.x = p.x;
				p_transformed.y = RESIZED_IMAGE_HEIGHT - p.y;
				obj.rightHeadPoint = p_transformed;
			}
			if (k == 16) {
				std::cout << "left: " << p.x << "," << p.y << std::endl;
				cv::Point p_transformed;
				p_transformed.x = p.x;
				p_transformed.y = RESIZED_IMAGE_HEIGHT - p.y;
				obj.leftHeadPoint = p_transformed;
			}
			if (k == 5) {
				neck_pts.push_back(cv::Point(p.x, p.y));
				neck_pts.push_back(cv::Point(p.x, RESIZED_IMAGE_HEIGHT));
			}
			if (k == 11) {
				neck_pts.push_back(cv::Point(p.x, p.y));
				neck_pts.push_back(cv::Point(p.x, RESIZED_IMAGE_HEIGHT));
			}
			if (k == 57) {
				mouth_central_bottom_corner = p;
			}
			if (k == 54) {
				mouth_right_corner = p;
			}
			if (k == 48) {
				mouth_left_corner = p;
			}
			if (k == 45) {
				right_eye_right_corner = p;
			}
			if (k == 42) {
				right_eye_left_corner = p;
			}
			if (k == 39) {
				left_eye_right_corner = p;
			}
			if (k == 36) {
				left_eye_left_corner = p;
			}
			if (k == 35) {
				nose_right_corner = p;
			}
			if (k == 31) {
				nose_left_corner = p;
			}
			if (k == 26) {
				right_brow_right_corner = p;
			}
			if (k == 22) {
				right_brow_left_corner = p;
			}
			if (k == 21) {
				left_brow_right_corner = p;
			}
			if (k == 17) {
				left_brow_left_corner = p;
			}
			if (k == 8) {
				chin_corner = p;
			}
			cv::circle(imageMat, p, 5, cv::Scalar(0, 0, 255), cv::FILLED);
		}
		std::vector<cv::Point> contour_hull;
		std::vector<cv::Point> contour_neck_hull;
		cv::convexHull(pts, contour_hull);
		cv::convexHull(neck_pts, contour_neck_hull);
		std::vector<std::vector<cv::Point>> hulls;
		hulls.push_back(contour_hull);
		hulls.push_back(contour_neck_hull);
		cv::drawContours(markers, hulls, 0, cv::Scalar(255),-1);
		cv::drawContours(markers, hulls, 1, cv::Scalar(255), -1);

		cv::GaussianBlur(markers, markers, cv::Size(3, 3), 11.0);

		//cv::namedWindow("Detected_shape");
		//cv::imshow("Detected_shape", markers);
		//cv::waitKey(0);

		face_mask_image = markers.data;

		// Pose image points
		pose_image_points.push_back(left_brow_left_corner);
		pose_image_points.push_back(left_brow_right_corner);
		pose_image_points.push_back(right_brow_left_corner);
		pose_image_points.push_back(right_brow_right_corner);
		pose_image_points.push_back(left_eye_left_corner);
		pose_image_points.push_back(left_eye_right_corner);
		pose_image_points.push_back(right_eye_left_corner);
		pose_image_points.push_back(right_eye_right_corner);
		pose_image_points.push_back(nose_left_corner);
		pose_image_points.push_back(nose_right_corner);
		pose_image_points.push_back(mouth_left_corner);
		pose_image_points.push_back(mouth_right_corner);
		pose_image_points.push_back(mouth_central_bottom_corner);
		pose_image_points.push_back(chin_corner);

		//fill in 3D ref points(world coordinates), model referenced from http://aifi.isr.uc.pt/Downloads/OpenGL/glAnthropometric3DModel.cpp
		std::vector<cv::Point3d> object_pts;
		object_pts.push_back(cv::Point3d(6.825897, 6.760612, 4.402142));     //#33 left brow left corner
		object_pts.push_back(cv::Point3d(1.330353, 7.122144, 6.903745));     //#29 left brow right corner
		object_pts.push_back(cv::Point3d(-1.330353, 7.122144, 6.903745));    //#34 right brow left corner
		object_pts.push_back(cv::Point3d(-6.825897, 6.760612, 4.402142));    //#38 right brow right corner
		object_pts.push_back(cv::Point3d(5.311432, 5.485328, 3.987654));     //#13 left eye left corner
		object_pts.push_back(cv::Point3d(1.789930, 5.393625, 4.413414));     //#17 left eye right corner
		object_pts.push_back(cv::Point3d(-1.789930, 5.393625, 4.413414));    //#25 right eye left corner
		object_pts.push_back(cv::Point3d(-5.311432, 5.485328, 3.987654));    //#21 right eye right corner
		object_pts.push_back(cv::Point3d(2.005628, 1.409845, 6.165652));     //#55 nose left corner
		object_pts.push_back(cv::Point3d(-2.005628, 1.409845, 6.165652));    //#49 nose right corner
		object_pts.push_back(cv::Point3d(2.774015, -2.080775, 5.048531));    //#43 mouth left corner
		object_pts.push_back(cv::Point3d(-2.774015, -2.080775, 5.048531));   //#39 mouth right corner
		object_pts.push_back(cv::Point3d(0.000000, -3.116408, 6.097667));    //#45 mouth central bottom corner
		object_pts.push_back(cv::Point3d(0.000000, -7.415691, 4.070434));    //#6 chin corner

		// Camera internals
		cv::Mat cam_matrix = cv::Mat(3, 3, CV_64FC1, K);
		cv::Mat dist_coeffs = cv::Mat(5, 1, CV_64FC1, D);

		// Solve for pose
		cv::solvePnP(object_pts, pose_image_points, cam_matrix, dist_coeffs, rotation_vec, translation_vec);

		//temp buf for decomposeProjectionMatrix()
		cv::Mat out_intrinsics = cv::Mat(3, 3, CV_64FC1);
		cv::Mat out_rotation = cv::Mat(3, 3, CV_64FC1);
		cv::Mat out_translation = cv::Mat(3, 1, CV_64FC1);

		//calc euler angle
		cv::Rodrigues(rotation_vec, rotation_mat);
		cv::hconcat(rotation_mat, translation_vec, pose_mat);
		cv::decomposeProjectionMatrix(pose_mat, out_intrinsics, out_rotation, out_translation, cv::noArray(), cv::noArray(), cv::noArray(), euler_angle);


		std::cout << "Rotation Matrix " << std::endl << euler_angle << std::endl;
		obj.pitch = euler_angle.at<double>(0);// *180.0f / 3.1415926535897932384;
		obj.yaw = euler_angle.at<double>(1);// *180.0f / 3.1415926535897932384;
		obj.roll = euler_angle.at<double>(2);// *180.0f / 3.1415926535897932384;

		obj.shape = pts;
		fdObjs.push_back(obj);
	}
	return fdObjs;
}

void FaceDetect::detectFacesTemplateMatching(const cv::Mat& frame)
{
	// Calculate duration of template matching
	m_templateMatchingCurrentTime = cv::getTickCount();
	double duration = (double)(m_templateMatchingCurrentTime - m_templateMatchingStartTime) / cv::getTickFrequency();

	// If template matching lasts for more than 2 seconds face is possibly lost
	// so disable it and redetect using cascades
	if (duration > m_templateMatchingMaxDuration) {
		m_foundFace = false;
		m_templateMatchingRunning = false;
		m_templateMatchingStartTime = m_templateMatchingCurrentTime = 0;
		m_facePosition.x = m_facePosition.y = 0;
		m_trackedFace.x = m_trackedFace.y = m_trackedFace.width = m_trackedFace.height = 0;
		return;
	}

	// Edge case when face exits frame while 
	if (m_faceTemplate.rows * m_faceTemplate.cols == 0 || m_faceTemplate.rows <= 1 || m_faceTemplate.cols <= 1) {
		m_foundFace = false;
		m_templateMatchingRunning = false;
		m_templateMatchingStartTime = m_templateMatchingCurrentTime = 0;
		m_facePosition.x = m_facePosition.y = 0;
		m_trackedFace.x = m_trackedFace.y = m_trackedFace.width = m_trackedFace.height = 0;
		return;
	}

	// Template matching with last known face 
	//cv::matchTemplate(frame(m_faceRoi), m_faceTemplate, m_matchingResult, CV_TM_CCOEFF);
	cv::matchTemplate(frame(m_faceRoi), m_faceTemplate, m_matchingResult, cv::TM_SQDIFF_NORMED);
	cv::normalize(m_matchingResult, m_matchingResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
	double min, max;
	cv::Point minLoc, maxLoc;
	cv::minMaxLoc(m_matchingResult, &min, &max, &minLoc, &maxLoc);

	// Add roi offset to face position
	minLoc.x += m_faceRoi.x;
	minLoc.y += m_faceRoi.y;

	// Get detected face
	//m_trackedFace = cv::Rect(maxLoc.x, maxLoc.y, m_trackedFace.width, m_trackedFace.height);
	m_trackedFace = cv::Rect(minLoc.x, minLoc.y, m_faceTemplate.cols, m_faceTemplate.rows);
	m_trackedFace = doubleRectSize(m_trackedFace, cv::Rect(0, 0, frame.cols, frame.rows));

	// Get new face template
	m_faceTemplate = getFaceTemplate(frame, m_trackedFace);

	// Calculate face roi
	m_faceRoi = doubleRectSize(m_trackedFace, cv::Rect(0, 0, frame.cols, frame.rows));

	// Update face position
	m_facePosition = centerOfRect(m_trackedFace);
}

void FaceDetect::detectFaceAroundRoi(const cv::Mat& frame)
{
	// Detect faces sized +/-20% off biggest face in previous search
	m_faceCascade.detectMultiScale(frame(m_faceRoi), m_allFaces, 1.1, 3, 0,
		cv::Size(m_trackedFace.width * 8 / 10, m_trackedFace.height * 8 / 10),
		cv::Size(m_trackedFace.width * 12 / 10, m_trackedFace.width * 12 / 10));

	if (m_allFaces.empty())
	{
		// Activate template matching if not already started and start timer
		m_templateMatchingRunning = true;
		if (m_templateMatchingStartTime == 0)
			m_templateMatchingStartTime = cv::getTickCount();
		return;
	}

	// Turn off template matching if running and reset timer
	m_templateMatchingRunning = false;
	m_templateMatchingCurrentTime = m_templateMatchingStartTime = 0;

	// Get detected face
	m_trackedFace = biggestFace(m_allFaces);

	// Add roi offset to face
	m_trackedFace.x += m_faceRoi.x;
	m_trackedFace.y += m_faceRoi.y;

	// Get face template
	m_faceTemplate = getFaceTemplate(frame, m_trackedFace);

	// Calculate roi
	m_faceRoi = doubleRectSize(m_trackedFace, cv::Rect(0, 0, frame.cols, frame.rows));

	// Update face position
	m_facePosition = centerOfRect(m_trackedFace);
}

void FaceDetect::detectFaceAllSizes(const cv::Mat& frame)
{
	// Minimum face size is 1/5th of screen height
	// Maximum face size is 2/3rds of screen height
	m_faceCascade.detectMultiScale(frame, m_allFaces, 1.1, 3, 0,
		cv::Size(frame.rows / 5, frame.rows / 5),
		cv::Size(frame.rows * 2 / 3, frame.rows * 2 / 3));

	if (m_allFaces.empty()) return;

	m_foundFace = true;

	// Locate biggest face
	m_trackedFace = biggestFace(m_allFaces);

	// Copy face template
	m_faceTemplate = getFaceTemplate(frame, m_trackedFace);

	// Calculate roi
	m_faceRoi = doubleRectSize(m_trackedFace, cv::Rect(0, 0, frame.cols, frame.rows));

	// Update face position
	m_facePosition = centerOfRect(m_trackedFace);
}

/*
* Face template is small patch in the middle of detected face.
*/
cv::Mat FaceDetect::getFaceTemplate(const cv::Mat& frame, cv::Rect face)
{
	face.x += face.width / 4;
	face.y += face.height / 4;
	face.width /= 2;
	face.height /= 2;

	cv::Mat faceTemplate = frame(face).clone();
	return faceTemplate;
}

cv::Rect FaceDetect::biggestFace(std::vector<cv::Rect>& faces) const
{
	assert(!faces.empty());

	cv::Rect* biggest = &faces[0];
	for (auto& face : faces) {
		if (face.area() < biggest->area())
			biggest = &face;
	}
	return *biggest;
}

cv::Point FaceDetect::centerOfRect(const cv::Rect& rect) const
{
	return cv::Point(rect.x + rect.width / 2, rect.y + rect.height / 2);
}

cv::Rect FaceDetect::doubleRectSize(const cv::Rect& inputRect, const cv::Rect& frameSize) const
{
	cv::Rect outputRect;
	// Double rect size
	outputRect.width = inputRect.width * 2;
	outputRect.height = inputRect.height * 2;

	// Center rect around original center
	outputRect.x = inputRect.x - inputRect.width / 2;
	outputRect.y = inputRect.y - inputRect.height / 2;

	// Handle edge cases
	if (outputRect.x < frameSize.x) {
		outputRect.width += outputRect.x;
		outputRect.x = frameSize.x;
	}
	if (outputRect.y < frameSize.y) {
		outputRect.height += outputRect.y;
		outputRect.y = frameSize.y;
	}

	if (outputRect.x + outputRect.width > frameSize.width) {
		outputRect.width = frameSize.width - outputRect.x;
	}
	if (outputRect.y + outputRect.height > frameSize.height) {
		outputRect.height = frameSize.height - outputRect.y;
	}

	return outputRect;
}