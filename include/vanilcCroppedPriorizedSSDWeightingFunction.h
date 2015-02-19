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

class CroppedPriorizedSSDWeightingFunction : public WeightingFunction {
public:
//	CroppedPriorizedSSDWeightingFunction() {};
	CroppedPriorizedSSDWeightingFunction(const Mat& neighborhoodPriorization, unsigned int numWeights) :
		neighborhoodPriorization(neighborhoodPriorization),
		neighborhoodPriorizationPtr(neighborhoodPriorization.ptr<double>()),
		bestDistances(1, numWeights, CV_64F, numeric_limits<double>::infinity()) {};
	CroppedPriorizedSSDWeightingFunction(const Point3i& referenceSpatialPoint, const Mat& neighborhoodPriorization, unsigned int numWeights) :
		WeightingFunction(referenceSpatialPoint),
		neighborhoodPriorization(neighborhoodPriorization),
		neighborhoodPriorizationPtr(neighborhoodPriorization.ptr<double>()),
		bestDistances(1, numWeights, CV_64F, numeric_limits<double>::infinity()) {};
	CroppedPriorizedSSDWeightingFunction(const Mat& referenceRegressionPoint, const Mat& neighborhoodPriorization, unsigned int numWeights) :
		WeightingFunction(referenceRegressionPoint),
		neighborhoodPriorization(neighborhoodPriorization),
		neighborhoodPriorizationPtr(neighborhoodPriorization.ptr<double>()),
		bestDistances(1, numWeights, CV_64F, numeric_limits<double>::infinity()) {};
	CroppedPriorizedSSDWeightingFunction(const Point3i& referenceSpatialPoint, const Mat& referenceRegressionPoint, const Mat& neighborhoodPriorization, unsigned int numWeights) :
		WeightingFunction(referenceSpatialPoint, referenceRegressionPoint),
		neighborhoodPriorization(neighborhoodPriorization),
		neighborhoodPriorizationPtr(neighborhoodPriorization.ptr<double>()),
		bestDistances(1, numWeights, CV_64F, numeric_limits<double>::infinity()) {};

	CroppedPriorizedSSDWeightingFunction* clone() const { return new CroppedPriorizedSSDWeightingFunction(*this); }; // "covariant return type" for "virtual copy constructor"

	void setReferencePoint(const Mat& referenceRegressionPoint) {
		bestDistances = Scalar(numeric_limits<double>::infinity());
		WeightingFunction::setReferencePoint(referenceRegressionPoint);
	};
	void setReferencePoint(const Point3i& referenceSpatialPoint, const Mat& referenceRegressionPoint) {
		bestDistances = Scalar(numeric_limits<double>::infinity());
		WeightingFunction::setReferencePoint(referenceSpatialPoint, referenceRegressionPoint);
	}

	double computeWeight(const Point3i& spatialPoint);
	double computeWeight(const Mat& regressionPoint); // row vector - if regressionPoint is larger than referenceRegressionPoint, protruding elements are ignored
	double computeWeight(const Point3i& spatialPoint, const Mat& regressionPoint);

private:
	const Mat neighborhoodPriorization;
	const double* const neighborhoodPriorizationPtr;
	Mat bestDistances;
};

} // end namespace vanilc

