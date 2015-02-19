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

#include "vanilcLSPredictor.h"

namespace vanilc {

using namespace std;
using namespace cv;

class FastLSPredictionComputer : public LSPredictionComputer {
public:
	FastLSPredictionComputer(Mat* covMat, Mat* coefficients, Mat* weights, double border_regularization, double inner_regularization, int solver) :
		LSPredictionComputer(covMat, coefficients, weights, IdentityWeightingFunction(), border_regularization, inner_regularization, 0, solver, 0) {};
	void init();

	// do only set the image when its memory has already been allocated! (otherwise the buffer is going to be empty, producing an error)
	void setImage(Mat* image, unsigned int maxval);

private:
	void estimate(const Point3i& currentPos);
	Mat getBuffer(const Point3i& currentPos);

	Mat covMatBuffer;
	Mat zeroBuffer; // covariance matrix with only zeros
	unsigned int currentSlice, currentRow; // necessary for ringbuffer to save memory
};

} // end namespace vanilc

