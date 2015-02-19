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

#include "vanilcContext.h"
#include "vanilcComputer.h"

namespace vanilc {

using namespace std;
using namespace cv;

class Predictor {
public:
	Predictor() :
		predictionComputer(NULL), varianceComputer(NULL), degreesOfFreedomComputer(new Computer) {};
	Predictor(const Context& context) :
		predictionComputer(NULL), varianceComputer(NULL), degreesOfFreedomComputer(new Computer), context(context) {};
	~Predictor() {
		if(predictionComputer) delete predictionComputer;
		if(varianceComputer) delete varianceComputer;
		if(degreesOfFreedomComputer) delete degreesOfFreedomComputer; }

	void setPredictionComputer(Computer* predictionComputer) {
		if(this->predictionComputer) delete(this->predictionComputer);
		this->predictionComputer = predictionComputer; this->predictionComputer->setPredictor(this); };
	void setVarianceComputer(Computer* varianceComputer) {
		if(this->varianceComputer) delete(this->varianceComputer);
		this->varianceComputer = varianceComputer; this->varianceComputer->setPredictor(this); };
	void setDegreesOfFreedomComputer(Computer* degreesOfFreedomComputer) {
		if(this->degreesOfFreedomComputer) delete(this->degreesOfFreedomComputer);
		this->degreesOfFreedomComputer = degreesOfFreedomComputer; this->degreesOfFreedomComputer->setPredictor(this); };

	const Context& getContext() const { return context; };
	unsigned int getMaxval() const { return maxval; };
	double getPrediction() const { return prediction; };

	void setImage(Mat* image, unsigned int maxval, bool buffered = true) {
		context.setImage(image);
		this->maxval = maxval;
		if(buffered) context.bufferOn();
		predictionComputer->init(); varianceComputer->init(); degreesOfFreedomComputer->init(); };

	double computePrediction(const Point3i& currentPos) {
		this->currentPos = currentPos;
		context.checkBorder(currentPos); // in border regions shrink neighborhood and training region
		return prediction = predictionComputer->compute(currentPos, &context); };
	double computeVariance() {
		return varianceComputer->compute(currentPos, &context); };
	double computeDegreesOfFreedom() {
		return degreesOfFreedomComputer->compute(currentPos, &context); };

private:
	Computer* predictionComputer;
	Computer* varianceComputer;
	Computer* degreesOfFreedomComputer;

	Context context;
	unsigned int maxval;

	Point3i currentPos;
	double prediction;
};

} // end namespace vanilc

