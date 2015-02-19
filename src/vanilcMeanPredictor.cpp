// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcMeanPredictor.h"

namespace vanilc {

double MeanPredictionComputer::compute(const Point3i& currentPos, Context* context) {
	double prediction;
	if(context->getNeighborhood().getNumberOfElements() > 1) { // not the first pixel
		Mat neighbors;
		context->contextOf(currentPos, neighbors);
		prediction = mean(neighbors.colRange(0, neighbors.cols - 1))[0];
	} else prediction = (1.0 + predictor->getMaxval()) / 2.0; // first pixel of image
	return prediction;
} // end MeanPredictionComputer::compute

} // end namespace vanilc

