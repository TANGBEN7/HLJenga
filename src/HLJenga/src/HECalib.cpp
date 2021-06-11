/***********************************************************************
˵����Opencvʵ�����۱궨�����۲���
***********************************************************************/

#include "HECalib.h"

#include <iostream>
#include <fstream>
#include <string>

#include <Eigen/Core>
#include <Eigen/Dense>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/imgproc/types_c.h>

using namespace Eigen;
using namespace cv;
using namespace std;

#define pi 3.1415926
#define board_width 11
#define board_height 8
#define board_square 7

//R��TתRT����
//������
void RR_2R(Mat &RR, Mat &TT, Mat &R, Mat &T, int i)
{
	cv::Rect T_rect(0, i, 1, 3);
	cv::Rect R_rect(0, i, 3, 3);
	R = RR(R_rect);
	T = TT(T_rect);
}

Mat R_T2RT(Mat &R, Mat &T)
{
	Mat RT;
	Mat_<double> R1 = (cv::Mat_<double>(4, 3) << R.at<double>(0, 0), R.at<double>(0, 1), R.at<double>(0, 2),
					   R.at<double>(1, 0), R.at<double>(1, 1), R.at<double>(1, 2),
					   R.at<double>(2, 0), R.at<double>(2, 1), R.at<double>(2, 2),
					   0.0, 0.0, 0.0);
	cv::Mat_<double> T1 = (cv::Mat_<double>(4, 1) << T.at<double>(0, 0), T.at<double>(1, 0), T.at<double>(2, 0), 1.0);

	cv::hconcat(R1, T1, RT); //C=A+B����ƴ��
	return RT;
}

//RTתR��T����
void RT2R_T(Mat &RT, Mat &R, Mat &T)
{
	cv::Rect R_rect(0, 0, 3, 3);
	cv::Rect T_rect(3, 0, 1, 3);
	R = RT(R_rect);
	T = RT(T_rect);
}

//�ж��Ƿ�Ϊ��ת����
bool isRotationMatrix(const cv::Mat &R)
{
	cv::Mat tmp33 = R({0, 0, 3, 3});
	cv::Mat shouldBeIdentity;

	shouldBeIdentity = tmp33.t() * tmp33;

	cv::Mat I = cv::Mat::eye(3, 3, shouldBeIdentity.type());

	return cv::norm(I, shouldBeIdentity) < 1e-6;
}

cv::Mat eulerAngleToRotatedMatrix(const cv::Mat &eulerAngle, const std::string &seq)
{
	CV_Assert(eulerAngle.rows == 1 && eulerAngle.cols == 3);

	eulerAngle /= 180 / CV_PI;
	cv::Matx13d m(eulerAngle);
	auto rx = m(0, 0), ry = m(0, 1), rz = m(0, 2);
	auto xs = std::sin(rx), xc = std::cos(rx);
	auto ys = std::sin(ry), yc = std::cos(ry);
	auto zs = std::sin(rz), zc = std::cos(rz);

	cv::Mat rotX = (cv::Mat_<double>(3, 3) << 1, 0, 0, 0, xc, -xs, 0, xs, xc);
	cv::Mat rotY = (cv::Mat_<double>(3, 3) << yc, 0, ys, 0, 1, 0, -ys, 0, yc);
	cv::Mat rotZ = (cv::Mat_<double>(3, 3) << zc, -zs, 0, zs, zc, 0, 0, 0, 1);
	cv::Mat rotZ1 = (cv::Mat_<double>(3, 3) << xc, -xs, 0, xs, xc, 0, 0, 0, 1);
	cv::Mat rotMat;

	if (seq == "zyx")
		rotMat = rotX * rotY * rotZ;
	else if (seq == "yzx")
		rotMat = rotX * rotZ * rotY;
	else if (seq == "zxy")
		rotMat = rotY * rotX * rotZ;
	else if (seq == "xzy")
		rotMat = rotY * rotZ * rotX;
	else if (seq == "yxz")
		rotMat = rotZ * rotX * rotY;
	else if (seq == "xyz")
		rotMat = rotZ * rotY * rotX;
	else if (seq == "zyz")
		rotMat = rotZ1 * rotY * rotZ;
	else
	{
		cv::error(cv::Error::StsAssert, "Euler angle sequence string is wrong.",
				  __FUNCTION__, __FILE__, __LINE__);
	}

	if (!isRotationMatrix(rotMat))
	{
		cv::error(cv::Error::StsAssert, "Euler angle can not convert to rotated matrix",
				  __FUNCTION__, __FILE__, __LINE__);
	}

	return rotMat;
	//cout << isRotationMatrix(rotMat) << endl;
}

