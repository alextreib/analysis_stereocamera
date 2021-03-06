/*
*  Reprojection3D.cpp
*  Description: Reproject a disparity image to 3D space (input: dispimage, Q-Matrix)
*  Created on: Mai 8, 2016
*  Author: Alexander Treib
*/
//http://www.pcl-users.org/reprojectImageTo3D-for-Point-Cloud-from-stereo-Images-td4037071.html
#include "Reprojection3D.h"

Reprojection3D::Reprojection3D()
{
	m_calibration_filename = "calibration/calibration.yml";
	m_rectification_filename = "calibration/rectification.yml";
	m_disparity_filename = "results/disparityoutput.jpg";
	m_point_cloud_filename = "results/npointcloud.ply";
	m_SADWindowSize = 15;
	m_numberOfDisparities = 96;
	m_alg = STEREO_SGBM;
}

Reprojection3D::~Reprojection3D()
{
}


bool Reprojection3D::readin()
{
	//reading calibration parameters
	FileStorage fs_calib(m_calibration_filename, CV_STORAGE_READ);
	if (!fs_calib.isOpened())
	{
		printf("Failed to open file %s\n", m_calibration_filename);
		return 0;
	}
	fs_calib["M1"] >> m_M1;
	fs_calib["D1"] >> m_D1;
	fs_calib["M2"] >> m_M2;
	fs_calib["D2"] >> m_D2;
	m_M1 *= 1.5;
	m_M2 *= 1.5;
	fs_calib.release();

	//reading rectification parameters
	FileStorage fs_rect(m_rectification_filename, CV_STORAGE_READ);
	if (!fs_rect.isOpened())
	{
		printf("Failed to open file %s\n", m_rectification_filename);
		return 0;
	}
	fs_rect["R1"] >> m_R1;
	fs_rect["R2"] >> m_R2;
	fs_rect["P1"] >> m_P1;
	fs_rect["P2"] >> m_P2;
	fs_rect["Q"] >> m_Q;
	fs_rect.release();

	return true;
}

void Reprojection3D::save_pointcloud(const Mat& mat)
{
	//opening file
	const double max_z = 1.0e4;
	FILE* pointcloudfile = fopen("results/pointcloud_buf.ply", "wt");

	//Collecting the point cloud
	std::string filebuffer;
	int vertex = 0;	//counter for valid points
	for (int y = 0; y < mat.rows; y++)
	{
		for (int x = 0; x < mat.cols; x++)
		{
			Vec3f point = mat.at<Vec3f>(y, x);
			if (fabs(point[2] - max_z) < FLT_EPSILON || fabs(point[2]) > max_z) continue;
			char tmp[100] = "";
			sprintf(tmp, "%f %f %f\n", point[0], point[1], point[2]);
			filebuffer.append(tmp);
			vertex++;
		}
	}

	//writing the ply file
	std::string header = "ply\nformat ascii 1.0\nelement vertex " + std::to_string(vertex) + "\nproperty float32 x\nproperty float32 y\nproperty float32 z\nend_header\n";

	//writing everything to
	fprintf(pointcloudfile, header.c_str());
	fprintf(pointcloudfile, filebuffer.c_str());

	fclose(pointcloudfile);

	//copying -> always have the latest result
	std::ifstream  src("results/pointcloud_buf.ply", std::ios::binary);
	std::ofstream  dst("results/pointcloud.ply", std::ios::binary);
	dst << src.rdbuf();
}


bool Reprojection3D::go(Mat &img1, Mat &img2, Mat &depthimage)
{
	//input is imgLeft, imgRight and the depthimage
	bool show_images = false;
	//float scale = 1.f;

	//if (scale != 1.f)
	//{
	//	Mat temp1, temp2;
	//	int method = scale < 1 ? INTER_AREA : INTER_CUBIC;
	//	resize(img1, temp1, Size(), scale, scale, method);
	//	img1 = temp1;
	//	resize(img2, temp2, Size(), scale, scale, method);
	//	img2 = temp2;
	//}

	this->readin();


	while (((cvWaitKey(100) & 0xff) != 27) && (show_images == 1))
	{
		namedWindow("left", 1);
		imshow("left", img1);
		namedWindow("right", 1);
		imshow("right", img2);
		namedWindow("disparity", 0);
		imshow("disparity", depthimage);
	}

	//save the corresponding depthimage
	imwrite(this->m_disparity_filename, depthimage);

	if (this->m_point_cloud_filename)
	{
		Mat pointcloud, depthimage_copy;
		depthimage.copyTo(depthimage_copy);
		reprojectImageTo3D(depthimage, pointcloud, this->m_Q);
		this->save_pointcloud(pointcloud);
	}
	return 1;
}

