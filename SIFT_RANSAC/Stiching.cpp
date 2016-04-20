#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/nonfree/nonfree.hpp>  
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <cv.h>
#include "cxcore.h"
#include <highgui.h>
#include <iostream>  
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include "Stiching.h"
using namespace cv; 
#define MAX_POINT_SIZE 20000
#define MAX_CORNERPOINT_NUM 10000
	//RANSAC_homography(num_matched, matched1, matched2, H, inlier_mask);
Stiching::Stiching(){
	
}
Stiching::~Stiching(){

}
int Stiching::ProcessStitching(String strs[])
{
	ofstream ofs;
	ofstream ofs_inline;
#ifdef _DEBUG
	ofs.open("E:\\feature_matched.txt", ios_base::trunc | ios_base::in);
	ofs_inline.open("E:\\inline_matched.txt", ios_base::trunc | ios_base::in);
	ofs << "    X��            Y��             X��           Y��" << endl;
#endif // _DEBUG
	initModule_nonfree();//��ʼ��ģ�飬ʹ��SIFT��SURFʱ�õ� 
	Ptr<FeatureDetector> detector = FeatureDetector::create( "SURF" );
	//����SIFT������������ɸĳ�SURF/ORB SURF�ȶ� ���Ⱥͽ����SIFT���� ��Ч�ʸ߳�����
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create( "SURF" );//���������������������ɸĳ�SURF/ORB
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create( "BruteForce" );//��������ƥ����  
//	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create("FlannBased");//��������ƥ����  
	if( detector.empty() || descriptor_extractor.empty() )  
		cout<<"fail to create detector!";  

	//����ͼ��  
	Mat img1 = imread(strs[0]);
	Mat img2 = imread(strs[1]);
	//���ͼ�����ROIѡȡ
	Rect roi(2888, 0, 1400, 1600);
	Rect roi2(0, 0, 1400, 1600);
	Mat roi_of_image1 = img1(roi);
	Mat roi_of_image2 = img2(roi2);
	//��������  
	double t = getTickCount();//��ǰ�δ���  
	vector<KeyPoint> m_LeftKey,m_RightKey;
	detector->detect(roi_of_image1, m_LeftKey);//���img1�е�SIFT�����㣬�洢��m_LeftKey��  
	detector->detect(roi_of_image2, m_RightKey);
	cout<<"ͼ��1���������:"<<m_LeftKey.size()<<"\t\tͼ��2���������:"<<m_RightKey.size()<<endl;  

	//����������������������Ӿ��󣬼�������������  
	Mat descriptors1,descriptors2;  
	descriptor_extractor->compute(roi_of_image1, m_LeftKey, descriptors1);
	descriptor_extractor->compute(roi_of_image2, m_RightKey, descriptors2);
	double t1 = ((double)getTickCount() - t) / getTickFrequency();
	cout<<"SURF�㷨��ʱ��"<<t1<<"��"<<endl;  
	//ROI��ȡ������������ͼΪ����ϵ ��˴�ͼ��Ҫת��
	for (int i = 0; i<m_LeftKey.size(); i++)
	{
		m_LeftKey[i].pt.x += 2888;//���
	}
	//����������  
	Mat img_m_LeftKey,img_m_RightKey;  
	drawKeypoints(img1,m_LeftKey,img_m_LeftKey,Scalar::all(-1),0);  
	drawKeypoints(img2,m_RightKey,img_m_RightKey,Scalar::all(-1),0);  
	//imshow("Src1",img_m_LeftKey);  
	//imshow("Src2",img_m_RightKey);  
	cvSaveImage("E:\\Src1.jpg", &IplImage(img_m_LeftKey));
	cvSaveImage("E:\\Src2.jpg", &IplImage(img_m_RightKey));
	//����ƥ��  
	vector<DMatch> matches;//ƥ����
	//�ٵ�һ��ɸѡ opencv�ڲ���matcherƥ���� Ӧ��ʹ��kdtree��������ŷ����þ��������ƥ��
	descriptor_matcher->match( descriptors1, descriptors2, matches );//ƥ������ͼ�����������  
	cout<<"Match������"<<matches.size()<<endl;  

	//����ƥ�����о����������Сֵ  
	//������ָ���������������ŷʽ���룬�������������Ĳ��죬ֵԽС��������������Խ�ӽ�  
	double max_dist = 0;  
	double min_dist = 100;
	for(int i=0; i<matches.size(); i++)  
	{  
		double dist = matches[i].distance;  
		if(dist < min_dist) min_dist = dist;  
		if(dist > max_dist) max_dist = dist;  
	}  
	cout<<"�����룺"<<max_dist<<endl;  
	cout<<"��С���룺"<<min_dist<<endl;  

	//ɸѡ���Ϻõ�ƥ���  
	vector<DMatch> goodMatches;
	//�ڵڶ���ɸѡ ������һ�������õ���ƥ��� ����ŷ����þ������ �����/��С���Ա�
	for(int i=0; i<matches.size(); i++)  
	{  
		if(matches[i].distance < 0.2 * max_dist)  
			//���� ֻѡȡ���ֵ���֮һ�� ���Ե����ø�С��ɸѡ���ӽ��ĵ㼯 ��ƥ������Լ���
		//if (matches[i].distance < 3 * min_dist)
		{  
			goodMatches.push_back(matches[i]);  
		}  
	}  
	cout<<"goodMatch������"<<goodMatches.size()<<endl;  

	//����ƥ����  
	Mat img_matches;  //��ʾƥ���ͼ��
	//��ɫ���ӵ���ƥ���������ԣ���ɫ��δƥ���������  
	drawMatches(img1,m_LeftKey,img2,m_RightKey,goodMatches,img_matches,  
		Scalar::all(-1)/*CV_RGB(255,0,0)*/,CV_RGB(0,255,0),Mat(),2);  
	double ttt = ((double)getTickCount() - t) / getTickFrequency();
	cout << "��ֹ��ƥ����ʱ��" << ttt << "��" << endl;
//	imshow("MatchSIFT",img_matches);
	cvSaveImage("E:\\MatchSIFT.jpg", &IplImage(img_matches));
	IplImage result=img_matches;

	//waitKey(0);  //�ȴ�����esc���˳�


	//RANSACƥ�����
	vector<DMatch> m_Matches=goodMatches;
	// ����ռ�
	int ptCount = (int)m_Matches.size();
	Mat p1(ptCount, 2, CV_32F);
	Mat p2(ptCount, 2, CV_32F);

	// ��Keypointת��ΪMat  
	Point2f pt;
	for (int i=0; i<ptCount; i++)
	{
		pt = m_LeftKey[m_Matches[i].queryIdx].pt;//���
		p1.at<float>(i, 0) = pt.x;
		p1.at<float>(i, 1) = pt.y;

		pt = m_RightKey[m_Matches[i].trainIdx].pt;//�ҵ�
		p2.at<float>(i, 0) = pt.x;
		p2.at<float>(i, 1) = pt.y;
	}

	// �۵�����ɸѡ ��RANSAC��������F ����F�������õ��ļ���ɸѡ��׵ĵ�
	Mat m_Fundamental;
	vector<uchar> m_RANSACStatus;       //����������ڴ洢RANSAC��ÿ�����״̬
	findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC);
	// ����Ұ�����
	double tt = ((double)getTickCount() - t) / getTickFrequency();
	cout << "��ֹRansacƥ����ʱ��" << tt << "��" << endl;
	int OutlinerCount = 0;
	for (int i=0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] == 0)    // ״̬Ϊ0��ʾҰ��
		{
			OutlinerCount++;
		}
	}
	int InlinerCount = ptCount - OutlinerCount;   // �����ڵ�
	cout<<"�ܵ���Ϊ �� "<<m_RANSACStatus.size()<<"  �ڵ���Ϊ��"<<InlinerCount<<endl;

	
   // �������������ڱ����ڵ��ƥ���ϵ
   vector<Point2f> m_LeftInlier;//���ڵ�
   vector<Point2f> m_RightInlier;//���ڵ�
   vector<DMatch> m_InlierMatches;//ƥ���

	m_InlierMatches.resize(InlinerCount);
	m_LeftInlier.resize(InlinerCount);
	m_RightInlier.resize(InlinerCount);
	InlinerCount=0;
	float inlier_minRx=img1.cols;        //���ڴ洢�ڵ�����ͼ��С�����꣬�Ա�����ں� �ü� Ĭ��ͼ��Ϊ����
	
	for (int i=0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] != 0)
		{
			m_LeftInlier[InlinerCount].x = p1.at<float>(i, 0);
			m_LeftInlier[InlinerCount].y = p1.at<float>(i, 1);
			m_RightInlier[InlinerCount].x = p2.at<float>(i, 0);
			m_RightInlier[InlinerCount].y = p2.at<float>(i, 1);
			m_InlierMatches[InlinerCount].queryIdx = InlinerCount;
			m_InlierMatches[InlinerCount].trainIdx = InlinerCount;
			if(m_RightInlier[InlinerCount].x<inlier_minRx) inlier_minRx=m_RightInlier[InlinerCount].x;   //�洢�ڵ�����ͼ��С������ Ϊ���ҳ�ͼ�������	
			InlinerCount++;
		}
	}

	// ���ڵ�ת��ΪdrawMatches����ʹ�õĸ�ʽ
	vector<KeyPoint> key1(InlinerCount);
	vector<KeyPoint> key2(InlinerCount);
	KeyPoint::convert(m_LeftInlier, key1);
	KeyPoint::convert(m_RightInlier, key2);

	//CvMat* HH, *A, *B, X;
	//double x[9];//����x�е�Ԫ�ؾ��Ǳ任����H�е�ֵ
	//int i;

	////�����Ը�������4
	////���任����Hչ����һ��8ά������X�У�ʹ��AX=B������ֻ��һ�ν����Է����鼴�����X��Ȼ���ٸ���X�ָ�H
	///* set up matrices so we can unstack homography into X; AX = B */
	//A = cvCreateMat(2 * key1.size(), 8, CV_64FC1);//����2n*8�ľ���һ����8*8
	//B = cvCreateMat(2 * key1.size(), 1, CV_64FC1);//����2n*1�ľ���һ����8*1
	//X = cvMat(8, 1, CV_64FC1, x);//����8*1�ľ���ָ������Ϊx
	//HH = cvCreateMat(3, 3, CV_64FC1);//����3*3�ľ���
	//cvZero(A);//��A����

	////������չ�����㣬��Ҫ����ԭ���ľ�����㷨�����·������A��B��ֵ������
	//for (int ii = 0; ii < key1.size(); ii++)
	//{
	//	//cvmSet(A, i, 0, key1[i].pt.x);//���þ���A��i��0�е�ֵΪpts[i].x
	//	//cvmSet(A, i + key1.size(), 3, key1[i].pt.x);
	//	//cvmSet(A, i, 1, key1[i].pt.y);
	//	//cvmSet(A, i + key1.size(), 4, key1[i].pt.y);
	//	//cvmSet(A, i, 2, 1.0);
	//	//cvmSet(A, i + key1.size(), 5, 1.0);
	//	//cvmSet(A, i, 6, -key1[i].pt.x *  key2[i].pt.x);
	//	//cvmSet(A, i, 7, -key1[i].pt.y *  key2[i].pt.x);
	//	//cvmSet(A, i + key1.size(), 6, -key1[i].pt.x *  key2[i].pt.y);
	//	//cvmSet(A, i + key1.size(), 7, -key1[i].pt.y *  key2[i].pt.y);
	//	//cvmSet(B, i, 0, key2[i].pt.x);
	//	//cvmSet(B, i + key1.size(), 0, key2[i].pt.y);

	//	cvmSet(A, 2 * ii, 0, key1[ii].pt.x);//���þ���A��i��0�е�ֵΪpts[i].x
	//	cvmSet(A, 2 * ii + 1, 3, key1[ii].pt.x);
	//	cvmSet(A, 2 * ii, 1, key1[ii].pt.y);
	//	cvmSet(A, 2 * ii + 1, 4, key1[ii].pt.y);
	//	cvmSet(A, 2 * ii, 2, 1.0);
	//	cvmSet(A, 2 * ii + 1, 5, 1.0);
	//	cvmSet(A, 2 * ii, 6, -key1[ii].pt.x *  key2[ii].pt.x);
	//	cvmSet(A, 2 * ii, 7, -key1[ii].pt.y *  key2[ii].pt.x);
	//	cvmSet(A, 2 * ii + 1, 6, -key1[ii].pt.x *  key2[ii].pt.y);
	//	cvmSet(A, 2 * ii + 1, 7, -key1[ii].pt.y *  key2[ii].pt.y);

	//	cvmSet(B, 2 * ii, 0, key2[ii].pt.x);
	//	cvmSet(B, 2 * ii + 1, 0, key2[ii].pt.y);
	//}

	////����OpenCV�����������Է�����
	//cvSolve(A, B, &X, CV_SVD);//��X��ʹ��AX=B
	//x[8] = 1.0;//�任�����[3][3]λ�õ�ֵΪ�̶�ֵ1
	//X = cvMat(3, 3, CV_64FC1, x);
	//cvConvert(&X, HH);//������ת��Ϊ����

	//cvReleaseMat(&A);
	//cvReleaseMat(&B);
	//cout <<endl<< "--SVM�㷨������Ϊ--   " << endl;
	//for (int row = 0; row < (*HH).rows; row++)
	//{
	//	float* pptr = (float*)((*HH).data.ptr + row * (*HH).step);//��row�����ݵ���ʼָ��
	//	for (int col = 0; col < (*HH).cols; col++)
	//	{
	//		//cout << *(pptr + 1 * col) <<"  ";
	//		cout << cvmGet(HH, row, col) << "\t";
	//	}
	//	cout << endl;
	//}
	//����homography
	CvPoint2D64f matched1[MAX_CORNERPOINT_NUM];
	CvPoint2D64f matched2[MAX_CORNERPOINT_NUM];
	//����H֮ǰ�����Ե�
	for (int i = 0; i < key1.size(); i++){		
#ifdef _DEBUG
		ofs << key1[i].pt.x << "   " << key1[i].pt.y << "   " << key2[i].pt.x << "   " << key2[i].pt.y << endl;
		matched1[i].x = key1[i].pt.x;
		matched1[i].y = key1[i].pt.y;
		matched2[i].x = key2[i].pt.x;
		matched2[i].y = key2[i].pt.y;
#endif // _DEBUG
	}
	CvMat *HH = cvCreateMat(3, 3, CV_64FC1);
	CvMat *inlier_mask = cvCreateMat(key1.size(), 1, CV_64FC1);
	
	Findhomography homo;
	homo.RANSAC_homography(key1.size(), matched1, matched2, HH, inlier_mask);
	CvPoint newmatched;
	int num_inlier = 0;
	for (int i = 0; i<key1.size(); i++){
		if (cvmGet(inlier_mask, i, 0) == 1){
			ofs_inline << matched1[i].x << "\t" << matched1[i].y << "\t" << matched2[i].x << "\t" << matched2[i].y << endl;
			num_inlier++;
		}
	}
	printf("�ڵ����Ϊ : %d\n", num_inlier);



	cout <<endl<< "--ֱ��Ransac�㷨������Ϊ--   " << endl;
	double c33 = cvmGet(HH, 2, 2);
	for (int row = 0; row < (*HH).rows; row++)
	{
		//float* pptr = (float*)((*HH).data.ptr + row * (*HH).step);//��row�����ݵ���ʼָ��
		for (int col = 0; col < (*HH).cols; col++)
		{
			//cout << *(pptr + 1 * col) <<"  ";
			cout << cvmGet(HH, row, col)/c33 << "\t";
		}
		cout << endl;
	}
    // ��ʾ����F������ڵ�ƥ��
	Mat OutImage;
	drawMatches(img1, key1, img2, key2, m_InlierMatches, OutImage);
