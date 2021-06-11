#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>


#define W_DST "dstImage"
#define W_RST "resultImage"
#define PI 3.1415926
#define fps 30

class My_rec
{
public:
	cv::Point2f center;											 //��������ϵ���е�λ��
	cv::Point2f vertex[4];										 //��������ϵ�ж���λ��
	cv::Point3f c_center;										 //�������ϵ���е�λ��
	cv::Point3f c_vertex[4];									 //�������ϵ�ж���λ��
	cv::Point3f w_center;										 //��������ϵ���е�λ��
	cv::Point3f w_vertex[4];									 //��������ϵ�ж���λ��
	int id;														 //0�������� 1������
	double theta, length, width, w_length, w_width, w_theta;     //��������ϵƫת�Ƕȣ����߳��ȣ��̱߳��ȣ���������ϵ���߳��ȣ��̱߳��ȣ�ƫת�Ƕ�
	void rec(cv::Point2f* p);									 //ͨ���ı����ĸ���������ʼ��
	void sort();												 //�Զ�������
	void print();												 //��ӡ��Ϣ
	void uv_to_xyz();											 //�е�λ�ô��������굽������ꡢ��������ϵ
};


void Get_RGB();
//double get_distance(Point2f p1, Point2f p2);
//double get_distance(Point3f p1, Point3f p2);

cv::Point3f pixel_to_camera(cv::Point2f p);

cv::Point3f camera_to_world(cv::Point3f p);

double get_distance(cv::Point2f p1, cv::Point2f p2);

double get_distance(cv::Point3f p1, cv::Point3f p2);

void ColorDect(int, void*, std::string imgPath);
