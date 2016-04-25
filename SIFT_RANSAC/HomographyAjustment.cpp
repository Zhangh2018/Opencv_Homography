#include "HomographyAjustment.h"
#include "math.h"
#include <opencv2/opencv.hpp>
#include <cmath>
//��ת�������
//������� 6����  4���ڣ�fx fy�� 4������ϵ�� ��ʱ�����������
const int nParameters = 8;
using namespace cv;
HomographyAjustment::HomographyAjustment(void)
{

	IsCognominalPointsImported = false;
	IsIntialValueSet = false;
	p0 = 0; p1 = 0; p2 = 0;
	p3 = 0; p4 = 0; p5 = 0;
	p6 = 0; p7 = 0; p8 = 0;
	//sx = -1500; sy = 0;
	IncrementalValue.resize(nParameters);
	priorIncrementalValue.resize(nParameters);
#ifdef _DEBUG
	ofs.open("E:\\log_10250.txt", ios_base::trunc | ios_base::in);
	nCount = 0;
#endif // _DEBUG

}

HomographyAjustment::~HomographyAjustment(void)
{
#ifdef _DEBUGv
	ofs.flush();
	ofs.close();
	nCount = 0;
#endif // _DEBUG
}

void HomographyAjustment::ImportCognominalPoints(const vector<double> xL,
	const vector<double> yL,
	const vector<double> xR,
	const vector<double> yR
	)
{
	//ASSERT(xL.size() == yL.size());//���Ե�ʹ�� ȷ����ʽ������ͬ
	//ASSERT(xR.size() == yR.size());
	//ASSERT(yL.size() == xR.size());
	this->xLSet.clear();
	this->yLSet.clear();
	this->xRSet.clear();
	this->yRSet.clear();
	for (size_t i = 0; i< xL.size(); i++)
	{
		this->xLSet.push_back(xL[i]);
		this->yLSet.push_back(yL[i]);
		this->xRSet.push_back(xR[i]);
		this->yRSet.push_back(yR[i]);
	}
	IsCognominalPointsImported = true;
	x_approximate.resize(xL.size());
	y_approximate.resize(yL.size());
}

/* ���ô�������ĳ�ֵ                                                                     */
//void HomographyAjustment::SetParameterIntialValue(double Xs,double Ys,double Zs,	double phi,double omega,double kappa,double fx,double fy,
//														   double x0,double y0,double K1,double K2,double P1,	double P2)
void HomographyAjustment::SetParameterIntialValue(Mat H)
{
	//1-1 1-2����
	//this->p0 = 1.04333566755;	this->p1 = -0.00251946695;	this->p2 = -3100;
	////this->p2 = -3081.797239701;
	//this->p3 = 0.00748672336659; this->p4 = 1.0239352537; this->p5 = 12.48172391;
	//this->p6 = 9.8987587e-006;	 this->p7 = -5.4800756038056e-006;

	//a1-a2����
	//this->p0 = 1.02215;	this->p1 = 0.00674;	this->p2 = -2974.7423;
	//this->p3 = 0.0003848283; this->p4 = 1.0192274; this->p5 = 26.937916336;
	//this->p6 = 4.672e-006;	 this->p7 = 5.603283e-006;

	//
	/*this->p0 = 1.0271305;	this->p1 = 0.007651189602;	this->p2 = -2985.5139;
	this->p3 = 0.002849349; this->p4 = 1.017; this->p5 = 18.118161496;
	this->p6 = 0.00000469;	 this->p7 = 0.000005218; p8 = 1;*/

	//this->p0 = 1.0189;	this->p1 = 0.00584;	this->p2 = -2964.241;
	//this->p3 = -0.0002737; this->p4 = 1.015548; this->p5 = 30.203;
	//this->p6 = 4.14543e-006;	 this->p7 = -7.3237e-007;
	
	p0 = H.at<double>(0, 0);
	p1 = H.at<double>(0, 1);
	p2 = H.at<double>(0, 2);
	p3 = H.at<double>(1, 0);
	p4 = H.at<double>(1, 1);
	p5 = H.at<double>(1, 2);
	p6 = H.at<double>(2, 0);
	p7 = H.at<double>(2, 1);
	p8 = 1;
}

