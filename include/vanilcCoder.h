// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#define _USE_MATH_DEFINES

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
// here, boost is only required for visual studio
#ifdef BOOST
	#include "boost/math/special_functions/erf.hpp"
#endif

#include "vanilcDefinitions.h"
#include "vanilcConfig.h"
#include "vanilcRawIO.h"
#include "vanilcPredictorConstructor.h"
#include "vanilcUniformDistributionFunction.h"
#include "vanilcLaplaceDistributionFunction.h"
#include "vanilcNormalDistributionFunction.h"
#include "vanilcTDistributionFunction.h"
#include "vanilcSparseDistributionFunction.h"
#ifdef ARITHMETIC_CODING
	#include "vanilcDistributionMaker.h"
	#include "vanilcArithmeticCoder.h"
#endif
#ifdef GOLOMB_CODING
	#include "vanilcRiceGolombCoder.h"
#endif

namespace vanilc {

using namespace std;
using namespace cv;

enum ImageType {img_gray, img_color, img_3D, IMG_END };

class Coder {
public:
	Coder(Config& config);
	~Coder();
	void setImage(const Mat& image);
	void getImage(Mat& image) const;
//	void setBitstream(Mat bitstream) { this->bitstream = bitstream; };
//	Mat getBitstream() { return bitstream; };
	unsigned int writeBitstreamToFile(const string filename);
	unsigned int readBitstreamFromFile(const string filename);
	void code(char encoding); // 2 = prediction only; 1 = encoding; 0 = decoding

private:
	void createPredictor();
	void convertTo2D(const Mat& image3D, Mat& image2D, unsigned int slice = 0) const;
	Mat transp(const Mat& image) const;
	void codeHeader(bool encoding, unsigned int &maxval, unsigned int &width, unsigned int &height, unsigned int &depth);

	// config
	Config* config;
	bool verbose;
	double sparsify_distribution;

	Mat image, predictionImage, varianceImage, dofImage;
	unsigned int type, bitdepth;
	unsigned int imageDirection; // 1 if image is being transposed before coding
	Context context, weightingContext;
	Predictor* predictor;
	#ifdef ARITHMETIC_CODING
		GenericDistributionCoder* entropyCoder;
	#elif defined GOLOMB_CODING
		RiceGolombCoder* entropyCoder;
	#endif
};

class NoUnsignedImageException : public Exception {
	virtual const char* what() const throw() { return "This program can only compress 8 bit and 16 bit unsigned integer images."; }
};

} // end namespace vanilc

