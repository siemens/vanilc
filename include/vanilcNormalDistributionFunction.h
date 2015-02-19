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
// here, boost is only required for visual studio
#ifdef BOOST
	#include "boost/math/special_functions/erf.hpp"
#endif

#include "vanilcDistributionFunction.h"

namespace vanilc {

using namespace std;
using namespace cv;

class NormalDistributionFunction : public DistributionFunction {
public:
	void setParameters(Mat parameters, unsigned int cropped = 0) {
		factor = parameters.at<double>(0, 0);
		factor05shifted = factor05 = 0.5 * factor;
		mean = parameters.at<double>(1, 0);
		if(parameters.at<double>(2, 0) < 1e-14) invstddev2 = 1.0 / sqrt(2.0 * 1e-14);
		else invstddev2 = 1.0 / sqrt(2.0 * parameters.at<double>(2, 0));
		if(cropped) {
			factor05shifted = factor05 *= factor / (computeValue(0.5 + cropped) - computeValue(-0.5));
			factor05shifted -= computeValue(-0.5);
		}
	};
	double getFactor() { return factor; };
	#ifdef BOOST
		double computeValue(double x) { return factor05shifted + factor05 * boost::math::erf(invstddev2 * (x - mean)); }; // version for visual studio using boost
	#elif defined WIN32
		double computeValue(double x) { return 0.0; }; // this case is only for the compiler but it should never happen
	#else
		double computeValue(double x) { return factor05shifted + factor05 * erf(invstddev2 * (x - mean)); }; // version without dependency on boost
	#endif
	NormalDistributionFunction* clone() const { return new NormalDistributionFunction(*this); }; // "covariant return type" for "virtual copy constructor"

private:
	double factor;
	double factor05;
	double factor05shifted;
	double mean;
	double invstddev2;
};

} // end namespace vanilc

