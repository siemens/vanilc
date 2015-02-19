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

namespace vanilc {

using namespace std;
using namespace cv;

class ResidualVarianceComputer : public Computer {
public:
	ResidualVarianceComputer(double radius) : estimationRegion(StructuringElement::createHalfEllipseElement(radius, radius, radius, false)) {};
	ResidualVarianceComputer(StructuringElement str) : estimationRegion(str) {};
	void init() { squaredResidualImage = predictor->getContext().getImage()->clone(); };

	double compute(const Point3i& currentPos, Context* context) {
		Point3i previousPos = currentPos;
		if(previousPos.x) --previousPos.x;
		else if(previousPos.y) { --previousPos.y; previousPos.x = squaredResidualImage.size[2] - 1; }
		else if(previousPos.z) { --previousPos.z; previousPos.y = squaredResidualImage.size[1] - 1; previousPos.x = squaredResidualImage.size[2] - 1; }
		else { previousPrediction = predictor->getPrediction(); return predictor->getMaxval() * predictor->getMaxval() / 4.0; }
		previousPrediction -= context->getImage()->at<double>(previousPos.z, previousPos.y, previousPos.x); // negative residual
		squaredResidualImage.at<double>(previousPos.z, previousPos.y, previousPos.x) = previousPrediction * previousPrediction;
		previousPrediction = predictor->getPrediction();
		estimationRegion.extractVectorFromImageBorderSafe(squaredResidualImage, currentPos, sampleVector);
//		if(sampleVector.cols > 1) return sum(sampleVector)[0] / (sampleVector.cols - 1); // sample variance: not applicable (prediction error is not the same as a residual!)
//		else return predictor->getMaxval() * predictor->getMaxval() / 16.0;
		return mean(sampleVector)[0];
	};

private:
	double previousPrediction;
	Mat squaredResidualImage;
	StructuringElement estimationRegion;
	Mat sampleVector;
};

} // end namespace vanilc

