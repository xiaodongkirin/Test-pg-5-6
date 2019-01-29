#include "CameraCalibrator.h"

int CameraCalibrator::addChessboardPoints(
         const std::vector<std::string>& filelist, 
         cv::Size & boardSize) {

    std::vector<cv::Point2f> imageCorners;
    std::vector<cv::Point3f> objectCorners;

	for (int i=0; i<boardSize.height; i++) {
		for (int j=0; j<boardSize.width; j++) {

			objectCorners.push_back(cv::Point3f(i, j, 0.0f));
		}
    }

    cv::Mat image; 
    int successes = 0;
    for (int i=0; i<filelist.size(); i++) {

        image = cv::imread(filelist[i],0);

		bool found = cv::findCirclesGrid(image, boardSize, imageCorners);

        //bool found = cv::findChessboardCorners(
        //               image, boardSize, imageCorners);

        // Get subpixel accuracy on the corners
        cv::cornerSubPix(image, imageCorners, 
                  cv::Size(5,5), 
                  cv::Size(-1,-1), 
			cv::TermCriteria(cv::TermCriteria::MAX_ITER +
                          cv::TermCriteria::EPS, 
             30,		// max number of iterations 
             0.1));     // min accuracy

          // If we have a good board, add it to our data
		  if (imageCorners.size() == boardSize.area()) {

			// Add image and scene points from one view
            addPoints(imageCorners, objectCorners);
            successes++;
          }

        cv::drawChessboardCorners(image, boardSize, imageCorners, found);
        cv::imshow("Corners on Chessboard", image);
        cv::waitKey(100);
    }

	return successes;
}

void CameraCalibrator::addPoints(const std::vector<cv::Point2f>& imageCorners, const std::vector<cv::Point3f>& objectCorners) {

	imagePoints.push_back(imageCorners);          
	objectPoints.push_back(objectCorners);
}

double CameraCalibrator::calibrate(cv::Size &imageSize)
{
	// undistorter must be reinitialized
	mustInitUndistort= true;

	//Output rotations and translations
    std::vector<cv::Mat> rvecs, tvecs;

	// start calibration
	return 
     calibrateCamera(objectPoints, // the 3D points
		            imagePoints,  // the image points
					imageSize,    // image size
					cameraMatrix, // output camera matrix
					distCoeffs,   // output distortion matrix
					rvecs, tvecs, // Rs, Ts 
					flag);        // set options
//					,CV_CALIB_USE_INTRINSIC_GUESS);

}

// remove distortion in an image (after calibration)
cv::Mat CameraCalibrator::remap(const cv::Mat &image) {

	cv::Mat undistorted;

	if (mustInitUndistort) { // called once per calibration
    
		cv::initUndistortRectifyMap(
			cameraMatrix,  // computed camera matrix
            distCoeffs,    // computed distortion matrix
            cv::Mat(),     // optional rectification (none) 
			cv::Mat(),     // camera matrix to generate undistorted
			cv::Size(640,480),
//            image.size(),  // size of undistorted
            CV_32FC1,      // type of output map
            map1, map2);   // the x and y mapping functions

		mustInitUndistort= false;
	}

	// Apply mapping functions
    cv::remap(image, undistorted, map1, map2, 
		cv::INTER_LINEAR); // interpolation type

	return undistorted;
}

void CameraCalibrator::setCalibrationFlag(bool radial8CoeffEnabled, bool tangentialParamEnabled) {

    // Set the flag used in cv::calibrateCamera()
    flag = 0;
    if (!tangentialParamEnabled) flag += CV_CALIB_ZERO_TANGENT_DIST;
	if (radial8CoeffEnabled) flag += CV_CALIB_RATIONAL_MODEL;
}

