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

#include "vanilcWeightingFunction.h"
#include "vanilcStructuringElement.h"

namespace vanilc {

using namespace std;
using namespace cv;

class InversePriorizedSSDWeightingFunction : public WeightingFunction {
public:
	InversePriorizedSSDWeightingFunction(const Mat& neighborhoodPriorization) :
		neighborhoodPriorization(neighborhoodPriorization),
		neighborhoodPriorizationPtr(neighborhoodPriorization.ptr<double>()) {};
	InversePriorizedSSDWeightingFunction(const Point3i& referenceSpatialPoint, const Mat& neighborhoodPriorization) :
		WeightingFunction(referenceSpatialPoint),
		neighborhoodPriorization(neighborhoodPriorization),
		neighborhoodPriorizationPtr(neighborhoodPriorization.ptr<double>()) {};
	InversePriorizedSSDWeightingFunction(const Mat& referenceRegressionPoint, const Mat& neighborhoodPriorization) :
		WeightingFunction(referenceRegressionPoint),
		neighborhoodPriorization(neighborhoodPriorization),
		neighborhoodPriorizationPtr(neighborhoodPriorization.ptr<double>()) {};
	InversePriorizedSSDWeightingFunction(const Point3i& referenceSpatialPoint, const Mat& referenceRegressionPoint, const Mat& neighborhoodPriorization) :
		WeightingFunction(referenceSpatialPoint, referenceRegressionPoint),
		neighborhoodPriorization(neighborhoodPriorization),
		neighborhoodPriorizationPtr(neighborhoodPriorization.ptr<double>()) {};

	InversePriorizedSSDWeightingFunction* clone() const { return new InversePriorizedSSDWeightingFunction(*this); }; // "covariant return type" for "virtual copy constructor"

	static Mat constructInverseEuclideanPriorization(const StructuringElement& neighborhood);

	void setReferencePoint(const Mat& referenceRegressionPoint) {
		WeightingFunction::setReferencePoint(referenceRegressionPoint);
		double currentIntensity = referenceRegressionPoint.at<double>(referenceRegressionPoint.cols - 1);
		currentIntensity = currentIntensity * currentIntensity;
		if(currentIntensity > maxval) maxval = currentIntensity;
	}

	// for efficiency store squared maxval; true maximum is somewhere between maxval/2 and maxval: start with maxval/2 and increase upon observation
	void setMaxval(unsigned int maxval) { this->maxval = (double)maxval * (double)maxval / 4; };

	double computeWeight(const Mat& regressionPoint); // row vector - if regressionPoint is larger than referenceRegressionPoint, protruding elements are ignored

private:
	const Mat neighborhoodPriorization;
	const double* const neighborhoodPriorizationPtr;
};

} // end namespace vanilc

