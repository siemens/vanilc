// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

#include "vanilcDistributionFunction.h"
#include "vanilcStructuringElement.h"

namespace vanilc {

using namespace std;
using namespace cv;

class SparseDistributionFunction : public DistributionFunction {
public:
	SparseDistributionFunction(DistributionFunction* basicDist, Mat* image, unsigned int maxval, bool independentSlices, StructuringElement context, double strength) :
		basicDist(basicDist),
		image(image),
		maxval(maxval),
		independentSlices(independentSlices),
		context(context),
		isFirstRow(false),
		contextHistogram(1, maxval + 1, CV_32S),
		contextTestarray(1, maxval + 1, CV_64F),
		longtermHistogram(1, maxval + 1, CV_32S, Scalar_<int>(0)),
		longtermTestarray(1, maxval + 1, CV_64F, Scalar(0.0)),
		strength(strength),
		kernelRadius(maxval / 10 + 1),
		smoothingKernel(getGaussianKernel(2 * kernelRadius + 1, (double)maxval / 20.0).reshape(0, 1)),
		smallestNumberOfPixelsWithSameIntensityInPast(image->total()),
		longtermTestarrayMean(0.0) {};
//			{ if(independentSlices) protectionMap = Mat(image->size[1], image->size[2], CV_8U, Scalar_<unsigned int>(0)); };
	~SparseDistributionFunction() { delete basicDist; };

	void setParameters(Mat parameters, unsigned int cropped = 0);
	double getFactor() { return basicDist->getFactor(); };
	double computeValue(double x);
	SparseDistributionFunction* clone() const { return new SparseDistributionFunction(*this); }; // "covariant return type" for "virtual copy constructor"

private:
	DistributionFunction* basicDist; // distribution to be sparsified
	Mat* image;
	unsigned int maxval;
	bool independentSlices; // for color images reset longterm histogram after each color channel
	StructuringElement context;
	bool isFirstRow;
	Mat contextHistogram; // histogram for only the context region
	Mat contextTestarray;
	Mat longtermHistogram; // histogram for all causal pixels up to the current position
	Mat longtermTestarray;
	double strength; // defines how aggressive distribution is sparsified
	unsigned int kernelRadius;
	Mat smoothingKernel;
	int smallestNumberOfPixelsWithSameIntensityInPast;
	double longtermTestarrayMean;
	vector<unsigned int> startsOfProbableValueRanges;
	vector<unsigned int> endsOfProbableValueRanges;
	vector<double> basicDistProbsAtStarts;
	vector<double> sparsifiedDistProbsAtEnds;
//	Mat protectionMap;
};

} // end namespace vanilc