/** @brief ��Ԫ��ת��ת����
*	@note  ��������double�� ��Ԫ������ q = w + x*i + y*j + z*k
*	@param q ��Ԫ������{w,x,y,z}����
*	@return ������ת����3*3
*/

cv::Mat quaternionToRotatedMatrix(const cv::Vec4d &q)
{
	double w = q[0], x = q[1], y = q[2], z = q[3];

	double x2 = x * x, y2 = y * y, z2 = z * z;
	double xy = x * y, xz = x * z, yz = y * z;
	double wx = w * x, wy = w * y, wz = w * z;

	cv::Matx33d res{
		1 - 2 * (y2 + z2),
		2 * (xy - wz),
		2 * (xz + wy),
		2 * (xy + wz),
		1 - 2 * (x2 + z2),
		2 * (yz - wx),
		2 * (xz - wy),
		2 * (yz + wx),
		1 - 2 * (x2 + y2),
	};
	return cv::Mat(res);
}

/** @brief ((��Ԫ��||ŷ����||��ת����) && ת������) -> 4*4 ��Rt
*	@param 	m				1*6 || 1*10�ľ���  -> 6  {x,y,z, rx,ry,rz}   10 {x,y,z, qw,qx,qy,qz, rx,ry,rz}
*	@param 	useQuaternion	�����1*10�ľ����ж��Ƿ�ʹ����Ԫ��������ת����
*	@param 	seq				���ͨ��ŷ���Ǽ�����ת������Ҫָ��ŷ����xyz������˳���磺"xyz" "zyx" Ϊ�ձ�ʾ��ת����
*/

cv::Mat attitudeVectorToMatrix(const cv::Mat &m, bool useQuaternion, const std::string &seq)
{
	CV_Assert(m.total() == 6 || m.total() == 10);
	/*if (m.cols == 1)
		m = m.t();*/
	cv::Mat tmp = cv::Mat::eye(4, 4, CV_64FC1);

	//���ʹ����Ԫ��ת������ת�������ȡm����ĵڵ��ĸ���Ա����4������
	if (useQuaternion) // normalized vector, its norm should be 1.
	{
		cv::Vec4d quaternionVec = m({3, 0, 4, 1});
		quaternionToRotatedMatrix(quaternionVec).copyTo(tmp({0, 0, 3, 3}));
		// cout << norm(quaternionVec) << endl;
	}
	else
	{
		cv::Mat rotVec;
		if (m.total() == 6)
			rotVec = m({3, 0, 3, 1}); //6
		else
			rotVec = m({7, 0, 3, 1}); //10

		//���seqΪ�ձ�ʾ���������ת����������"xyz"����ϱ�ʾŷ����
		if (0 == seq.compare(""))
			cv::Rodrigues(rotVec, tmp({0, 0, 3, 3}));
		else
			eulerAngleToRotatedMatrix(rotVec, seq).copyTo(tmp({0, 0, 3, 3}));
	}
	tmp({3, 0, 1, 3}) = m({0, 0, 3, 1}).t() / 1000.0f;

	//std::swap(m,tmp);
	return tmp;
}