//	cvNamedWindow( "Match features", 1);
//	cvShowImage("Match features", &IplImage(OutImage));
	cvSaveImage("Match features.jpg", &IplImage(OutImage));
	//waitKey(0);

	//cvDestroyAllWindows();

	//����H���Դ洢RANSAC�õ��ĵ�Ӧ����
	//Mat H = findHomography( m_LeftInlier, m_RightInlier, RANSAC );
//	Mat H=Mat::Mat(HH, true);


	//�ܵ��Ĵ�ɸѡ �ڽ���Hʱһ��Ransacһ��ѡ�����һֻ��
	Mat H = findHomography(m_LeftInlier, m_RightInlier, RANSAC);
	cout<< endl<< H << endl;

	//double aa1 = 1.045972;
	//double aa2 = -0.000410;
	//double aa3 = -3101.372651;
	//double bb1 = 0.004664;
	//double bb2 = 1.032072;
	//double bb3 = 16.766268;
	//double cc1 = 0.000010;
	//double cc2 = -0.000005;
	//double cc3 = 1;
	double aa1 = 1.027131;
	double aa2 = 0.007651;
	double aa3 = -2985.513937;
	double bb1 = 0.002849;
	double bb2 = 1.017038;
	double bb3 = 18.118161;
	double cc1 = 0.000005;
	double cc2 = 0.000005;
	double cc3 = 1;

	H.at<double>(0, 0) = aa1 / cc3;
	H.at<double>(0, 1) = aa2 / cc3;
	H.at<double>(0, 2) = aa3/ cc3;
	H.at<double>(1, 0) = bb1 / cc3;
	H.at<double>(1, 1) = bb2 / cc3;
	H.at<double>(1, 2) = bb3 / cc3;
	H.at<double>(2, 0) = cc1 / cc3;
	H.at<double>(2, 1) = cc2 / cc3;
	H.at<double>(2, 2) = 1;

	cout <<endl<< H << endl;
	//�洢��ͼ�Ľǣ�����任����ͼλ��
	std::vector<Point2f> obj_corners(4);
	obj_corners[0] = Point(0,0); obj_corners[1] = Point( img1.cols, 0 );
	obj_corners[2] = Point( img1.cols, img1.rows ); obj_corners[3] = Point( 0, img1.rows );
	std::vector<Point2f> scene_corners(4);
	perspectiveTransform( obj_corners, scene_corners, H);

	//������ͼ����H�任����ͼ����λ��
	Point2f offset( (float)img1.cols, 0);//��ͼ���½� ��Ϊƽ������  
	line( OutImage, scene_corners[0]+offset, scene_corners[1]+offset, Scalar( 0, 255, 0), 4 );
	line( OutImage, scene_corners[1]+offset, scene_corners[2]+offset, Scalar( 0, 255, 0), 4 );
	line( OutImage, scene_corners[2]+offset, scene_corners[3]+offset, Scalar( 0, 255, 0), 4 );
	line( OutImage, scene_corners[3]+offset, scene_corners[0]+offset, Scalar( 0, 255, 0), 4 );
