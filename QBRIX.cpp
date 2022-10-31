// QBRIX.cpp : Defines the entry point for the console application.
// Michela Lecca - October 31, 2022

#include "stdafx.h"
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <queue>
#include <opencv2/opencv.hpp> 
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>


using namespace cv;
using namespace std;


#define SAVE
#define DEBUG

double*
ComputeHistogram(Mat image, int undersample)
{
	int     len = (int)(256.0 / undersample);
	double *histo = new double[len];
	int     k, r, c, nr = image.rows, nc = image.cols;
	int     val;

	for (c = 0; c < len; ++c)
		histo[c] = 0;

	for (r = 0; r < nr; ++r)
		for (c = 0; c < nc; ++c)
		{
			val = (int)(image.at<uchar>(r, c));
			k = val / undersample;
			histo[k] ++;
		}

	double A = nr*nc;

	for (c = 0; c < len; ++c)
		histo[c] /= A;

	return histo;
}

/*** spatially weighted histogram - as in the original paper ***/

double*
ComputeHistogramW (Mat image, 
					int x, int y, double alpha, int undersample)
{
	int     len = (int)(256.0 / undersample);
	double *histo = new double[len];
	int     k, r, c, nr = image.rows, nc = image.cols;
	double  D = nr*nr + nc*nc;
	double  d;
	int     val;
	for (c = 0; c < len; ++c)
		histo[c] = 0;

	for (r = 0; r < nr; ++r)
		for (c = 0; c < nc; ++c)
		{
			val = (int)(image.at<uchar>(r, c));
			k = val / undersample;
			d = (r - x)*(r - x) + (c - y)*(c - y);
			if (d > 0)
			{
				d = D/d; 
				//d = sqrt(d);
				//d = pow(D / d, 2.0);
				histo[k] += d;
			}
		}

	double A = 0;

	for (c = 0; c < len; ++c)
		A += histo[c];
	for (c = 0; c < len; ++c)
		histo[c] /= A;

	return histo;
}


void
ComputePercentileHisto(double* histo, int length,
					double percentage, double* out)
{
	double     v, tmp, tmp1, sum = 0.0;
	int        i;

	for (i = 0; i < length; i++)
		sum += histo[i];

	sum *= (percentage / 100.0);

	/* compute percentile */
	i = -1;
	tmp = 0.0;
	while (tmp <= sum)
	{
		i++;
		v = histo[i];
		tmp += v;
	}

	tmp1 = tmp - histo[i];

	*out = (tmp - sum) < (sum - tmp1) ? i : i - 1;

	return;
}

Mat
GLOBALQBRIX(Mat image, double quantile)
{
	int     nr = image.rows;
	int     nc = image.cols;
	int     r, c;
	double* histo = ComputeHistogram(image, 1);
	double  qval, v;
	ComputePercentileHisto(histo, 256, quantile, &qval);

	Mat     retinexIm = Mat::zeros(nr, nc, CV_8UC3);
	

	for (r = 0; r < nr; ++r)
		for (c = 0; c < nc; ++c)
		{
			v = (double)(image.at<uchar>(r, c));
			if (v > 0)
			{
				if (v > qval)
					v = 255;
				else
					v = v / qval * 255;

				retinexIm.at<uchar>(r, c) = (uchar)v;
			}
		}

	return retinexIm;

}


Mat
LOCALQBRIX(Mat image, double alpha, double quantile)
{
	int     nr = image.rows, nc = image.cols;
	int     r, c;
	double* histo = new double[256];
	Mat     retinexIm;
	double  qval, v;

	retinexIm = Mat::zeros(nr, nc, CV_8UC1);

	for (r = 0; r < nr; ++r)
	{
		for (c = 0; c < nc; ++c)
			if (image.at<uchar>(r, c) > 0)
			{
				qval = 0;
				histo = ComputeHistogramW(image, r, c, alpha, 1);
				ComputePercentileHisto(histo, 256, quantile, &qval);
	
		
				v = (double)image.at<uchar>(r, c);

				if (v > qval)
				{
					retinexIm.at<uchar>(r, c) = 255;
				}
				else
				{
					v = 255 * v / qval;
					retinexIm.at<uchar>(r, c) = (uchar)(v);
				}

			}

	}

	return retinexIm;
}





int
main(int argc, char* argv[])
{
	Mat             image = imread(argv[1]); //, CV_LOAD_IMAGE_ANYDEPTH);
	double          quantile = atof(argv[2]); // between 0 and 100
	double          wdistance = atof(argv[3]); // if wdistance < 0 --> global qbrix
	int             pargc = 4;

#ifdef DEBUG
	namedWindow("image", CV_WINDOW_AUTOSIZE);
	imshow("image", image);
	waitKey(0);
#endif


#ifdef SAVE
	char*           outdir = argv[pargc];
	string          nn = argv[1];
	int             dim = nn.rfind("/");
	int             size = nn.rfind(".");
	dim++;

	string          outn;
	outn.assign(nn, dim, size - dim);

	char            outf[20000];
	sprintf_s(outf, "%s/%s.png", outdir, outn.c_str());
	cout << " output in " << outf << endl;
	pargc++;
#endif

	int              nr = image.rows, nc = image.cols;


	int       i, r, c;
	vector<Mat> channel(3);
	split(image, channel);

	Mat      rchannel[3];


	if (wdistance > 0)
	{
		for (i = 0; i < 3; i++)
		{
			cout << " local QBRIX; processing channel " << i << endl;
			rchannel[i] = LOCALQBRIX(channel[i], wdistance, quantile);
		}
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			cout << " global QBRIX,  channel " << i << " with quantile = " << quantile << endl;
			rchannel[i] = GLOBALQBRIX(channel[i], quantile);
		}
	}

	Mat qbrixIm;
	qbrixIm.create(nr, nc, CV_8UC3);
	for (i = 0; i < 3; i++)
	{
		for (r = 0; r < nr; ++r)
			for (c = 0; c < nc; ++c)
			{
				qbrixIm.at<Vec3b>(r, c)[i] = rchannel[i].at<uchar>(r, c);
			}
	}

#ifdef DEBUG
	namedWindow("qbrix (color)", CV_WINDOW_AUTOSIZE);
	imshow("qbrix (color)", qbrixIm);
	waitKey(0);
#endif

#ifdef SAVE
	imwrite(outf, qbrixIm);
#endif

	return 0;

}
