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
// here, boost is always required - otherwise this header file may be included but will not work
#ifdef BOOST
	#include "boost/math/distributions/students_t.hpp"
#endif

#include "vanilcDistributionFunction.h"

namespace vanilc {

using namespace std;
using namespace cv;

class TDistributionFunction : public DistributionFunction {
public:
	#ifdef BOOST
		TDistributionFunction() : tDistribution(5.0) {};
	#endif
	void setParameters(Mat parameters, unsigned int cropped = 0) {
		factor1 = factor = parameters.at<double>(0, 0);
		shift = 0;
		mean = parameters.at<double>(1, 0);
		if(parameters.at<double>(2, 0) < 1e-14) invstddev = 1.0 / sqrt(1e-14);
		else invstddev = 1.0 / sqrt(parameters.at<double>(2, 0));
		#ifdef BOOST
			tDistribution = boost::math::students_t(parameters.at<double>(3, 0));
		#endif
		if(cropped) {
			factor1 *= factor / (computeValue(0.5 + cropped) - computeValue(-0.5));
			shift = -computeValue(-0.5);
		}
	};
	double getFactor() { return factor; };
	#ifdef BOOST
		double computeValue(double x) { return shift + factor1 * cdf(tDistribution, invstddev * (x - mean)); };
	#else
		double computeValue(double x) { return 0.0; }; // remember, that this function cannot be used without boost, so this is only to satisfy the compiler
	#endif
	TDistributionFunction* clone() const { return new TDistributionFunction(*this); }; // "covariant return type" for "virtual copy constructor"

private:
	double factor;
	#ifdef BOOST
		boost::math::students_t tDistribution;
	#endif
	double factor1;
	double shift;
	double mean;
	double invstddev;
	double dof;
};

} // end namespace vanilc

