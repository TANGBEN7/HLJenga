#pragma once
#include <vector>
#include <iostream>
using namespace std;

struct PosStruct
{
	double x;				// x���꣬��λmm
	double y;				// y���꣬��λmm
	double z;				// z���꣬��λmm
	double yaw;				// yaw���꣬��λ��
	double pitch;			// pitch���꣬��λ��
	double roll;			// roll���꣬��λ��
	bool config[3]{ 1,1,1 };	// config, ��ʾ��������̬
};

class CHLMotionPlan
{
private:
	double mJointAngleBegin[6];					//��ʼ��λ�ĹؽڽǶ�,��λ��
	double mJointAngleEnd[6];					//������λ�ĹؽڽǶȣ���λ��
	double mStartMatrixData[16];				//��ʼ��λ��ת����������
	double mEndMatrixData[16];					//������λ��ת����������
	double mSampleTime;							//������λ����λS
	double mVel;								//�ٶȣ���λm/s
	double mAcc;								//���ٶȣ���λm/s/s
	double mDec;								//���ٶȣ���λm / s / s
	bool mConfig[3];							//��������̬
	double start_x, start_y, start_z;
	double end_x, end_y, end_z;
	double end_yaw, end_pitch, end_roll;
	double start_yaw, start_pitch, start_roll;
	double mAngleVel;							// �ؽ��ٶȣ���λ��/s
	double mAngleAcc;							// �ؽڼ��ٶȣ���λ��/s/s
	double mAngleDec;							// �ؽڼ��ٶȣ���λ��/s/s

	// 1-viaPt LFPB
	double td12;
	double td23;
	vector<double> x_time;
	vector<double> y_time;
	vector<double> z_time;
	vector<double> r_time;
	vector<double> x_plan;
	vector<double> y_plan;
	vector<double> z_plan;
	vector<double> r_plan;

public:
	CHLMotionPlan();
	virtual ~CHLMotionPlan();

	// joint space and CCS line
	void SetSampleTime(double sampleTime);																	// ���ò���ʱ��
	void SetPlanPoints_line(PosStruct startPos, PosStruct endPos);												// ������ʼ��λ�ͽ�����λ�ĵѿ�������
	void SetProfile(double AngleVel, double AngleAcc, double AngleDec, double vel, double acc, double dec);	// �����˶��������ٶȡ����ٶȺͼ��ٶ�																					// ��ȡ�켣�滮����ɢ��λ	
	void GetPlanPoints_line(PosStruct startPos, PosStruct endPos, string fileNum, int *duration);							// ��ȡ�켣�滮����ɢ��λ

	// CCS 1-viaPt LFPB
	void SetSampleTime();							//���ò���ʱ��Ϊ0.001
	void SetPlanPoints(PosStruct startPos, PosStruct endPos);
	void SetProfile(double vel, double acc, double dec, double mtd12, double mtd23);
	void GetPlanPoints(int index);
	void GetPlanPointsSeg0(int index);
	void LFPB(double th1, double th2, double th3, double td12, double td23, vector<double>& dura, vector<double>& plan);
	void LFPB_Z(double th1, double th2, double th3, double td12, double td23, vector<double>& dura, vector<double>& plan);
	void LFPB_th6(double th1, double th2, double th3, double td12, double td23,
		vector<double>& dura, vector<double>& plan);
};

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}