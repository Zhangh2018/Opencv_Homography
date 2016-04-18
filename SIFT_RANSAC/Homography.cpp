#include "Homography.h"


Homography::Homography()
{
}


Homography::~Homography()
{
}

CvMat* lsq_homog(vector<Point2f> leftPts, vector<Point2f> RightPts, int n)
{
	CvMat* H, *A, *B, X;
	double x[9];//����x�е�Ԫ�ؾ��Ǳ任����H�е�ֵ
	int i;

	//�����Ը�������4
	if (n < 4)
	{
		fprintf(stderr, "Warning: too few points in lsq_homog(), %s line %d\n",
			__FILE__, __LINE__);
		return NULL;
	}

	//���任����Hչ����һ��8ά������X�У�ʹ��AX=B������ֻ��һ�ν����Է����鼴�����X��Ȼ���ٸ���X�ָ�H
	/* set up matrices so we can unstack homography into X; AX = B */
	A = cvCreateMat(2 * n, 8, CV_64FC1);//����2n*8�ľ���һ����8*8
	B = cvCreateMat(2 * n, 1, CV_64FC1);//����2n*1�ľ���һ����8*1
	X = cvMat(8, 1, CV_64FC1, x);//����8*1�ľ���ָ������Ϊx
	H = cvCreateMat(3, 3, CV_64FC1);//����3*3�ľ���
	cvZero(A);//��A����

	//������չ�����㣬��Ҫ����ԭ���ľ�����㷨�����·������A��B��ֵ������
	for (i = 0; i < n; i++)
	{
		cvmSet(A, i, 0, leftPts[i].x);//���þ���A��i��0�е�ֵΪpts[i].x
		cvmSet(A, i + n, 3, leftPts[i].x);
		cvmSet(A, i, 1, leftPts[i].y);
		cvmSet(A, i + n, 4, leftPts[i].y);
		cvmSet(A, i, 2, 1.0);
		cvmSet(A, i + n, 5, 1.0);
		cvmSet(A, i, 6, -leftPts[i].x * RightPts[i].x);
		cvmSet(A, i, 7, -leftPts[i].y * RightPts[i].x);
		cvmSet(A, i + n, 6, -leftPts[i].x * RightPts[i].y);
		cvmSet(A, i + n, 7, -leftPts[i].y * RightPts[i].y);
		cvmSet(B, i, 0, RightPts[i].x);
		cvmSet(B, i + n, 0, RightPts[i].y);
	}

	//����OpenCV�����������Է�����
	cvSolve(A, B, &X, CV_SVD);//��X��ʹ��AX=B
	x[8] = 1.0;//�任�����[3][3]λ�õ�ֵΪ�̶�ֵ1
	X = cvMat(3, 3, CV_64FC1, x);
	cvConvert(&X, H);//������ת��Ϊ����

	cvReleaseMat(&A);
	cvReleaseMat(&B);
	return H;
}