//	imshow( "Good Matches & Object detection", OutImage );
	cvSaveImage("E:\\Good Matches Object detection.jpg", &IplImage(OutImage));
/*		while(1)
	{
		if(waitKey(100)==19) cvSaveImage("E:\\Good Matches Object detection.jpg",  &IplImage(OutImage));  
		if(waitKey(100)==27) break;
	}      */                                                        //��esc������ctl+s����ͼ��
	
	int drift = scene_corners[1].x;                                                        //����ƫ����

	//�½�һ������洢��׼������ͼ�Ľǵ�λ��
	int width = int(max(abs(scene_corners[1].x), abs(scene_corners[2].x)));
	int height= img1.rows;                                                                  //���ߣ�int height = int(max(abs(scene_corners[2].y), abs(scene_corners[3].y)));
	float origin_x=0,origin_y=0;
		if(scene_corners[0].x<0) {//���ӳ��� ��ͼ���Ͻ�ƫ�Ƶ�����x=0���
			if (scene_corners[3].x<0) origin_x+=min(scene_corners[0].x,scene_corners[3].x);//��������½�Ҳ�ܵ�x=0��� �趨����ͼ����߽�
		else origin_x+=scene_corners[0].x;}//������½�û����ƫ ����߽縳ֵ
		width-=int(origin_x);
	if(scene_corners[0].y<0) {//ͬ�� �趨�ϱ߽�
		if (scene_corners[1].y) origin_y+=min(scene_corners[0].y,scene_corners[1].y);
		else origin_y+=scene_corners[0].y;}
	//��ѡ��height-=int(origin_y);
	Mat imageturn=Mat::zeros(width,height,img1.type());//��ƴ �߶Ȳ���

	//��ȡ�µı任����ʹͼ��������ʾ
	for (int i=0;i<4;i++) {scene_corners[i].x -= origin_x; } 	//��ƴ  ��ѡ��ƴ��scene_corners[i].y -= (float)origin_y; }
	Mat H1=getPerspectiveTransform(obj_corners, scene_corners);
	cout <<endl<< H1 << endl;
	//����ͼ����ͼ��任����ʾЧ��
	warpPerspective(img1,imageturn,H1,Size(width,height));	
