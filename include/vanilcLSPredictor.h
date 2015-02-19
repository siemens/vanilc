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

#include "vanilcPredictor.h"
#include "vanilcInversePriorizedSQDWeightingFunction.h"
#include "vanilcInversePriorizedSSDWeightingFunction.h"
#include "vanilcCroppedPriorizedSSDWeightingFunction.h"
#include "vanilcIdentityWeightingFunction.h"

namespace vanilc {

using namespace std;
using namespace cv;

class LSPredictionComputer : public Computer {
public:
	LSPredictionComputer(Mat* covMat, Mat* coefficients, Mat* weights, const WeightingFunction& weightingFunction,
		double border_regularization, double inner_regularization, int wlsVarianceEquation, int solver, int maxTrainingVectors) :
			covMat(covMat), coefficients(coefficients), weights(weights), weightingFunction(weightingFunction.clone()),
			weightingContext(NULL), otherWeightingFunction(NULL),
			border_regularization(border_regularization), inner_regularization(inner_regularization),
			wlsVarianceEquation(wlsVarianceEquation), solver(solver), maxTrainingVectors(maxTrainingVectors) {};
	LSPredictionComputer(Mat* covMat, Mat* coefficients, Mat* weights, const WeightingFunction& weightingFunction,
		Context* weightingContext, const WeightingFunction& otherWeightingFunction,
		double border_regularization, double inner_regularization, int wlsVarianceEquation, int solver, int maxTrainingVectors) :
			covMat(covMat), coefficients(coefficients), weights(weights), weightingFunction(weightingFunction.clone()),
			weightingContext(weightingContext), otherWeightingFunction(otherWeightingFunction.clone()),
			border_regularization(border_regularization), inner_regularization(inner_regularization),
			wlsVarianceEquation(wlsVarianceEquation), solver(solver), maxTrainingVectors(maxTrainingVectors) {};
	~LSPredictionComputer() { delete covMat; delete coefficients; delete weights; delete weightingFunction; if(otherWeightingFunction) delete otherWeightingFunction; };
	virtual void init() {
		weightingFunction->setMaxval(predictor->getMaxval());
		if(weightingContext) {
			otherWeightingFunction->setMaxval(predictor->getMaxval());
			weightingContext->setImage(predictor->getContext().getImage());
			if(predictor->getContext().getBuffered()) weightingContext->bufferOn();
		}
	};
	double compute(const Point3i& currentPos, Context* context);

protected:
	virtual void estimate(const Point3i& currentPos);
	// state for current pixel
	Context* context;
	Context* weightingContext; // only for matching in order to compute weights (with otherWeightingFunction)
	Mat* covMat;
	Mat* coefficients;
	Mat* weights;

private:
	void solveSystem() {
		if(!solve(covMat->colRange(0, covMat->rows), covMat->colRange(covMat->rows, covMat->cols), *coefficients, solver))
			solve(covMat->colRange(0, covMat->rows), covMat->colRange(covMat->rows, covMat->cols), *coefficients, DECOMP_QR);
	}

	WeightingFunction* weightingFunction;
	WeightingFunction* otherWeightingFunction;
	const double border_regularization, inner_regularization;
	const int wlsVarianceEquation, solver, maxTrainingVectors;
};


class LSVarianceComputer : public Computer {
public:
	LSVarianceComputer(Mat* covMat, Mat* coefficients, Mat* weights, int wlsVarianceEquation) :
		covMat(covMat), coefficients(coefficients), weights(weights), wlsVarianceEquation(wlsVarianceEquation) {};
	double compute(const Point3i& currentPos, Context* context);

protected:
	Mat* covMat;
	Mat* coefficients;

private:
	Mat* weights;
	const int wlsVarianceEquation;
};


class LSDegreesOfFreedomComputer : public Computer {
public:
	LSDegreesOfFreedomComputer(Mat* covMat) : covMat(covMat) {};
	double compute(const Point3i& currentPos, Context* context);

protected:
	Mat* covMat;
};

} // end namespace vanilc