/* ��ת�����������(ÿһ�ε�����ͨ���µ��ⷽλԪ���еĽ�Ԫ�ؿ��Ը�����ת����)                                                                     */
/* �����µ��������������µ�14������ֵ                                                                     */
void  HomographyAjustment::UpdateParameters()
{
	p0 += IncrementalValue[0];
	p1 += IncrementalValue[1];
	p2 += IncrementalValue[2];
	p3 += IncrementalValue[3];

	p4 += IncrementalValue[4];
	p5 += IncrementalValue[5];
	p6 += IncrementalValue[6];
	p7 += IncrementalValue[7];


#ifdef _DEBUG
	ofs << "*******************************���ռ�����*******************************:\n" << endl;
	ofs << fixed << setprecision(6) << p0 << "\t";
	ofs << fixed << setprecision(6) << p1 << "\t";
	ofs << fixed << setprecision(6) << p2 << endl;
	ofs << fixed << setprecision(6) << p3 << "\t";
	ofs << fixed << setprecision(6) << p4 << "\t";
	ofs << fixed << setprecision(6) << p5 << endl;
	ofs << fixed << setprecision(6) << p6 << "\t";
	ofs << fixed << setprecision(6) << p7 << "\t";
	ofs << fixed << setprecision(6) << 1 << endl;

	//for (int q = 0; q < nParameters; q++){
	//	ofs << "    \tm" + q ;
	//	ofs << " = ";
	//	ofs<<scientific << setprecision(6) << accuracies[q] << endl;
	//}
#endif // _DEBUG
}
/* ���ù��߷���-���ݿ��Ƶ��﷽����-���¼�����������ֵ                                                                     */
void HomographyAjustment::UpdateApproximateCoordinates()
{

#ifdef _DEBUG
	//ofs<<"����������:"<<endl;
	//ofs<<'\t'<<"x"<<'\t'<<"y"<<endl;
#endif // _DEBUG
	for (size_t i = 0; i<x_approximate.size(); i++)
	{
		double X_cal = (xLSet[i] * p0 + yLSet[i] * p1 + p2) / (xLSet[i] * p6 + yLSet[i] * p7 + 1);
		double Y_cal = (xLSet[i] * p3 + yLSet[i] * p4 + p5) / (xLSet[i] * p6 + yLSet[i] * p7 + 1);

		x_approximate[i] = X_cal;//���x����ֵ
		y_approximate[i] = Y_cal;//���y����ֵ
		//#ifdef _DEBUG
		//		ofs<<'\t'<<fixed<<setprecision(6)<<x_approximate[i]<<'\t'<<y_approximate[i]<<endl;
		//#endif // _DEBUG
	}
}

//��X ��Y ��Z ���� ���� ��k A������ 18��ֵ ��2*3����Ԫ���й� ��2*3���Ԫ���й� ��2*3���ڷ�λԪ���й�
/* ���ƽ������������                                                                   */

void HomographyAjustment::GetIncrementalValue(int nCount)
{
	int nPts = (int)xLSet.size();
	//Mat tempMask = Mat::ones(m_LeftInlier.size(), 1, CV_8U);
	Mat DesignMatrix = Mat::zeros(nPts * 2, nParameters, CV_64F);
	Mat constMatrix = Mat::zeros(nPts * 2, 1, CV_64F);

	//��A���󸳳�ֵ
	double a11 = 0, a12 = 0, a13 = 0, a14 = 0, a15 = 0, a16 = 0, a17 = 0, a18 = 0;
	double a21 = 0, a22 = 0, a23 = 0, a24 = 0, a25 = 0, a26 = 0, a27 = 0, a28 = 0;
	int deleteCount = 0;
	//��������A����ֵ������ÿ���﷽�����µ���������ֵ
	for (int i = 0; i<nPts; i++)
	{
		//����ÿ�ε���ʱ��X Y Z-ba
		if (hehe.count(i) >= 1){
			continue;
		}
		double homoScale = p6 * xLSet[i] + p7*yLSet[i] + p8;

		a11 = xLSet[i] / homoScale;//x��Xsƫ��
		a12 = yLSet[i] / homoScale;//x��Ysƫ��
		a13 = 1 / homoScale;//x��Zsƫ��
		a14 = 0;
		a15 = 0;
		a16 = 0;

		a21 = 0;
		a22 = 0;
		a23 = 0;
		a24 = xLSet[i] / homoScale;
		a25 = yLSet[i] / homoScale;
		a26 = 1 / homoScale;//y��k��

		a17 = -(p0*xLSet[i] * xLSet[i]) / (homoScale*homoScale);
		a18 = -(p1*yLSet[i] * yLSet[i]) / (homoScale*homoScale);

		a27 = -(p3*xLSet[i ] * xLSet[i ]) / (homoScale*homoScale);
		a28 = -(p4*yLSet[i] * yLSet[i ]) / (homoScale*homoScale);
		
		

		double vvvx = xRSet[i ] - x_approximate[i];
		double vvvy = yRSet[i ] - y_approximate[i];
		//#ifdef _DEBUG
		//	ofs << '\t' << fixed << setprecision(6) << vvvx << '\t' << vvvy << endl;
		//#endif // _DEBUG
		//

		if (nCount >= 10 && (vvvx > 3.0 || vvvy > 3.0)){
			deleteCount++;
			hehe.insert(i);
			continue;
		}
		DesignMatrix.at<double>((i-deleteCount) * 2, 0) = a11;
		DesignMatrix.at<double>((i-deleteCount) * 2, 1) = a12;
		DesignMatrix.at<double>((i-deleteCount) * 2, 2) = a13;
		DesignMatrix.at<double>((i-deleteCount) * 2, 3) = a14;
		DesignMatrix.at<double>((i-deleteCount) * 2, 4) = a15;
		DesignMatrix.at<double>((i-deleteCount) * 2, 5) = a16;
		DesignMatrix.at<double>((i-deleteCount) * 2, 6) = a17;
		DesignMatrix.at<double>((i-deleteCount) * 2, 7) = a18;

		DesignMatrix.at<double>((i-deleteCount) * 2 + 1, 0) = a21;
		DesignMatrix.at<double>((i-deleteCount) * 2 + 1, 1) = a22;
		DesignMatrix.at<double>((i-deleteCount) * 2 + 1, 2) = a23;
		DesignMatrix.at<double>((i-deleteCount) * 2 + 1, 3) = a24;
		DesignMatrix.at<double>((i-deleteCount) * 2 + 1, 4) = a25;
		DesignMatrix.at<double>((i-deleteCount) * 2 + 1, 5) = a26;
		DesignMatrix.at<double>((i-deleteCount) * 2 + 1, 6) = a27;
		DesignMatrix.at<double>((i-deleteCount) * 2 + 1, 7) = a28;

		constMatrix.at<double>((i-deleteCount) * 2, 0) = vvvx;//���V��ֵ=ʵ��ֵ�����ֵ�Ĳ�
		constMatrix.at<double>((i-deleteCount) * 2 + 1, 0) = vvvy;
	}
	DesignMatrix.resize(nPts - deleteCount, nParameters);
	constMatrix.resize(nPts - deleteCount, 1);

	//cout << deleteCount << "   " << xLSet.size() << "    " << endl;
#ifdef _DEBUG
	ofs << "����������:" << endl;
#endif // _DEBUG
	accuracies.clear();
	//��ʼ����
	Mat AT = DesignMatrix.t();
	Mat ATA = AT*DesignMatrix;
	Mat ATA_1 = ATA.inv();
	Mat ATL = AT*constMatrix;
	Mat result = ATA_1*ATL;
	//cout << result << endl;

	for (int i = 0; i<result.rows; i++)
	{
		IncrementalValue[i] = result.at<double>(i, 0);
#ifdef _DEBUG
		ofs << '\t' << fixed << setprecision(6) << IncrementalValue[i] << endl;
#endif // _DEBUG
	}
	Mat vv = DesignMatrix*result - constMatrix;
	double m00 = 0.0;
	for (int j = 0; j < nParameters; j++){
		m00 += vv.at<double>(j, 0) * vv.at<double>(j, 0);
	}
	m00 = sqrt(m00 / (2 * xLSet.size() - nParameters));
	for (int j = 0; j < nParameters; j++){
		accuracies.push_back(sqrt(ATA_1.at<double>(j, j))*m00);
	}
}