//	imshow("image_Perspective", imageturn);
	cvSaveImage("E:\\Left image after perspectiving.jpg", &IplImage(imageturn));
//	waitKey(0);

	//ͼ���ں�
	int width_ol=width-int(inlier_minRx-origin_x);
	int start_x=int(inlier_minRx-origin_x);
    cout<<"width: "<<width<<"\timg1.width: "<<img1.cols<<"\tstart_x: "<<start_x<<"\twidth_ol: "<<width_ol<<endl;
	uchar* ptr=imageturn.data;//ʹ��ָ������ݸ�ֵ
	double alpha=0, beta=1;
	for (int row=0;row<height;row++) {
		ptr=imageturn.data+row*imageturn.step+(start_x)*imageturn.elemSize();//��ǰ��ָ��
		for(int col=0;col<width_ol;col++)
		{
			uchar* ptr_c1=ptr+imageturn.elemSize1();
			uchar* ptr_c2=ptr_c1+imageturn.elemSize1();
			uchar* ptr2=img2.data+row*img2.step+(col+int(inlier_minRx))*img2.elemSize();
			uchar* ptr2_c1=ptr2+img2.elemSize1();  
			uchar* ptr2_c2=ptr2_c1+img2.elemSize1();

			alpha=double(col)/double(width_ol); beta=1-alpha;//ͼ���ں� Խ����ƴ���ߵĵط� ʹ��ƽ��ֵ �������ҷֱ�ʹ������ͼ�������

			if (*ptr==0&&*ptr_c1==0&&*ptr_c2==0) {
				*ptr=(*ptr2);
				*ptr_c1=(*ptr2_c1);
				*ptr_c2=(*ptr2_c2);
			}

			*ptr=(*ptr)*beta+(*ptr2)*alpha;
			*ptr_c1=(*ptr_c1)*beta+(*ptr2_c1)*alpha;
			*ptr_c2=(*ptr_c2)*beta+(*ptr2_c2)*alpha;

			ptr+=imageturn.elemSize();
		}	}
	
