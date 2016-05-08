/*
*  Disparity.cpp
*  Description: Computes a disparity image based on the calibrated, rectified images of the stereo camera
*  Created on: Mai 8, 2016
*  Author: Alexander Treib
*/

#include "Disparity.h"


Disparity::Disparity()
{
}


Disparity::~Disparity()
{
}


bool Disparity::create_all_trackbars(std::string windowname)
{
	createTrackbar("m_SADWindowSize", windowname, &m_SADWindowSize, m_trackbar_samples);
	createTrackbar("m_numberOfDisparities", windowname, &m_numberOfDisparities, m_trackbar_samples);
	createTrackbar("m_preFilterCap", windowname, &m_preFilterCap, m_trackbar_samples);
	createTrackbar("m_minDisparity", windowname, &m_minDisparity, m_trackbar_samples);
	createTrackbar("m_uniquenessRatio", windowname, &m_uniquenessRatio, m_trackbar_samples);
	createTrackbar("m_speckleWindowSize", windowname, &m_speckleWindowSize, m_trackbar_samples);
	createTrackbar("m_speckleRange", windowname, &m_speckleRange, m_trackbar_samples);
	createTrackbar("m_disp12MaxDiff", windowname, &m_disp12MaxDiff, m_trackbar_samples);
	createTrackbar("m_P1", windowname, &m_P1, m_trackbar_samples);
	createTrackbar("m_P2", windowname, &m_P2, m_trackbar_samples);
	return 1;
}

StereoSGBM Disparity::sgbm_settings()
{

	//!Use filestorage
	StereoSGBM sgbm;

	sgbm.SADWindowSize = m_SADWindowSize;  //range 0 to 100
	sgbm.numberOfDisparities = m_numberOfDisparities;// 20 * 16 * m_numberOfDisparities / m_trackbar_samples; //range 10 to 330; divisible by 16
	sgbm.preFilterCap = m_preFilterCap;
	sgbm.minDisparity = m_minDisparity; //200 * m_minDisparity / m_trackbar_samples - 200;
	sgbm.uniquenessRatio = m_uniquenessRatio;
	sgbm.speckleWindowSize = m_speckleWindowSize;
	sgbm.speckleRange = m_speckleRange;
	sgbm.disp12MaxDiff = m_disp12MaxDiff;
	sgbm.fullDP = m_fullDP;
	sgbm.P1 = m_P1;
	sgbm.P2 = m_P2;

	return sgbm;
}

Mat Disparity::go(Mat &imgLeft, Mat &imgRight, Mat &Q)
{
	Mat disp, disp_n;
	//readin the settings from sgbm_settings
	StereoSGBM sgbm = this->sgbm_settings();

	//Calculate the disparity and normalize it (due to CV_16)
	sgbm(imgLeft, imgRight, disp);
	normalize(disp, disp_n, 0, 255, CV_MINMAX, CV_8U);
	return disp_n;
}
