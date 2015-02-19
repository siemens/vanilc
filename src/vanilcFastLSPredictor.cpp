// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcFastLSPredictor.h"

namespace vanilc {

void FastLSPredictionComputer::init() {
	int sz[] = {predictor->getContext().getImage()->size[0] - predictor->getContext().getFullNeighborhood().getMask().size[0] + 1, // covMatBuffer size
		predictor->getContext().getImage()->size[1] - predictor->getContext().getFullNeighborhood().getMask().size[1] + 1,
		predictor->getContext().getImage()->size[2] - predictor->getContext().getFullNeighborhood().getMask().size[2] + 1,
		predictor->getContext().getFullNeighborhood().getNumberOfElements(), predictor->getContext().getFullNeighborhood().getNumberOfElements()};
	if(predictor->getContext().getFullTrainingregion().getFront()) {
		if(predictor->getContext().getFullTrainingregion().getFront() + 2 <= (unsigned int)sz[0]) // ringbuffered slices
			sz[0] = predictor->getContext().getFullTrainingregion().getFront() + 2;
	} else {sz[0] = 1;
		if(predictor->getContext().getFullTrainingregion().getTop() + 2 <= (unsigned int)sz[1]) // ringbuffered rows
			sz[1] = predictor->getContext().getFullTrainingregion().getTop() + 2;
	}
	covMatBuffer = Mat(5, sz, CV_64F, numeric_limits<double>::quiet_NaN());
	zeroBuffer = Mat(predictor->getContext().getFullNeighborhood().getNumberOfElements(), predictor->getContext().getFullNeighborhood().getNumberOfElements(), CV_64F, Scalar(0.0));
	currentSlice = predictor->getContext().getFullTrainingregion().getFront() + 1;
	currentRow = predictor->getContext().getFullTrainingregion().getTop() + 1;
} // end FastLSPredictionComputer::init

// estimate covariance matrix
void FastLSPredictionComputer::estimate(const Point3i& currentPos) {
	if(context->getNeighborhood().getMask().total() != context->getFullNeighborhood().getMask().total())
		LSPredictionComputer::estimate(currentPos); // if full neighborhood not yet available at border regions, for simplicity use WLS implementation
	else {
		Mat sampleVector;
		context->contextOf(currentPos, sampleVector); // get current neighborhood and store it in sampleVector
		covMat->create(context->getFullNeighborhood().getNumberOfElements(), context->getFullNeighborhood().getNumberOfElements() + 1, CV_64F); // one more row for later variance estimation!
		sampleVector.reshape(0, sampleVector.cols).copyTo(covMat->col(covMat->cols - 1)); // put neighborhood in last column for variance estimate
		int left = context->getTrainingregion().getLeft(), right = context->getTrainingregion().getRight(), top = context->getTrainingregion().getTop();
		(*covMat)(Rect(0, 0, sampleVector.cols, sampleVector.cols))
			= getBuffer(currentPos + Point3i(     -1,      0, 0))
			- getBuffer(currentPos + Point3i(     -1,     -1, 0))
			+ getBuffer(currentPos + Point3i(  right,     -1, 0))
			- getBuffer(currentPos + Point3i(-1-left,      0, 0))
			- getBuffer(currentPos + Point3i(  right, -1-top, 0))
			+ getBuffer(currentPos + Point3i(-1-left, -1-top, 0));
		if(context->getTrainingregion().getFront()) { // 3-D training region
			int bottom = context->getTrainingregion().getBottom(), front = context->getTrainingregion().getFront();
			(*covMat)(Rect(0, 0, sampleVector.cols, sampleVector.cols)) +=
				- getBuffer(currentPos + Point3i(     -1,      0, -1      ))
				+ getBuffer(currentPos + Point3i(     -1,     -1, -1      ))
				- getBuffer(currentPos + Point3i(  right,     -1, -1      ))
				+ getBuffer(currentPos + Point3i(-1-left,      0, -1      ))
				+ getBuffer(currentPos + Point3i(  right, bottom, -1      ))
				- getBuffer(currentPos + Point3i(-1-left, bottom, -1      ))
				- getBuffer(currentPos + Point3i(  right, bottom, -1-front))
				+ getBuffer(currentPos + Point3i(-1-left, bottom, -1-front))
				+ getBuffer(currentPos + Point3i(  right, -1-top, -1-front))
				- getBuffer(currentPos + Point3i(-1-left, -1-top, -1-front));
		}
		*covMat = covMat->rowRange(0, covMat->rows - 1); // make last row invisible for computePrediction function of WLS
//		context->getContextElementsOf(currentPos); // only necessary if computeVariance method from parent class WLS is used
	}
	*weights = weights->colRange(0, 0); // set used region
} // end FastLSPredictionComputer::estimate

Mat FastLSPredictionComputer::getBuffer(const Point3i& currentPos) {
	int pos[] = {currentPos.z - context->getFullNeighborhood().getFront(), currentPos.y - context->getFullNeighborhood().getTop(),
		currentPos.x - context->getFullNeighborhood().getLeft(), 0, 0}; // buffer position
	if(pos[0] < 0 || pos[1] < 0 || pos[2] < 0 || pos[2] >= covMatBuffer.size[2] ||
		pos[1] >= context->getImage()->size[1] - context->getFullNeighborhood().getMask().size[1] + 1)
			return zeroBuffer; // outside the buffer return zero matrix
	if(pos[0] >= covMatBuffer.size[0]) { // slice ringbuffer is active
		if(pos[0] > (int)currentSlice) { // rotate ringbuffer
			pos[0] %= covMatBuffer.size[0]; // ringbuffer position
			int startSlice[] = {pos[0], 0, 0, 0, 0};
			for(double *nanPtr = &(covMatBuffer.at<double>(startSlice)),
				*endPtr = nanPtr + covMatBuffer.size[1] * covMatBuffer.size[2] * covMatBuffer.size[3] * covMatBuffer.size[4];
				nanPtr < endPtr; nanPtr += covMatBuffer.size[3] * covMatBuffer.size[4])
					*nanPtr = numeric_limits<double>::quiet_NaN(); // set upper left matrix values to nan
			++currentSlice;
			currentRow = context->getTrainingregion().getTop() + 1;
		} else pos[0] %= covMatBuffer.size[0];
	}
	if(!context->getTrainingregion().getFront()) { // row ringbuffer is active
		if(pos[1] > (int)currentRow) { // rotate ringbuffer
			pos[1] %= covMatBuffer.size[1]; // ringbuffer position
			int startRow[] = {0, pos[1], 0, 0, 0};
			for(double *nanPtr = &(covMatBuffer.at<double>(startRow)),
				*endPtr = nanPtr + covMatBuffer.size[2] * covMatBuffer.size[3] * covMatBuffer.size[4];
				nanPtr < endPtr; nanPtr += covMatBuffer.size[3] * covMatBuffer.size[4])
					*nanPtr = numeric_limits<double>::quiet_NaN(); // set upper left matrix values to nan
			++currentRow;
		} else pos[1] %= covMatBuffer.size[1];
	}
	double* bufPtr = &(covMatBuffer.at<double>(pos));
	#ifdef WIN32
	if(_isnan(*bufPtr)) {
	#else
	if(isnan(*bufPtr)) {
	#endif
		double* currentBufPtr = bufPtr;
		const double*    leftBufPtr = getBuffer(currentPos + Point3i(-1,  0, 0)).ptr<double>();
		const double*     topBufPtr = getBuffer(currentPos + Point3i( 0, -1, 0)).ptr<double>();
		const double* topleftBufPtr = getBuffer(currentPos + Point3i(-1, -1, 0)).ptr<double>();
		Mat sampleVector;
		context->contextOf(currentPos, sampleVector);
		const double* const sampleVectorPtr = sampleVector.ptr<double>();
		for(int k = 0; k < sampleVector.cols; ++k) // matrix is continuous!
			for(int l = 0; l < sampleVector.cols; ++l)
				*(currentBufPtr++) = sampleVectorPtr[k] * sampleVectorPtr[l] + *(topBufPtr++) + *(leftBufPtr++) - *(topleftBufPtr++);
		if(context->getTrainingregion().getFront()) { // 3-D training region
			const double*        frontBufPtr = getBuffer(currentPos + Point3i( 0,  0, -1)).ptr<double>();
			const double*    frontleftBufPtr = getBuffer(currentPos + Point3i(-1,  0, -1)).ptr<double>();
			const double*     fronttopBufPtr = getBuffer(currentPos + Point3i( 0, -1, -1)).ptr<double>();
			const double* fronttopleftBufPtr = getBuffer(currentPos + Point3i(-1, -1, -1)).ptr<double>();
			currentBufPtr = bufPtr;
			for(int k = 0; k < sampleVector.cols; ++k)
				for(int l = 0; l < sampleVector.cols; ++l)
					*(currentBufPtr++) += *(frontBufPtr++) - *(frontleftBufPtr++) - *(fronttopBufPtr++) + *(fronttopleftBufPtr++);
		}
	}
	return Mat(context->getFullNeighborhood().getNumberOfElements(), context->getFullNeighborhood().getNumberOfElements(), CV_64F, bufPtr);
} // end FastLSPredictionComputer::updateBuffer

} // end namespace vanilc

