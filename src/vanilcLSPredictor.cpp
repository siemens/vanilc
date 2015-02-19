// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcLSPredictor.h"

namespace vanilc {

double LSPredictionComputer::compute(const Point3i& currentPos, Context* context) {
	this->context = context;
	if(!context->getTrainingregion().getNumberOfElements()) { // first 4 pixels / 8 voxels in image
		context->contextOf(currentPos, *coefficients); // misuse coefficients vector to store neighbors for variance computation
		if(coefficients->cols > 1) // not the first pixel
			return coefficients->at<double>(coefficients->cols - 1) = mean(coefficients->colRange(0, coefficients->cols - 1))[0];
		else return (1.0 + predictor->getMaxval()) / 2.0; // first pixel of image
	}
	estimate(currentPos);
	coefficients->create(covMat->rows + 1, 2, CV_64F); // reset to maximum size (should not need memory re-allocation)
	*coefficients = coefficients->rowRange(0, covMat->rows); // set used region
	if(border_regularization != 0.0) { // Tikhonov regularization for border and for inner pixels
		if(context->isBorder()) { // border
			covMat->diag() += Scalar(border_regularization);
			solveSystem();
			covMat->col(covMat->cols - 2) += coefficients->col(0) * border_regularization; // correct RHS for variance estimation
		} else if(inner_regularization != 0.0) { // inner
			covMat->diag() += Scalar(inner_regularization);
			solveSystem();
			covMat->col(covMat->cols - 2) += coefficients->col(0) * inner_regularization; // correct RHS for variance estimation
		} else solveSystem();
	} else solveSystem();
	double prediction = covMat->col(covMat->cols - 1).dot(coefficients->col(0)); // linear prediction using dot product
	return (prediction < 0.0 ? 0.0 : (prediction > predictor->getMaxval() ? predictor->getMaxval() : prediction)); // crop to valid value range
} // end LSPredictionComputer::compute

// estimate covariance matrix
void LSPredictionComputer::estimate(const Point3i& currentPos) {
	Mat sampleVector, weightedSampleVector;
	context->contextOf(currentPos, sampleVector); // get current neighborhood and store it in sampleVector

	// init covMat
	covMat->create(context->getFullNeighborhood().getNumberOfElements(), context->getFullNeighborhood().getNumberOfElements() + 1, CV_64F);
	*covMat = (*covMat)(Rect(0, 0, sampleVector.cols + 1, sampleVector.cols)); // Rect(x, y, width, height)
	*covMat = Scalar(0.0); // set covariance matrix to zero
	sampleVector.reshape(0, sampleVector.cols).copyTo(covMat->col(covMat->cols - 1)); // put neighborhood in last column for variance estimate
	sampleVector = sampleVector.colRange(0, sampleVector.cols - 1); // remove last (current) pixel

	// other neighborhood is used for matching (weight computation) than for prediction?
	Mat weightingVector;
	if(weightingContext) {
		weightingContext->checkBorder(currentPos);
		if(weightingContext->isBorder()) context->setTrainingregion(weightingContext->getTrainingregion()); // use smaller training region for context
		weightingContext->contextOf(currentPos, weightingVector); // get current matching neighborhood and store it in weightingVector
		weightingVector = weightingVector.colRange(0, weightingVector.cols - 1); // remove last (current) pixel
		otherWeightingFunction->setReferencePoint(weightingVector); // set as reference for block matching to compute weights
		weightingContext->getContextElementsOf(currentPos);
	} else weightingFunction->setReferencePoint(sampleVector); // set as reference for block matching to compute weights

	// init weights
	weights->create(1, context->getFullTrainingregion().getNumberOfElements(), CV_64F); // reset to maximum size (should not need memory re-allocation)
	*weights = weights->colRange(0, context->getTrainingregion().getNumberOfElements()); // set used region
	double* weightsPtr = weights->ptr<double>();

	if(maxTrainingVectors) {
		Mat sampleVectors(maxTrainingVectors, context->getNeighborhood().getNumberOfElements(), CV_64F, Scalar(0.0));
		Mat correspondingWeights(maxTrainingVectors, 1, CV_64F, Scalar(0.0));
		context->getContextElementsOf(currentPos);
		while(!context->getNextContextElement(sampleVector)) {
			if(weightingContext) {
				weightingContext->getNextContextElement(weightingVector);
				*weightsPtr = otherWeightingFunction->computeWeight(weightingVector);
			} else *weightsPtr = weightingFunction->computeWeight(sampleVector); // do weighting for WLS and store weight
			if(*(weightsPtr++)) {
				int index[2];
				minMaxIdx(correspondingWeights, NULL, NULL, index);
				correspondingWeights.at<double>(index[0], 0) = *(weightsPtr - 1);
				sampleVector.copyTo(sampleVectors.row(index[0]));
			}
		}
		double minWeight; minMaxIdx(correspondingWeights, &minWeight, NULL);
		weightsPtr = weights->ptr<double>() - 1;
		for(int i = 0; i < weights->cols; ++i) if(*(++weightsPtr) < minWeight) *weightsPtr = 0; // set small weights to zero
		for(int i = 0; i < maxTrainingVectors; ++i) {
			weightedSampleVector = sampleVectors.row(i) * correspondingWeights.at<double>(i, 0);
			const double* const sampleVectorPtr = sampleVectors.ptr<double>(i);
			const double* const weightedSampleVectorPtr = weightedSampleVector.ptr<double>();
			for(int k = 0; k < covMat->rows; ++k) {
				double* covMatPtr = covMat->ptr<double>(k) + k;
				for(int l = k; l < sampleVector.cols; ++l) *(covMatPtr++) += sampleVectorPtr[k] * weightedSampleVectorPtr[l];
			}
		}
	} else {
		context->getContextElementsOf(currentPos);
		while(!context->getNextContextElement(sampleVector)) {
			if(weightingContext) {
				weightingContext->getNextContextElement(weightingVector);
				weightedSampleVector = sampleVector * (*(weightsPtr++) = otherWeightingFunction->computeWeight(weightingVector));
			} else weightedSampleVector = sampleVector * (*(weightsPtr++) = weightingFunction->computeWeight(sampleVector)); // do weighting for WLS and store weight
			const double* const sampleVectorPtr = sampleVector.ptr<double>();
			const double* const weightedSampleVectorPtr = weightedSampleVector.ptr<double>();
			for(int k = 0; k < covMat->rows; ++k) {
				double* covMatPtr = covMat->ptr<double>(k) + k;
				for(int l = k; l < sampleVector.cols; ++l) *(covMatPtr++) += sampleVectorPtr[k] * weightedSampleVectorPtr[l];
			}
		}
	}
	*covMat = covMat->rowRange(0, covMat->rows - 1); // make last row invisible for computePrediction function of WLS
	for(int k = 1; k < covMat->rows; ++k) { // copy values from upper triangular matrix
		double* covMatPtr = covMat->ptr<double>(k);
		for(int l = 0; l < k; ++l) *(covMatPtr++) = covMat->at<double>(l, k);
	}
} // end LSPredictionComputer::estimate


double LSVarianceComputer::compute(const Point3i& currentPos, Context* context) {
	if(!context->getTrainingregion().getNumberOfElements()) {
		if(coefficients->cols < 3) {
			if(coefficients->cols < 2) return predictor->getMaxval() * predictor->getMaxval() * .25; // first image pixel (use heuristic)
			else return predictor->getMaxval() * predictor->getMaxval() * .0625; // second image pixel (also use heuristic)
		} else { // other pixels where no training region is available (use conventional variance estimation)
			*coefficients = coefficients->colRange(0, coefficients->cols - 1) - coefficients->at<double>(coefficients->cols - 1); // subtract mean
			return coefficients->dot(*coefficients) / (coefficients->cols - 1); // unbiased sample variance
		}
	}
	double coeffEstErrorFactor = covMat->col(covMat->cols - 1).dot(coefficients->col(1)); // coefficients estimation error relative to variance of input gaussian process
	if(coeffEstErrorFactor < 0) coeffEstErrorFactor = 10; // singular matrix! -> n' * (X'X)^-1 * n no longer positive definite -> unreliable estimates

	double sumOfSquaredResiduals = 0.0;
	if(wlsVarianceEquation) {
		Mat sampleVector;
		double residual;
	//	double wSum = 0.0, numer = 0.0, denom = 0.0, weight, residualsum = 0.0, weightsum = 0.0, p = 0.0, q = 0.0, squaredWeight, weightsSum = 0.0;
		*coefficients = coefficients->col(0);
		coefficients->push_back(-1.0); // subtract value of dependent variable (true pixel value) to compute residuals
		*coefficients = coefficients->t(); // transpose for dot product
	//	context->getContextElementsOf();
	//	while(!context->getNextContextElement(sampleVector)) {
	//		residual = coefficients->dot(sampleVector);
	//		weight = (1.0 / *(weightsPtr++));
	//		numer += weight * (residual * residual);
	//		denom += weight * weight;
	//	}
	//	if(denom < 1e-5) denom = 1e-5;
	//	numer /= denom;
	//	while(!context->getNextContextElement(sampleVector)) {
	//		residual = coefficients->dot(sampleVector);
	//		weight = (1.0 / *(weightsPtr++));
	//		numer += weight * (residual * residual);
	//		residualsum += (residual * residual);
	//		weightsum += weight;
	//		denom += weight * weight;
	//	}
	//	q = ((double)weights->cols * denom - weightsum * weightsum);
	//	if(q > 1e-5) {
	//		p = (denom * residualsum - weightsum * numer) / q;
	//		q = ((double)weights->cols * numer - weightsum * residualsum) / q;
	//	}
	////	if(p + q > 1e-5) q /= p + q; else q /= 1e-5;
	//	if(q < 1e-5) q = 1e-5;
	//	if(p < 0) p = 0;
	//	q /= p + q;
		double* weightsPtr = weights->ptr<double>();
		context->getContextElementsOf();
		while(!context->getNextContextElement(sampleVector)) {
			residual = coefficients->dot(sampleVector);
	//		weight = 1.0 / ((1.0 / *(weightsPtr)) * numer + 1.0);
	//		weight = 1.0 / ((1.0 / *(weightsPtr) - 1.0) * q + 1.0);
	//		weight = 1.0 / ((1.0 / *(weightsPtr++) - 1.0) * 10.0 + 1.0);
	//		wSum += *weightsPtr;
			*weightsPtr *= *weightsPtr;
	//		weightsSum += squaredWeight = *weightsPtr * *(weightsPtr++);
	//		weightsSum += squaredWeight = weight * *(weightsPtr++);
	//		weightsSum += squaredWeight = weight;
	//		weightsSum += squaredWeight = *(weightsPtr++);
	//		sumOfSquaredResiduals += residual * residual * squaredWeight;
			sumOfSquaredResiduals += residual * residual * *(weightsPtr++);
		}
	} else {
		sumOfSquaredResiduals = (covMat->ptr<double>())[(context->getFullNeighborhood().getNumberOfElements() + 1) * covMat->rows + covMat->cols - 2]
			- covMat->col(covMat->cols - 2).dot(coefficients->col(0));
		if(sumOfSquaredResiduals < 0.0) sumOfSquaredResiduals = 0.0;
	}

	// [sum of squared residuals] * [weights normalization factor (mean of all weights shall be 1)] * [factor to consider coefficients estimation error] / [DOF]
	if(weights->cols) return sumOfSquaredResiduals * (double)weights->cols * (1.0 + coeffEstErrorFactor) / (predictor->computeDegreesOfFreedom() * sum(*weights)[0]);
	else return sumOfSquaredResiduals * (1.0 + coeffEstErrorFactor) / predictor->computeDegreesOfFreedom();
} // end LSVarianceComputer::compute


double LSDegreesOfFreedomComputer::compute(const Point3i& currentPos, Context* context) {
	if(!context->getTrainingregion().getNumberOfElements()) return 1.0;
	int dof = context->getTrainingregion().getNumberOfElements() - covMat->rows; // n - k
	return (dof < 1 ? 1 : dof);
} // end LSDegreesOfFreedomComputer::compute

} // end namespace vanilc

