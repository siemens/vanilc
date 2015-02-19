// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcInversePriorizedSSDWeightingFunction.h"

namespace vanilc {

Mat InversePriorizedSSDWeightingFunction::constructInverseEuclideanPriorization(const StructuringElement& neighborhood) {
	int sz[] = { neighborhood.getMask().size[0], neighborhood.getMask().size[1], neighborhood.getMask().size[2] };
	Mat priorization(3, sz, CV_64F);
	for(int j = 0; j < (int)neighborhood.getSlcs(); ++j)
		for(int k = 0; k < (int)neighborhood.getRows(); ++k)
			for(int l = 0; l < (int)neighborhood.getCols(); ++l)
				priorization.at<double>(j, k, l) = 1.0 /
					sqrt((double)( (j - neighborhood.getFront()) * (j - neighborhood.getFront())
					+ (k - neighborhood.getTop()) * (k - neighborhood.getTop())
					+ (l - neighborhood.getLeft()) * (l - neighborhood.getLeft()) ));
	priorization = neighborhood.extractVectorFromPatch(priorization).colRange(0, neighborhood.getNumberOfElements() - 1);
	return priorization / norm(priorization, NORM_L1);
} // end InversePriorizedSSDWeightingFunction::constructInverseEuclideanPriorization

double InversePriorizedSSDWeightingFunction::computeWeight(const Mat& regressionPoint) {
	double distance, result = 0.0, meanRefValue = 0.0, varRefValue = 0.0, meanDstValue = 0.0;
	const double* regressionPointPtr = regressionPoint.ptr<double>();
	for(int l = 0; l < referenceRegressionPoint.cols; ++l) {
		distance = (referenceRegressionPointPtr[l] - *(regressionPointPtr++)) * neighborhoodPriorizationPtr[l];
		result += distance * distance;
	}
	return maxval / (maxval + result);
} // end InversePriorizedSSDWeightingFunction::computeWeight

} // end namespace vanilc

