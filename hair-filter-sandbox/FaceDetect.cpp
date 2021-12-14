#include "FaceDetect.h"


//Intrisics can be calculated using opencv sample code under opencv/sources/samples/cpp/tutorial_code/calib3d
//Normally, you can also apprximate fx and fy by image width, cx by half image width, cy by half image height instead
double K[9] = { 6.5308391993466671e+002, 0.0, 3.1950000000000000e+002, 0.0, 6.5308391993466671e+002, 2.3950000000000000e+002, 0.0, 0.0, 1.0 };
double D[5] = { 7.0834633684407095e-002, 6.9140193737175351e-002, 0.0, 0.0, -1.3073460323689292e+000 };

FaceDetect::FaceDetect()
{
	// Load face cascade model
	m_faceCascade.load("assets/data/lbpcascade_frontalface_improved.xml");

	// Create mat for face mask
	m_mask_image_mat.create(cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT), CV_8UC1);
	m_mask_image_mat.setTo(cv::Scalar(0.0));

	// Load shape predictor
	dlib::deserialize("assets/data/shape_predictor_81_face_landmarks.dat") >> m_shapePredictor;
}

FaceDetectObj FaceDetect::getFaceObject(unsigned char* image, int width, int height)
{
	FaceDetectObj faceDetectObj;
	
	// Process image before detection
	cv::Mat imageMat = cv::Mat(height, width, CV_8UC3, image);
	cv::Mat image_gray;
	cv::Mat bgray;
	cv::resize(imageMat, imageMat, cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT), 0, 0, cv::INTER_LINEAR_EXACT);
	if (imageMat.channels() > 1) {
		cvtColor(imageMat, bgray, cv::COLOR_BGR2GRAY);
	} else {
		bgray = imageMat.clone();
	}
	cv::equalizeHist(bgray, bgray);
	cv::GaussianBlur(bgray, image_gray, cv::Size(0, 0), 3);
	cv::addWeighted(bgray, 1.5, image_gray, -0.5, 0, image_gray);

	// DEBUG::Open-CV image_gray
	//cv::namedWindow("Grayscale image");
	//cv::imshow("Grayscale image", image_gray);
	//cv::waitKey(0);

	// Detect the face we are tracking
	if (!m_foundFace)
		detectFaceAllSizes(image_gray); // Detect using cascades over whole image
	else {
		detectFaceAroundRoi(image_gray); // Detect using cascades only in ROI
		if (m_templateMatchingRunning) {
			detectFacesTemplateMatching(image_gray); // Detect using template matching
		}
	}

	// Dlib get face shape landmarks using tracked face
	dlib::rectangle tracked_face_rect = dlib::rectangle((long)m_trackedFace.tl().x, (long)m_trackedFace.tl().y, (long)m_trackedFace.br().x - 1, (long)m_trackedFace.br().y - 1);
	dlib::array2d<dlib::rgb_pixel> img_dlib;
	dlib::assign_image(img_dlib, dlib::cv_image<unsigned char>(image_gray));
	dlib::full_object_detection face_shape = m_shapePredictor(img_dlib, tracked_face_rect);

#if ENABLE_DEBUG
	std::cout << "Number of face landmarks: " << face_shape.num_parts() << std::endl;
	std::cout << "Pixel position of first landmark:  " << face_shape.part(0) << std::endl;
	std::cout << "Pixel position of second landmark: " << face_shape.part(1) << std::endl;
