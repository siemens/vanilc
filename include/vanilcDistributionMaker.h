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
#include <cmath>

#include "vanilcDefinitions.h"
#include "vanilcDistributionFunction.h"

namespace vanilc {

using namespace std;
using namespace cv;

class DistributionElement;
class ImplicitDistributionElement;

class DistributionMaker {
public:
	typedef vector<ImplicitDistributionElement> ImplicitDistributionType; // values are only computed when accessed

	DistributionMaker() {};
	DistributionMaker(unsigned int numberOfSamplingPoints);
	DistributionMaker(const DistributionMaker& original);
	DistributionMaker& operator=(const DistributionMaker& original);
	~DistributionMaker();

	void addDistributionFunction(DistributionFunction* function) { functions.push_back(function); }; // overtakes memory management for function!
	DistributionFunction* getDistributionFunction(unsigned int i = 0) { return functions[i]; };

	double computeValue(const unsigned int position) const; // compute value at only one position
	ImplicitDistributionType* getImplicitDistribution() { return &implicitDistribution; };

	void setRangeParameters(RANGETYPE offset, RANGETYPE multiplier) { this->offset = offset; this->multiplier = multiplier; };
	RANGETYPE computeRangeValue(const unsigned int position) { return offset + (RANGETYPE)(computeValue(position) * multiplier + 0.5); };

	void print();

private:
	ImplicitDistributionType implicitDistribution;
	vector<DistributionFunction*> functions;
	RANGETYPE offset, multiplier;
};

class DistributionElement {
public:
	virtual double get() const = 0;
	virtual RANGETYPE getRangeValue() const = 0;
	virtual bool operator<(const RANGETYPE d) = 0;
	virtual bool operator>(const RANGETYPE d) = 0;
	virtual bool operator<=(const RANGETYPE d) = 0;
	virtual bool operator>=(const RANGETYPE d) = 0;
	virtual unsigned int getPosition() const = 0;
};

class ImplicitDistributionElement : public DistributionElement {
public:
	ImplicitDistributionElement() : distributionMaker(NULL), position(0) {};
	ImplicitDistributionElement(DistributionMaker* distributionMaker, unsigned int position) : distributionMaker(distributionMaker), position(position) {};
	double get() const { return distributionMaker->computeValue(position); };
	RANGETYPE getRangeValue() const { return distributionMaker->computeRangeValue(position); };
	bool operator<(const RANGETYPE d) { return distributionMaker->computeRangeValue(position) < d; };
	bool operator>(const RANGETYPE d) { return distributionMaker->computeRangeValue(position) > d; };
	bool operator<=(const RANGETYPE d) { return distributionMaker->computeRangeValue(position) <= d; };
	bool operator>=(const RANGETYPE d) { return distributionMaker->computeRangeValue(position) >= d; };
	DistributionMaker* getDistributionMaker() { return distributionMaker; };
	unsigned int getPosition() const { return position; };

private:
	DistributionMaker* distributionMaker;
	unsigned int position;
};

} // end namespace vanilc

