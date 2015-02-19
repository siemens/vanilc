// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcNLMPredictor.h"

namespace vanilc {

double NLMPredictionComputer::compute(const Point3i& currentPos, Context* context) {
	double prediction = 0.0;
	Mat sampleVector;
	context->contextOf(currentPos, sampleVector); // get current neighborhood and store it in sampleVector
	if(context->getTrainingregion().getNumberOfElements()) {
		sampleVector = sampleVector.colRange(0, sampleVector.cols - 1); // remove last (current) pixel
		weightingFunction->setReferencePoint(sampleVector); // set as reference for block matching to compute weights
		double weight, sumOfWeights = 0.0;
		context->getContextElementsOf(currentPos);
		while(!context->getNextContextElement(sampleVector)) {
			sumOfWeights += (weight = weightingFunction->computeWeight(sampleVector));
			prediction += sampleVector.at<double>(0, sampleVector.cols - 1) * weight;
		}
		prediction /= sumOfWeights;
		prediction = (prediction < 0.0 ? 0.0 : (prediction > predictor->getMaxval() ? predictor->getMaxval() : prediction)); // crop to valid value range
	} else { // first 4 pixels / 8 voxels in image
		if(context->getNeighborhood().getNumberOfElements() > 1) { // not the first pixel
			prediction = mean(sampleVector.colRange(0, sampleVector.cols - 1))[0];
		} else { // first image pixel
			prediction = (1.0 + predictor->getMaxval()) / 2.0;
		}
	}
	return prediction;
} // end NLMPredictionComputer::compute

} // end namespace vanilc

