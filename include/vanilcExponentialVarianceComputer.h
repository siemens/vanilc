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

class ExponentialVarianceComputer : public Computer {
public:
	ExponentialVarianceComputer() : variance(0.0) {};

	double compute(const Point3i& currentPos, Context* context) {
		if(context->getNeighborhood().getLeft()) {
//			Mat sampleVector;
//			context->contextOf(currentPos, sampleVector); // get current 3-pixel neighborhood and store it in sampleVector
//			double previousImageValue = sampleVector.at<double>(sampleVector.cols - 2);
			double previousImageValue = context->getImage()->at<double>(currentPos.z, currentPos.y, currentPos.x - 1);
			variance = variance * 0.8 + (previousImageValue - previousPrediction) * (previousImageValue - previousPrediction) * 0.2;
		} else if(variance == 0.0) variance = predictor->getMaxval() * predictor->getMaxval() / 4.0;
		previousPrediction = predictor->getPrediction();
		return variance;
	};

private:
	double variance;
	double previousPrediction;
};

} // end namespace vanilc