//	imshow("image_overlap", imageturn);
	cvSaveImage("E:\\Left image with overlap.jpg", &IplImage(imageturn));
	//waitKey(0);

	Mat img_result=Mat::zeros(height,width+img2.cols-drift,img1.type());
	uchar* ptr_r=imageturn.data;
	
	for (int row=0;row<height;row++) {
		ptr_r=img_result.data+row*img_result.step;

		for(int col=0;col<imageturn.cols;col++)
		{
			uchar* ptr_rc1=ptr_r+imageturn.elemSize1();  uchar*  ptr_rc2=ptr_rc1+imageturn.elemSize1();

			uchar* ptr=imageturn.data+row*imageturn.step+col*imageturn.elemSize();
			uchar* ptr_c1=ptr+imageturn.elemSize1();  uchar*  ptr_c2=ptr_c1+imageturn.elemSize1();

			*ptr_r=*ptr;
			*ptr_rc1=*ptr_c1;
			*ptr_rc2=*ptr_c2;

			ptr_r+=img_result.elemSize();
		}	

		ptr_r=img_result.data+row*img_result.step+imageturn.cols*img_result.elemSize();
		for(int col=imageturn.cols;col<img_result.cols;col++)
		{
			uchar* ptr_rc1=ptr_r+imageturn.elemSize1();  uchar*  ptr_rc2=ptr_rc1+imageturn.elemSize1();

			uchar* ptr2=img2.data+row*img2.step+(col-imageturn.cols+drift)*img2.elemSize();
			uchar* ptr2_c1=ptr2+img2.elemSize1();  uchar* ptr2_c2=ptr2_c1+img2.elemSize1();

			*ptr_r=*ptr2;
			*ptr_rc1=*ptr2_c1;
			*ptr_rc2=*ptr2_c2;

			ptr_r+=img_result.elemSize();
		}	
	}

	imshow("image_result", img_result);
	cvSaveImage("E:\\final_result.jpg", &IplImage(img_result));
	//	while(1)
	//{
	//	if(waitKey(100)==19) cvSaveImage("E:\\final_result.jpg",  &IplImage(img_result));  
	//	if(waitKey(100)==27) break;                     //��esc�˳���ctl+s����ͼ��
	//}
	system("pause");
	#ifdef _DEBUG
			ofs.flush();
			ofs.close();
			ofs_inline.flush();
			ofs_inline.close();
	#endif // _DEBUG
	return 0;
}