void eye_in_hand(String DATAPATH)
{
	//�������۱궨����
	std::vector<Mat> R_gripper2base;
	std::vector<Mat> t_gripper2base;
	std::vector<Mat> R_target2cam;
	std::vector<Mat> t_target2cam;
	std::vector<Point3f> t_target2base;
	Mat R_cam2gripper = (Mat_<double>(3, 3));
	Mat t_cam2gripper = (Mat_<double>(3, 1));
	Eigen::Matrix3d rotation_matrix;
	vector<Mat> images;
	cv::Mat Hcg; //�������camera��ĩ��grab��λ�˾���
	std::vector<cv::Mat> vecHg, vecHc;
	Mat tempR, tempT;

	ifstream filein(DATAPATH + "calibration.txt"); //��ȡͼƬ��
	if (!filein)
	{
		cout << "δ�ҵ��ļ�������" << endl;
		return;
	}
	ofstream fout(DATAPATH + "Result.txt");
	cout << "�ǵ��ȡ" << endl;
	int imagcount = 0;								   //ͼƬ����
	Size image_size;								   //ͼƬ�ߴ�
	Size board_size = Size(board_width, board_height); //�궨��ǵ���
	vector<Point2f> Corners;						   //����ÿ��ͼ�Ľǵ���Ϣ
	vector<vector<Point2f>> AllCorners;				   //����ͼƬ�Ľǵ���Ϣ
	string imagename;
	Mat grayImag; //�Ҷ�ͼ
	Mat imagin;
	String path;
	while (getline(filein, imagename))
	{
		imagcount++;
		cout << "imagecout=" << imagcount << endl;
		imagin = imread(DATAPATH + imagename); //����ͼƬ
		if (imagcount == 1)
		{
			image_size.height = imagin.rows; //ͼ��ĸ߶�Ӧ������
			image_size.width = imagin.cols;	 //ͼ��Ŀ��Ӧ������
			cout << "image_size.width = " << image_size.width << endl;
			cout << "image_size.height = " << image_size.height << endl;
		}
		cout << "begin  find corners! " << endl;
		bool findcorners = findChessboardCorners(imagin, board_size, Corners); //Ѱ�ҽǵ�

		cout << "find corners! " << endl;
		if (!findcorners)
		{
			cout << "can not find corners!!";
			return;
		}
		cvtColor(imagin, grayImag, CV_RGB2GRAY);					//��ͼƬתΪ�Ҷ�ͼ
		find4QuadCornerSubpix(grayImag, Corners, Size(5, 5));		//�����ؾ�ȷ��
		AllCorners.push_back(Corners);								//�洢�ǵ���Ϣ
		drawChessboardCorners(grayImag, board_size, Corners, true); //���ǵ�����
		namedWindow("gray_src", 1);
		//resizeWindow("gray_src", 600, 600);
		imshow("gray_src", grayImag); //��ʾͼƬ
		waitKey(200);
	}
	int totalImag; //ͼƬ����
	totalImag = AllCorners.size();
	cout << "total Imag = " << totalImag << endl;
	cout << "�ǵ��ȡ������" << endl;
	Size square_size = Size(board_square, board_square);  //ÿ�����̸�Ĵ�С
	vector<vector<Point3f>> corner_coor;				  //��ʵ�ǵ�����
	Mat InnerMatri = Mat(3, 3, CV_32FC1, Scalar::all(0)); //�ڲξ���
	Mat distMatri = Mat(1, 5, CV_32FC1, Scalar::all(0));  //�������
	vector<Mat> translationMat;							  //ƽ�ƾ���
	vector<Mat> rotation;								  //��ת����
	cout << "��ʼ����궨...." << endl;
	Point3f realpoint;
	for (int i = 0; i < imagcount; i++)
	{
		vector<Point3f> tempPoint; //����ÿ��ͼ�Ľǵ���ʵ����
		for (int j = 0; j < board_size.height; j++)
		{
			for (int k = 0; k < board_size.width; k++)
			{
				realpoint.x = k * square_size.width;
				realpoint.y = j * square_size.height;
				realpoint.z = 0;
				tempPoint.push_back(realpoint);
			}
		}
		corner_coor.push_back(tempPoint);
	}
	cout << "����ڲ�������11��\n"
		 << InnerMatri << endl;
	cout << "����ϵ��11��\n"
		 << distMatri << endl;
	calibrateCamera(corner_coor, AllCorners, image_size, InnerMatri, distMatri, rotation, translationMat, 0); //�궨
	cout << "�궨��������" << endl;
	cout << "��ʼ������...." << endl;
	cout << "����ڲ�������11��\n"
		 << InnerMatri << endl;
	cout << "����ϵ��11��\n"
		 << distMatri << endl;
	fout << "����ڲ�������\n"
		 << InnerMatri << endl;
	fout << "����ϵ����\n"
		 << distMatri << endl;
	for (int i = 0; i < imagcount; i++)
	{
		Mat tmp, tmpr, tmpt, rota_Mat, pnp_R, pnp_t; // �������ѭ�����棬��Ȼ���bug
		cv::solvePnP(corner_coor[i], AllCorners[i], InnerMatri, distMatri, pnp_R, pnp_t, false, SOLVEPNP_ITERATIVE);
		Rodrigues(pnp_R, rota_Mat);
		tmpr = rota_Mat;
		tmpt = pnp_t / 1000.0f;
		R_target2cam.push_back(tmpr);
		t_target2cam.push_back(tmpt);
		fout << "��" << i + 1 << "��ͼ����������ϵ����ת����Ϊ��\n"
			 << tmpr << endl;
		fout << "��" << i + 1 << "��ͼ����������ϵ��ƽ�ƾ���Ϊ��\n"
			 << tmpt << endl;
		tmp = R_T2RT(tmpr, tmpt);
		vecHc.push_back(tmp);
	}
	cout << "�궨��������" << endl;
	cout << "��ʼ������...." << endl;
	ifstream ifile(DATAPATH + "rpy.txt", ios::in);
	if (!ifile)
	{
		cout << "δ�ҵ���������̬�����ļ�������" << endl;
	}
	string strr;
	istringstream ostr;
	string lefted;
	vector<Mat> toolbase;
	while (getline(ifile, strr))
	{
		Mat tmp(1, 6, CV_64F);
		;
		ostr.clear();
		ostr.str(strr);
		for (int j = 0; j < 6; j++)
		{
			ostr >> tmp.at<double>(0, j);
		}
		toolbase.push_back(tmp);
	}
	cout << "��ȡ��е��λ���ļ�����" << endl;
	for (int i = 0; i < imagcount; i++) //�����е��λ��
	{
		cout << "�����" << i + 1 << "���е��λ��" << endl;
		cv::Mat tmp = attitudeVectorToMatrix(toolbase[i], false, "zyz"); //��е��λ��Ϊŷ����-��ת����
		vecHg.push_back(tmp);
		RT2R_T(tmp, tempR, tempT);
		cv::cv2eigen(tempR, rotation_matrix);
		Eigen::Vector3d eulerAngle = rotation_matrix.eulerAngles(2, 1, 2);
		R_gripper2base.push_back(tempR);
		t_gripper2base.push_back(tempT);
	}
	//���۱궨��CALIB_HAND_EYE_TSAI��Ϊ11ms�����
	calibrateHandEye(R_gripper2base, t_gripper2base, R_target2cam, t_target2cam, R_cam2gripper, t_cam2gripper, CALIB_HAND_EYE_TSAI);
	Hcg = R_T2RT(R_cam2gripper, t_cam2gripper); //����ϲ�
	fout << "������е��֮������λ��Ϊ��\n"
		 << Hcg << endl;
	cout << "Hcg ����Ϊ��\n " << Hcg << endl;
	cout << "�Ƿ�Ϊ��ת����" << isRotationMatrix(Hcg) << std::endl
		 << std::endl; //�ж��Ƿ�Ϊ��ת����
	//Tool_In_Base*Hcg*Cal_In_Cam����ÿ�����ݽ��жԱ���֤
	for (int i = 0; i < imagcount; i++)
	{
		//��������ϵ��֤
		cout << "��" << i << "���궨�����������ϵ����������֤�궨����ȷ�ԣ�" << endl;
		cout << vecHg[i] * Hcg * vecHc[i] << endl;
	}
}