#endif

	// DEBUG::Open-CV tracked face rectangle
	//cv::rectangle(imageMat, m_trackedFace, cv::Scalar(0, 255, 0));
	//cv::Rect rc(cv::Point2i(face_shape.get_rect().left(), face_shape.get_rect().top()), cv::Point2i(face_shape.get_rect().right() + 1, face_shape.get_rect().bottom() + 1));
	//cv::rectangle(imageMat, rc, cv::Scalar(255, 0, 0));

	// Face Pose Variables
	cv::Mat rotation_vec;                           //3 x 1
	cv::Mat rotation_mat;                           //3 x 3 R
	cv::Mat translation_vec;                        //3 x 1 T
	cv::Mat pose_mat = cv::Mat(3, 4, CV_64FC1);     //3 x 4 R | T
	cv::Mat euler_angle = cv::Mat(3, 1, CV_64FC1);

	// Landmarks and Points variables for rendering and pose estimation
	std::vector<cv::Point> face_landmarks;
	std::vector<cv::Point> neck_landmarks;
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

	// Process all face landmarks
	for (unsigned long k = 0; k < face_shape.num_parts(); k++) {
		cv::Point p;
		p.x = face_shape.part(k).x();
		p.y = face_shape.part(k).y();
		face_landmarks.push_back(p);
		if (k == 8) {
			std::cout << "bot: " << p.x << "," << p.y << std::endl;
			cv::Point p_transformed;
			p_transformed.x = p.x;
			p_transformed.y = RESIZED_IMAGE_HEIGHT - p.y;
			faceDetectObj.botPoint = p_transformed;
		}
		if (k == 71) {
			std::cout << "top: " << p.x << "," << p.y << std::endl;
			cv::Point p_transformed;
			p_transformed.x = p.x;
			p_transformed.y = RESIZED_IMAGE_HEIGHT - p.y;
			faceDetectObj.topPoint = p_transformed;
		}
		if (k == 0) {
			std::cout << "right: " << p.x << "," << p.y << std::endl;
			cv::Point p_transformed;
			p_transformed.x = p.x;
			p_transformed.y = RESIZED_IMAGE_HEIGHT - p.y;
			faceDetectObj.rightPoint = p_transformed;
		}
		if (k == 16) {
			std::cout << "left: " << p.x << "," << p.y << std::endl;
			cv::Point p_transformed;
			p_transformed.x = p.x;
			p_transformed.y = RESIZED_IMAGE_HEIGHT - p.y;
			faceDetectObj.leftPoint = p_transformed;
		}
		if (k == 5) {
			neck_landmarks.push_back(cv::Point(p.x, p.y));
			neck_landmarks.push_back(cv::Point(p.x, RESIZED_IMAGE_HEIGHT));
		}
		if (k == 11) {
			neck_landmarks.push_back(cv::Point(p.x, p.y));
			neck_landmarks.push_back(cv::Point(p.x, RESIZED_IMAGE_HEIGHT));
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
		// DEBUG::Open-CV show all points
		//cv::circle(imageMat, p, 5, cv::Scalar(0, 0, 255), cv::FILLED);
	}

	// DEBUG::Open-CV show imageMat with rect and circles
	//cv::namedWindow("imageMat");
	//cv::imshow("imageMat", imageMat);
	//cv::waitKey(0);

	// Generate mask image
	std::vector<cv::Point> contour_hull;
	std::vector<cv::Point> contour_neck_hull;
	cv::convexHull(face_landmarks, contour_hull);
	cv::convexHull(neck_landmarks, contour_neck_hull);
	std::vector<std::vector<cv::Point>> hulls;
	hulls.push_back(contour_hull);
	hulls.push_back(contour_neck_hull);
	cv::drawContours(m_mask_image_mat, hulls, 0, cv::Scalar(255),-1);
	cv::drawContours(m_mask_image_mat, hulls, 1, cv::Scalar(255), -1);
	cv::GaussianBlur(m_mask_image_mat, m_mask_image_mat, cv::Size(3, 3), 11.0);

	m_face_mask_image = m_mask_image_mat.data;

	// DEBUG::Open-CV show mask image
	//cv::namedWindow("Mask image");
	//cv::imshow("Mask image", m_mask_image_mat);
	//cv::waitKey(0);

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
	std::vector<cv::Point3d> model_reference_landmarks;
	model_reference_landmarks.push_back(cv::Point3d(6.825897, 6.760612, 4.402142));     //#33 left brow left corner
	model_reference_landmarks.push_back(cv::Point3d(1.330353, 7.122144, 6.903745));     //#29 left brow right corner
	model_reference_landmarks.push_back(cv::Point3d(-1.330353, 7.122144, 6.903745));    //#34 right brow left corner
	model_reference_landmarks.push_back(cv::Point3d(-6.825897, 6.760612, 4.402142));    //#38 right brow right corner
	model_reference_landmarks.push_back(cv::Point3d(5.311432, 5.485328, 3.987654));     //#13 left eye left corner
	model_reference_landmarks.push_back(cv::Point3d(1.789930, 5.393625, 4.413414));     //#17 left eye right corner
	model_reference_landmarks.push_back(cv::Point3d(-1.789930, 5.393625, 4.413414));    //#25 right eye left corner
	model_reference_landmarks.push_back(cv::Point3d(-5.311432, 5.485328, 3.987654));    //#21 right eye right corner
	model_reference_landmarks.push_back(cv::Point3d(2.005628, 1.409845, 6.165652));     //#55 nose left corner
	model_reference_landmarks.push_back(cv::Point3d(-2.005628, 1.409845, 6.165652));    //#49 nose right corner
	model_reference_landmarks.push_back(cv::Point3d(2.774015, -2.080775, 5.048531));    //#43 mouth left corner
	model_reference_landmarks.push_back(cv::Point3d(-2.774015, -2.080775, 5.048531));   //#39 mouth right corner
	model_reference_landmarks.push_back(cv::Point3d(0.000000, -3.116408, 6.097667));    //#45 mouth central bottom corner
	model_reference_landmarks.push_back(cv::Point3d(0.000000, -7.415691, 4.070434));    //#6 chin corner

	// Camera internals
	cv::Mat cam_matrix = cv::Mat(3, 3, CV_64FC1, K);
	cv::Mat dist_coeffs = cv::Mat(5, 1, CV_64FC1, D);

	// Solve for pose
	cv::solvePnP(model_reference_landmarks, pose_image_points, cam_matrix, dist_coeffs, rotation_vec, translation_vec);

	//temp buf for decomposeProjectionMatrix()
	cv::Mat out_intrinsics = cv::Mat(3, 3, CV_64FC1);
	cv::Mat out_rotation = cv::Mat(3, 3, CV_64FC1);
	cv::Mat out_translation = cv::Mat(3, 1, CV_64FC1);

	//calc euler angle
	cv::Rodrigues(rotation_vec, rotation_mat);
	cv::hconcat(rotation_mat, translation_vec, pose_mat);
	cv::decomposeProjectionMatrix(pose_mat, out_intrinsics, out_rotation, out_translation, cv::noArray(), cv::noArray(), cv::noArray(), euler_angle);

	// Store face landmarks and pose
	std::cout << "Rotation Matrix " << std::endl << euler_angle << std::endl;
	faceDetectObj.pitch = euler_angle.at<double>(0);// *180.0f / 3.1415926535897932384;
	faceDetectObj.yaw = euler_angle.at<double>(1);// *180.0f / 3.1415926535897932384;
	faceDetectObj.roll = euler_angle.at<double>(2);// *180.0f / 3.1415926535897932384;
	faceDetectObj.landmarks = face_landmarks;

	return faceDetectObj;
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