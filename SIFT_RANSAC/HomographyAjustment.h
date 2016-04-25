#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
using std::vector;

#ifdef _DEBUG
#include <fstream>
#include <iomanip>
using namespace cv;
using namespace std;
#endif // _DEBUG

class HomographyAjustment
{
public:
	HomographyAjustment(void);

	/*����DLT����*/

public:
	~HomographyAjustment(void);

	/* ����ͬ��������:������ݡ���(x,y)���﷽���ꡪ����X,Y,Z��                                                                     */
	void ImportCognominalPoints(const vector<double> xL,
		const vector<double> yL,
		const vector<double> xR,
		const vector<double> yR);

	/* �����µ��������������µĲ���ֵ                                                                     */
	void UpdateParameters();

	/* ���ù��߷������¼����������Ľ���ֵ                                                                     */
	void UpdateApproximateCoordinates();

	/*  ���ƽ������������                                                                   */
	void GetIncrementalValue(int nCount);

	/* �����У����:һ��Ҫ�ȵ���ͬ�������ݣ�ImportCognominalPoints���������ô��������ֵ��SetParameterIntialValue��                                                                     */
	bool CameraCalibarion();

	/* ���ô�������ĳ�ֵ                                                                     */
	//void SetParameterIntialValue(double Xs,double Ys,double Zs,	double phi,double omega,double kappa,double fx,double fy,
	//	double x0,double y0,double K1,double K2,double P1,	double P2);
	void SetParameterIntialValue(Mat H);

	/* ȷ�ϵ����Ƿ���ֹ                                                                     */
	bool IsTerminating();

	/* ��ȡ���յļ���ֵ                                                                     */
	void GetFinalResult(vector<double> & parameters);

private:
	/* ������������ⷽλԪ�أ����࣬���������� �ĸ���ֵ������ÿ��ƽ������õ��Ĳ�����������������                                                                    */
	vector<double> IncrementalValue;

	vector<double> priorIncrementalValue;
	/* ��������ֵ����                                                                     */
	vector<double> xLSet;
	vector<double> yLSet;
	vector<double> xRSet;
	vector<double> yRSet;
	/* ����������ֵ����                                                                     */
	vector<double> x_approximate;
	vector<double> y_approximate;

	set<int> hehe;
	/* ��Ҫ����Ĳ�����������ⷽλԪ�أ����࣬����������                                                                     */
	//�ⷽλԪ��
	//��λ������+����
	double p0;
	double p1;
	double p2;
	double p3;
	double p4;
	double p5;
	double p6;
	double p7;
	double p8;


	//���ȼ���
	vector<double> accuracies;

	//��ֵ����
	double xx0;
	double yy0;
	double qr[12];
#ifdef _DEBUG
	ofstream ofs;

#endif // _DEBUG
	int nCount;
	/* ���(�Ƿ��Ѿ�����ͬ���������ݣ��Ƿ��Ѿ����ò�����ֵ)                                                                     */
	bool IsCognominalPointsImported;

	bool IsIntialValueSet;
};
