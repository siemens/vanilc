// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcSparseDistributionFunction.h"

namespace vanilc {

void SparseDistributionFunction::setParameters(Mat parameters, unsigned int cropped) {
	unsigned int j = (unsigned int)(parameters.at<double>(parameters.rows - 3, 0));
	unsigned int k = (unsigned int)(parameters.at<double>(parameters.rows - 2, 0));
	unsigned int l = (unsigned int)(parameters.at<double>(parameters.rows - 1, 0));
	Point3i previousPosition; unsigned int previousImageIntensity = maxval / 2;
	if(l) previousPosition = Point3i(l - 1, k, j);
	else if(k) previousPosition = Point3i(image->size[2] - 1, k - 1, j);
	else previousPosition = Point3i(image->size[2] - 1, image->size[1] - 1, j - 1);
	if(previousPosition.z > -1) previousImageIntensity = (unsigned int)(image->at<double>(previousPosition.z, previousPosition.y, previousPosition.x));
//	if(k > 0 && computeValue((double)previousImageIntensity + .5) - computeValue((double)previousImageIntensity - .5) < 1e-14)
//		protectionMap.at<unsigned int>(previousPosition.y, previousPosition.x) = 1;
//	if(k == image->size[1] - 1 && l == image->size[2] - 1) cout << "[" << countNonZero(protectionMap) << "]";
	basicDist->setParameters(parameters, cropped);
	// longterm histogram creation and unprobable value detection
	if(independentSlices && k == 0 && l == 0) {
		longtermHistogram = Scalar_<int>(0);
		longtermTestarray = Scalar(0.0);
		smallestNumberOfPixelsWithSameIntensityInPast = image->total();
		longtermTestarrayMean = 0.0;
	}
	int& previousIntensityFrequency = longtermHistogram.at<int>(0, previousImageIntensity);
	double increment = previousIntensityFrequency ? -1.0 / (previousIntensityFrequency * (previousIntensityFrequency + 1)) : 1.0;
	++previousIntensityFrequency;
	longtermTestarrayMean += increment / (double)(maxval + 1);
	longtermTestarray.colRange(previousImageIntensity >= kernelRadius ? previousImageIntensity - kernelRadius : 0,
		previousImageIntensity <= maxval - kernelRadius ? previousImageIntensity + kernelRadius + 1 : maxval + 1) += increment *
		smoothingKernel.colRange(previousImageIntensity >= kernelRadius ? 0 : kernelRadius - previousImageIntensity,
		previousImageIntensity <= maxval - kernelRadius ? 2 * kernelRadius + 1 : kernelRadius + 1 + maxval - previousImageIntensity);
	// find minimum of longterm histogram (except for zeros)
	if(previousIntensityFrequency == 1) smallestNumberOfPixelsWithSameIntensityInPast = 1;
	else if(previousIntensityFrequency == smallestNumberOfPixelsWithSameIntensityInPast + 1) {
		++smallestNumberOfPixelsWithSameIntensityInPast;
		int* longtermHistogramPtr = longtermHistogram.ptr<int>();
		for(unsigned int i = 0; i <= maxval; ++i)
			if(*(longtermHistogramPtr++) == previousIntensityFrequency - 1) { --smallestNumberOfPixelsWithSameIntensityInPast; break; }
	}
	if(k == 0 && (independentSlices || j == 0)) { isFirstRow = true; return; } // don't sparsify in first image row
	isFirstRow = false;
	// create context histogram, find mean and minimum (except for zeros), and fill contextTestarray for unprobable value detection algorithm
	context.computeHistogramFromImageBorderSafe(*image, Point3i(l, k, j), contextHistogram);
	int* contextHistogramPtr = contextHistogram.ptr<int>() - 1; double* contextTestarrayPtr = contextTestarray.ptr<double>();
	int smallestNumberOfPixelsWithSameIntensityInContext = image->total(); double contextTestarrayMean = 0.0;
	for(unsigned int i = 0; i <= maxval; ++i)
		if(*(++contextHistogramPtr)) {
			if(*contextHistogramPtr < smallestNumberOfPixelsWithSameIntensityInContext) smallestNumberOfPixelsWithSameIntensityInContext = *contextHistogramPtr;
			contextTestarrayMean += *(contextTestarrayPtr++) = 1.0 / (double)*(contextHistogramPtr);
		} else *(contextTestarrayPtr++) = 0.0;
	contextTestarrayMean /= (double)(maxval + 1);
	sepFilter2D(contextTestarray, contextTestarray, -1, smoothingKernel, Mat(1, 1, CV_64F, Scalar(1.0)));
	// compute distribution irregularities
	startsOfProbableValueRanges.clear(); endsOfProbableValueRanges.clear();
	basicDistProbsAtStarts.clear(); sparsifiedDistProbsAtEnds.clear();
	bool isUnprobable, probableValueRun = false; double currentProb = 0.0;
	contextHistogramPtr = contextHistogram.ptr<int>(); contextTestarrayPtr = contextTestarray.ptr<double>();
	int* longtermHistogramPtr = longtermHistogram.ptr<int>(); double* longtermTestarrayPtr = longtermTestarray.ptr<double>();
	for(unsigned int i = 0; i <= maxval; ++i) {
		isUnprobable = !*contextHistogramPtr && *contextTestarrayPtr < contextTestarrayMean * smallestNumberOfPixelsWithSameIntensityInContext * strength
			|| !*longtermHistogramPtr && *longtermTestarrayPtr < longtermTestarrayMean * smallestNumberOfPixelsWithSameIntensityInPast;
		if(probableValueRun) { if(isUnprobable) {
			probableValueRun = false;
			endsOfProbableValueRanges.push_back(i);
			sparsifiedDistProbsAtEnds.push_back(currentProb += basicDist->computeValue((double)i - .5) - basicDistProbsAtStarts.back());
		}} else if(!isUnprobable) {
			probableValueRun = true;
			startsOfProbableValueRanges.push_back(i);
			basicDistProbsAtStarts.push_back(basicDist->computeValue((double)i - .5));
		}
		++contextHistogramPtr; ++contextTestarrayPtr; ++longtermHistogramPtr; ++longtermTestarrayPtr;
	}
	if(probableValueRun) { // finalize
		endsOfProbableValueRanges.push_back(maxval + 1);
		sparsifiedDistProbsAtEnds.push_back(currentProb += basicDist->computeValue((double)maxval + .5) - basicDistProbsAtStarts.back());
	}
} // end SparseDistributionFunction::setParameters

double SparseDistributionFunction::computeValue(double x) {
	if(isFirstRow) return basicDist->computeValue(x);
	unsigned int intx = (unsigned int)(x + .5);
	if(sparsifiedDistProbsAtEnds.back() < 1e-14) { // assume uniform distribution upon wrong predictions
		unsigned int numberOfProbableElements = 0, indexInProbableElements = 0; // uniform distribution among probable values
		for(unsigned int i = 0; i < startsOfProbableValueRanges.size(); ++i) {
			if(intx >= startsOfProbableValueRanges[i] && intx < endsOfProbableValueRanges[i])
				indexInProbableElements = numberOfProbableElements + intx - startsOfProbableValueRanges[i];
			numberOfProbableElements += endsOfProbableValueRanges[i] - startsOfProbableValueRanges[i];
			if(intx >= endsOfProbableValueRanges[i]) indexInProbableElements = numberOfProbableElements;
		}
		return basicDist->getFactor() / (double)numberOfProbableElements * indexInProbableElements;
//		return basicDist->getFactor() / ((double)maxval + 1.0) * (x + .5); // uniform distribution among all values
	}
	unsigned int* front = &endsOfProbableValueRanges.front(); // get C-array
	unsigned int index = upper_bound(front, &endsOfProbableValueRanges.back() + 1, intx) - front; // found position in endsOfProbableValueRanges
	if(index == endsOfProbableValueRanges.size() || intx < startsOfProbableValueRanges[index]) // unprobable value
		return index ? sparsifiedDistProbsAtEnds[index - 1] / sparsifiedDistProbsAtEnds.back() * basicDist->getFactor() : 0.0;
	return ((index ? sparsifiedDistProbsAtEnds[index - 1] : 0.0) + (basicDist->computeValue(x) - basicDistProbsAtStarts[index])) / sparsifiedDistProbsAtEnds.back() * basicDist->getFactor();
} // end SparseDistributionFunction::computeValue

} // end namespace vanilc

