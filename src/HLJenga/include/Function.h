#pragma once
#include <vector>
#include "RectDetect.h"
#include <string>
#include "MotionPlan.h"

struct pointCCSCfg
{
	double x;
	double y;
	double z;
	double theta1;
	double theta2;
	double theta3;
};

struct pointJointCfg
{
	double j1;
	double j2;
	double j3;
	double j4;
	double j5;
	double j6;
};

//���ڴ������ع��ܺ�����������˵ġ����ӳ�ʼ���������˵��˶�����ľ��ץȡ˳���

//�Ի���������Ϊ��
PosStruct cfgToPos(pointCCSCfg cfg);

void RobotConnect();

void initialization();

void close();

void sysLogin();

void powerUp();

void sysAbort();

void sysStart();

void home();

void setSpeed(std::string speed);

void moveToJointConfig(pointJointCfg cfg);

void moveToCCSConfig(pointCCSCfg cfg);

void enablePPB();

void disablePPB();

void setJointFrame();

void setWorldFrame();

void ppbToStartPoint();

void ppbLoadFile(std::string path);

void ppbRun();

void printCCSConfig();

void printJointConfig();

void ppbStop();

void enterAutoMode();

void vacuumStart();

void vacuumStop();

void pressureStart();

void pressureStop();