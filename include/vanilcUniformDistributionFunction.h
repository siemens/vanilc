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

#include "vanilcDistributionFunction.h"

namespace vanilc {

using namespace std;
using namespace cv;

class UniformDistributionFunction : public DistributionFunction {
public:
	void setParameters(Mat parameters, unsigned int cropped = 0) {
		factor = parameters.at<double>(0, 0);
		if(~cropped) cropped = 255; // uniform distribution _must_ be cropped
		factorMaxval = factor / (1.0 + cropped);
	};
	double getFactor() { return factor; };
	double computeValue(double x) { return factorMaxval * (x + .5); };
	UniformDistributionFunction* clone() const { return new UniformDistributionFunction(*this); }; // "covariant return type" for "virtual copy constructor"

private:
	double factor;
	double factorMaxval;
};

} // end namespace vanilc

