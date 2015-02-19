// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcDistributionMaker.h"

namespace vanilc {

DistributionMaker::DistributionMaker(unsigned int numberOfSamplingPoints) : functions(vector<DistributionFunction*>()), offset(0), multiplier(1) {
	for(unsigned int i = implicitDistribution.size(); i < numberOfSamplingPoints; ++i)
		implicitDistribution.push_back(ImplicitDistributionElement(this, i));
} // end DistributionMaker::DistributionMaker

DistributionMaker::DistributionMaker(const DistributionMaker& original) {
	offset = original.offset;
	multiplier = original.multiplier;
	for(unsigned int i = implicitDistribution.size(); i < original.implicitDistribution.size(); ++i)
		implicitDistribution.push_back(ImplicitDistributionElement(this, i));
	for(unsigned int i = 0; i < original.functions.size(); ++i)
		addDistributionFunction(original.functions[i]->clone());
} // end DistributionMaker::DistributionMaker

DistributionMaker& DistributionMaker::operator=(const DistributionMaker& original) {
	if(&original != this) {
		offset = original.offset;
		multiplier = original.multiplier;
		for(unsigned int i = implicitDistribution.size(); i < original.implicitDistribution.size(); ++i)
			implicitDistribution.push_back(ImplicitDistributionElement(this, i));
		for(unsigned int i = 0; i < original.functions.size(); ++i) {
			addDistributionFunction(original.functions[i]->clone());
		}
	}
	return *this;
} // end DistributionMaker::operator=

DistributionMaker::~DistributionMaker() {
	for(vector<DistributionFunction*>::iterator it = functions.begin(); it != functions.end(); ++it)
		delete *it;
} // end DistributionMaker::~DistributionMaker

double DistributionMaker::computeValue(const unsigned int position) const {
	double result = 0;
	if(position >= implicitDistribution.size() - 1) return 1;
	if(position)
		for(vector<DistributionFunction*>::const_iterator it = functions.begin(); it != functions.end(); ++it)
			result += (*it)->computeValue(-0.5 + (double)position);
	return result;
} // end DistributionMaker::computeValue

void DistributionMaker::print() {
	cout << implicitDistribution.size() << endl;
	for(unsigned int i = 0; i < implicitDistribution.size(); ++i)
		cout << implicitDistribution[i].get() << " ";
	cout << endl << endl;
} // end DistributionMaker::printDistribution

} // end namespace vanilc