/* �����У����:һ��Ҫ�ȵ���ͬ�������ݣ�ImportCognominalPoints���������ô��������ֵ��SetParameterIntialValue��                                                                     */
bool HomographyAjustment::CameraCalibarion()
{

	//	if (!IsCognominalPointsImported||!IsIntialValueSet)
	if (!IsCognominalPointsImported)
	{
		return false;
	}

	do
	{
#ifdef _DEBUG
		ofs << "��" << nCount << "�ε���:----------------------------------------" << endl;
		ofs << "priorIncrementalValue:\n" << endl;
		nCount++;
#endif // _DEBUG
		for (size_t i = 0; i<priorIncrementalValue.size(); i++)
		{
			priorIncrementalValue[i] = IncrementalValue[i];
#ifdef _DEBUG
			ofs << '\t' << priorIncrementalValue[i] << "  ";
#endif // _DEBUG
		}
#ifdef _DEBUG
		ofs << endl;
#endif // _DEBUG
		UpdateApproximateCoordinates();
		GetIncrementalValue(nCount);
		UpdateParameters();
	} while (!IsTerminating() && nCount<100);
	cout << "ɾ������ �� " << hehe.size()<< endl;
	return true;
}

void HomographyAjustment::GetFinalResult(vector<double> &parameters)
{
	parameters.push_back(p0);
	parameters.push_back(p1);
	parameters.push_back(p2);
	parameters.push_back(p3);
	parameters.push_back(p4);
	parameters.push_back(p5);
	parameters.push_back(p6);
	parameters.push_back(p7);

	for (int q = 0; q < nParameters; q++){
		parameters.push_back(accuracies[q]);
	}
}
bool HomographyAjustment::IsTerminating()
{
	const double termWucha = 0.00000001;
	vector<double> delta;
	for (size_t i = 0; i<nParameters; i++)
	{
		delta.push_back(IncrementalValue[i] - priorIncrementalValue[i]);//�޲��Ǳ���������ǰ�������Ĳ�
	}
	for (size_t i = 0; i<nParameters; i++)
	{

		if (delta[i]<termWucha)
			//if (delta[i] <= termAngle)
		{
			continue;
		}
		else
		{
			return false;
		}
	}
	return true;
}
