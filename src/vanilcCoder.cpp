// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcCoder.h"

namespace vanilc {

Coder::Coder(Config& config) : config(&config), imageDirection(0), predictor(NULL) {
	// config
	verbose = !config.get<bool>("quiet");
	sparsify_distribution = config.get<double>("sparsify_distribution");

	// configure context
	if(config.get<double>("neighborhood_front") > 0) // 3-D neighborhood prediction?
		context.setFullNeighborhood(StructuringElement::createHalfEllipsoidElement(config.get<double>("neighborhood_top"), config.get<double>("neighborhood_left"), config.get<double>("neighborhood_right"), config.get<double>("neighborhood_bottom"), config.get<double>("neighborhood_front"), true));
	else context.setFullNeighborhood(StructuringElement::createHalfEllipseElement(config.get<double>("neighborhood_top"), config.get<double>("neighborhood_left"), config.get<double>("neighborhood_right"), true)); // configure 2-D neighborhood mask

	if(config.get<int>("training_size_3D") > 0)
		context.setFullTrainingregion(StructuringElement::createHalfCuboidElement(config.get<int>("training_size"), config.get<int>("training_size"), config.get<int>("training_size"), config.get<int>("training_size"), config.get<int>("training_size_3D"), false)); // 3-D training mask
	else context.setFullTrainingregion(StructuringElement::createHalfSquareElement(config.get<int>("training_size"), false)); // 2-D training mask

	if(config.get<double>("other_matching_neighborhood") > 0.0) {
		if(config.get<double>("neighborhood_front") > 0) // 3-D neighborhood prediction?
			weightingContext.setFullNeighborhood(StructuringElement::createHalfEllipsoidElement(config.get<double>("other_matching_neighborhood"), config.get<double>("other_matching_neighborhood"), config.get<double>("other_matching_neighborhood"), config.get<double>("other_matching_neighborhood"), config.get<double>("other_matching_neighborhood"), true));
		else weightingContext.setFullNeighborhood(StructuringElement::createHalfEllipseElement(config.get<double>("other_matching_neighborhood"), config.get<double>("other_matching_neighborhood"), config.get<double>("other_matching_neighborhood"), true)); // configure 2-D neighborhood mask
		weightingContext.setFullTrainingregion(context.getTrainingregion());
	}

	createPredictor();

	// configure entropy coder
	#ifdef ARITHMETIC_CODING
		entropyCoder = new ArithmeticCoder();
	#elif defined GOLOMB_CODING
		entropyCoder = new RiceGolombCoder();
	#endif

	// show masks for debugging
	#ifdef DEBUGOUT
		Mat outMat(context.getNeighborhood().getRows(), context.getNeighborhood().getCols(), CV_8U);
		for(int j = 0; j <= context.getNeighborhood().getFront(); ++j) {
			convertTo2D(context.getNeighborhood().getMask(), outMat, j); cout << outMat << endl << endl; }
		outMat.create(context.getTrainingregion().getRows(), context.getTrainingregion().getCols(), CV_8U);
		convertTo2D(context.getTrainingregion().getMask(), outMat, 0); cout << outMat << endl << endl;
	#endif
} // end Coder::Coder

Coder::~Coder() {
	if(predictor) delete predictor;
	if(entropyCoder) delete entropyCoder;
} // end Coder::~Coder

void Coder::createPredictor() {
	if(predictor) { delete predictor; predictor = NULL; }
	if(config->get<string>("predictor") == "MEAN")
		predictor = PredictorConstructor::constructMeanpredictor(*config, context);
	else if(config->get<string>("predictor") == "MED")
		predictor = PredictorConstructor::constructMEDpredictor(*config, context);
	else if(config->get<string>("predictor") == "NLM")
		predictor = PredictorConstructor::constructNLMpredictor(*config, context);
	else if(config->get<string>("predictor") == "FASTLS")
		predictor = PredictorConstructor::constructFastLSpredictor(*config, context);
	else if(config->get<string>("predictor") == "LS")
		predictor = PredictorConstructor::constructLSpredictor(*config, context);
	else
		// configure covariance matrix estimator with weighting function and contexts for training and prediction
		if(config->get<double>("other_matching_neighborhood") > 0.0)
			predictor = PredictorConstructor::constructWLSpredictor(*config, context, &weightingContext);
		else predictor = PredictorConstructor::constructWLSpredictor(*config, context);
} // end Coder::definePredictor

void Coder::convertTo2D(const Mat& image3D, Mat& image2D, unsigned int slice) const {
	image2D.create(image3D.size[1], image3D.size[2], CV_64F);
	int sz[] = { 1, image3D.size[1], image3D.size[2] };
	Range r[] = { Range(slice, slice + 1), Range::all(), Range::all() };
	image3D(r).copyTo(Mat(3, sz, CV_64F, image2D.ptr<void>()));
} // end Coder::convertTo2D

Mat Coder::transp(const Mat& image) const { // transpose a 3-D matrix (slices individually)
	Mat tmp = image;
	if(imageDirection) {
		int sz[] = { image.size[0], image.size[2], image.size[1] };
		tmp = Mat(3, sz, CV_64F);
		for(int j = 0; j < tmp.size[0]; ++j)
			for(int k = 0; k < tmp.size[1]; ++k) {
				double* tmpPtr = &(tmp.at<double>(j, k, 0));
				for(int l = 0; l < tmp.size[2]; ++l)
					*(tmpPtr++) = image.at<double>(j, l, k);
			}
	}
	return tmp;
} // end Coder::transp

// store type and bitdepth and convert all images to 3-D double arrays for internal processing
void Coder::setImage(const Mat& image) {
	if(image.depth() != CV_8U && image.depth() != CV_16U) throw NoUnsignedImageException(); // floating point images cannot be compressed
	if(image.channels() > 1) {
		type = img_color;
		vector<Mat> vectorOfChannels;
		split(image, vectorOfChannels);
		int sz[] = { image.channels() + 1, image.rows, image.cols };
		this->image = Mat(3, sz, CV_64F, 1.0); // ones to enable affine prediction
		for(int i = 0; i < image.channels(); ++i) {
			Range r[] = { Range(i + 1, i + 2), Range::all(), Range::all() };
			vectorOfChannels.at(CHANNEL_ORDER[i]).convertTo(Mat(image.size(), CV_64F, this->image(r).ptr<double>()), CV_64F);
		}
	} else {
		if(image.dims == 3) {
			type = img_3D;
			image.convertTo(this->image, CV_64F);
		} else { // also a 2-D image must be converted to a 3-D array!
			type = img_gray;
			int sz[] = { 1, image.rows, image.cols };
			this->image.create(3, sz, CV_64F);
			image.convertTo(Mat(image.size(), CV_64F, this->image.ptr<double>()), CV_64F);
		}
	}
	double maxval;
	minMaxIdx(this->image, NULL, &maxval);
	#ifdef WIN32
		bitdepth = (unsigned int)(log(maxval) / log(2.0) + 1.0);
	#else
		bitdepth = (unsigned int)(log2(maxval) + 1.0);
	#endif
	// attention: if the following line is commented, original 16 bit images are decoded as 8 bit images if maximum value was smaller than 256!
	if(image.depth() == CV_16U && bitdepth < 9) bitdepth = 9;
	if(config->get<bool>("adaptive_transposition")) {
		Range rA[] = { Range::all(), Range::all(), Range(0, 1) };
		Range rB[] = { Range::all(), Range::all(), Range(1, this->image.size[2]) };
		Range r1[] = { Range::all(), Range::all(), Range(0, this->image.size[2] - 1) };
		Range r2[] = { Range::all(), Range::all(), Range(1, this->image.size[2]) };
		Mat diff = this->image(r2) - this->image(r1);
		double finiteDiffsX = pow(mean(diff.mul(diff))[0], 2) * mean(this->image(rA).mul(this->image(rA)))[0];
		rA[1] = Range(0, 1); rA[2] = Range::all();
		rB[1] = Range(1, this->image.size[1]); rB[2] = Range::all();
		r1[1] = Range(0, this->image.size[1] - 1); r1[2] = Range::all();
		r2[1] = Range(1, this->image.size[1]); r2[2] = Range::all();
		diff = this->image(r2) - this->image(r1);
		double finiteDiffsY = pow(mean(diff.mul(diff))[0], 2) * mean(this->image(rA).mul(this->image(rA)))[0];
		#ifdef DEBUGOUT
			cout << "Y-to-X diff ratio: " << finiteDiffsY / finiteDiffsX << endl;
		#endif
		if(finiteDiffsY > finiteDiffsX) {
			imageDirection = 1;
			this->image = transp(this->image);
		}
	}
	predictor->setImage(&(this->image), ((1 << bitdepth) - 1), config->get<bool>("neighborhood_buffer"));
} // end Coder::setImage

// restore original image type
void Coder::getImage(Mat& image) const {
	Mat tmp = this->image;
	if(config->get<bool>("adaptive_transposition"))
		tmp = transp(tmp);
	if(type == img_color) {
		vector<Mat> vectorOfChannels(tmp.size[0] - 1);
		for(int i = 0; i < tmp.size[0] - 1; ++i) {
			if(bitdepth <= 8) vectorOfChannels.at(CHANNEL_ORDER[i]).create(tmp.size[1], tmp.size[2], CV_8U);
			else vectorOfChannels.at(CHANNEL_ORDER[i]).create(tmp.size[1], tmp.size[2], CV_16U);
			Range r[] = { Range(i + 1, i + 2), Range::all(), Range::all() };
			int sz[] = { 1, tmp.size[1], tmp.size[2] };
			tmp(r).convertTo(Mat(3, sz, vectorOfChannels.at(CHANNEL_ORDER[i]).type(), vectorOfChannels.at(CHANNEL_ORDER[i]).ptr<void>()),
				vectorOfChannels.at(CHANNEL_ORDER[i]).type());
		}
		merge(vectorOfChannels, image);
	} else {
		if(type == img_3D) {
			if(bitdepth <= 8) tmp.convertTo(image, CV_8U);
			else tmp.convertTo(image, CV_16U);
		} else { // construct new 2-D header
			if(bitdepth <= 8) image.create(tmp.size[1], tmp.size[2], CV_8U);
			else image.create(tmp.size[1], tmp.size[2], CV_16U);
			int sz[] = { 1, tmp.size[1], tmp.size[2] };
			tmp.convertTo(Mat(3, sz, image.type(), image.ptr<void>()), image.type());
		}
	}
} // end Coder::getImage

unsigned int Coder::writeBitstreamToFile(const string filename) {
	ofstream fs(filename.c_str(), ios::out | ios::binary);
	unsigned int filesize = entropyCoder->writeBitstream(fs);
	fs.close();
	return filesize;
} // end Coder::writeBitstreamToFile

unsigned int Coder::readBitstreamFromFile(const string filename) {
	ifstream fs(filename.c_str(), ios::in | ios::binary);
	unsigned int filesize = entropyCoder->readBitstream(fs);
	fs.close();
	return filesize;
} // end Coder::readBitstreamFromFile

void Coder::codeHeader(bool encoding, unsigned int &maxval, unsigned int &width, unsigned int &height, unsigned int &depth) {
	// header: image type
	#ifdef ARITHMETIC_CODING
		DistributionMaker imageTypeDistribution(IMG_END + 1);
		imageTypeDistribution.addDistributionFunction(new UniformDistributionFunction());
		imageTypeDistribution.getDistributionFunction()->setParameters((Mat_<double>(1, 1) << 1.0), 2);
		entropyCoder->setDistribution(imageTypeDistribution.getImplicitDistribution());
	#elif defined GOLOMB_CODING
		entropyCoder->setParameters(IMG_END / 2, IMG_END * IMG_END);
	#endif
	entropyCoder->code(type, encoding);
	// header: transposed?
	if(config->get<bool>("adaptive_transposition")) {
		#ifdef ARITHMETIC_CODING
			DistributionMaker imageTransposedDistribution(3);
			imageTransposedDistribution.addDistributionFunction(new UniformDistributionFunction());
			imageTransposedDistribution.getDistributionFunction()->setParameters((Mat_<double>(1, 1) << 1.0), 1);
			entropyCoder->setDistribution(imageTransposedDistribution.getImplicitDistribution());
		#elif defined GOLOMB_CODING
			entropyCoder->setParameters(.5, 1);
		#endif
		entropyCoder->code(imageDirection, encoding);
	}
	// header: bitdepth
	#ifdef ARITHMETIC_CODING
		DistributionMaker imageDepthDistribution(18); // maximum bit depth: 16 bit
		imageDepthDistribution.addDistributionFunction(new LaplaceDistributionFunction());
		imageDepthDistribution.getDistributionFunction()->setParameters((Mat_<double>(3, 1) << 1.0, 8.0, 4.0 * 4.0), 16);
		entropyCoder->setDistribution(imageDepthDistribution.getImplicitDistribution());
	#elif defined GOLOMB_CODING
		entropyCoder->setParameters(8.0, 4.0 * 4.0);
	#endif
	entropyCoder->code(bitdepth, encoding);
	maxval = ((1 << bitdepth) - 1); // maximum intensity value in image
	// header: image dimensions (width, height, possibly depth)
	#ifdef ARITHMETIC_CODING
		DistributionMaker imageDimDistribution(config->get<int>("max_image_size") + 2); // maximum number of pixels in one dimension plus two
		imageDimDistribution.addDistributionFunction(new LaplaceDistributionFunction());
		imageDimDistribution.addDistributionFunction(new UniformDistributionFunction());
		// standard deviation = MAX_IMAGE_SIZE / 9 : upper bound for standard deviation by assuming MAX_IMAGE_SIZE < ~40000 pixels
		imageDimDistribution.getDistributionFunction(0)->setParameters((Mat_<double>(3, 1) << 1.0, 0.0, config->get<int>("max_image_size") * config->get<int>("max_image_size") / 81), config->get<int>("max_image_size"));
		imageDimDistribution.getDistributionFunction(1)->setParameters((Mat_<double>(1, 1) << 0.0), config->get<int>("max_image_size"));
		entropyCoder->setDistribution(imageDimDistribution.getImplicitDistribution());
	#elif defined GOLOMB_CODING
		entropyCoder->setParameters(config->get<int>("max_image_size") / 9, config->get<int>("max_image_size") * config->get<int>("max_image_size") / 81);
	#endif
	if(encoding) {
		width = image.size[image.dims - 1];
		height = image.size[image.dims - 2];
		if(type == img_color || type == img_3D) depth = image.size[0];
	}
	entropyCoder->code(width, encoding);
	#ifdef ARITHMETIC_CODING
		const double imageDimRegRatio = ((double)config->get<int>("max_image_size") + 1.0) / (double)(1 << config->get<int>("max_bits_per_pixel") - 1);
		imageDimDistribution.getDistributionFunction(0)->setParameters(
			(Mat_<double>(3, 1) << 1.0 - imageDimRegRatio, (double)width, (double)(width * width)), config->get<int>("max_image_size"));
		imageDimDistribution.getDistributionFunction(1)->setParameters((Mat_<double>(1, 1) << imageDimRegRatio), config->get<int>("max_image_size"));
		entropyCoder->setDistribution(imageDimDistribution.getImplicitDistribution());
	#elif defined GOLOMB_CODING
		entropyCoder->setParameters((double)width, (double)(width * width));
	#endif
	entropyCoder->code(height, encoding);
	if(type == img_3D) entropyCoder->code(depth, encoding);
	else if(type == img_color) {
		#ifdef ARITHMETIC_CODING
			imageDimDistribution.getDistributionFunction(0)->setParameters((Mat_<double>(3, 1) << 1.0 - imageDimRegRatio, 3.0, 2.0), config->get<int>("max_image_size"));
			imageDimDistribution.getDistributionFunction(1)->setParameters((Mat_<double>(1, 1) << imageDimRegRatio), config->get<int>("max_image_size"));
		#elif defined GOLOMB_CODING
			entropyCoder->setParameters(3.0, 2.0);
		#endif
		entropyCoder->code(depth, encoding);
	}
	if(!encoding) {
		int sz[] = { depth, height, width };
		image = Mat(3, sz, CV_64F, numeric_limits<double>::quiet_NaN());
		if(type == img_color) {
			Range r[] = { Range(0, 1), Range::all(), Range::all() };
			image(r) = Scalar(1.0);
		}
		predictor->setImage(&image, maxval, config->get<bool>("neighborhood_buffer"));
	}
} // end Coder::codeHeader

void Coder::code(char encoding) {
	double prediction, variance, dof;
	unsigned int maxval, width, height, depth = 1;
	codeHeader((bool)encoding, maxval, width, height, depth);

	// choose a parametric distribution for pixel intensities
	#ifdef OBSERVEENCODING
		namedWindow("Prediction observation window", CV_WINDOW_NORMAL);
		Mat residualImage;
	#endif
	#ifdef ARITHMETIC_CODING
		DistributionMaker distributionMaker(maxval + 2);
		DistributionFunction* mainDistributionFunction;
		double regDistVar, regDistRatio;
	if(encoding < 2) { // not only prediction
		if(config->get<string>("distribution") == "T")
			mainDistributionFunction = new TDistributionFunction();
		else if(config->get<string>("distribution") == "LAPLACE")
			mainDistributionFunction = new LaplaceDistributionFunction();
		else if(config->get<string>("distribution") == "UNIFORM")
			mainDistributionFunction = new UniformDistributionFunction();
		else
			mainDistributionFunction = new NormalDistributionFunction();
		if(sparsify_distribution > 0.0) distributionMaker.addDistributionFunction(
			new SparseDistributionFunction(mainDistributionFunction, &image, maxval, type == img_color ,
				StructuringElement::createHalfCircleElement(config->get<double>("sparsification_size"), false), sparsify_distribution));
		else distributionMaker.addDistributionFunction(mainDistributionFunction);
		if(config->get<string>("regularization_distribution") == "UNIFORM") {
			distributionMaker.addDistributionFunction(new UniformDistributionFunction());
			regDistVar = 1.0;
			regDistRatio = ((double)maxval + 1.0) / (double)(1 << config->get<int>("max_bits_per_pixel") - 1); // needs further examination why " - 1" is necessary
		} else if(config->get<string>("regularization_distribution") == "LAPLACE") {
			distributionMaker.addDistributionFunction(new LaplaceDistributionFunction());
			regDistVar = pow((double)maxval / log(config->get<double>("max_to_min_regularization_ratio")), 2.0) * 2.0;
			regDistRatio = config->get<double>("max_to_min_regularization_ratio") * sqrt(regDistVar * 2.0) / (double)(1 << config->get<int>("max_bits_per_pixel"))
				* (1.0 - exp(-((double)maxval + 1.0) / sqrt(2.0 * regDistVar))); // upper bound
		} else {
			distributionMaker.addDistributionFunction(new NormalDistributionFunction());
			regDistVar = ((double)maxval * (double)maxval) / (2.0 * log(config->get<double>("max_to_min_regularization_ratio")));
			regDistRatio = config->get<double>("max_to_min_regularization_ratio") * sqrt(regDistVar * 2.0 * M_PI) / (double)(1 << config->get<int>("max_bits_per_pixel"))
				#ifdef BOOST
					* boost::math::erf(((double)maxval + 1.0) / sqrt(8.0 * regDistVar)); // upper bound (version with dependency on boost)
				#elif defined WIN32
					; // this case is only to satisfy the compiler but it should never happen
				#else
					* erf(((double)maxval + 1.0) / sqrt(8.0 * regDistVar)); // upper bound (version with dependency on GCC)
				#endif
		}
		entropyCoder->setDistribution(distributionMaker.getImplicitDistribution());
	#else
	if(encoding < 2) { // not only prediction
	#endif
	} else { // only prediction
		predictionImage.create(3, image.size, CV_64F);
		varianceImage.create(3, image.size, CV_64F);
		dofImage.create(3, image.size, CV_64F);
	}
	#ifdef DEBUGOUT
		cout << "Processed Pixel lines (overall " << height << " lines):" << endl;
	#else
		if(verbose) cout << "progress (%): [";
	#endif

	// main processing loop for pixel-wise coding
	for(int j = (type == img_color ? 1 : 0); j < (int)depth; ++j) {
		#ifdef DEBUGOUT
			if(type == img_color) { switch(CHANNEL_ORDER[j - 1]) {
				case 0: cout << "blue:  "; break; case 1: cout << "green: "; break; case 2: cout << "red:   "; break; default: cout << "?????: "; }};
		#endif
		#ifdef OBSERVEENCODING
			Mat tmp = transp(image);
			convertTo2D(tmp, residualImage, j);
			residualImage = residualImage / maxval;
			imshow("Prediction observation window", residualImage);
			waitKey(50);
		#endif
		if(config->get<int>("inter_channel_prediction") && type == img_color && j > 1) { // include previous color channels into prediction neighborhood (affine prediction)
			if(config->get<int>("inter_channel_prediction") == 1)
				context.setFullNeighborhood(StructuringElement::createHalfEllipseElementMultichannel(
					config->get<double>("neighborhood_top"), config->get<double>("neighborhood_left"), config->get<double>("neighborhood_right"), j, true));
			else	context.setFullNeighborhood(StructuringElement::createHalfEllipseElementMultichannelForward(
					config->get<double>("neighborhood_top"), config->get<double>("neighborhood_left"), config->get<double>("neighborhood_right"), j, true));
			createPredictor();
			predictor->setImage(&image, maxval, config->get<bool>("neighborhood_buffer"));
		}
		for(int k = 0, kk = 0, percentage = (100 * (type == img_color ? j - 1 : j) - 1) / (int)(type == img_color ? depth - 1 : depth) + 1;
			percentage <= (100 * (type == img_color ? j : j + 1) - 1) / (int)(type == img_color ? depth - 1 : depth) + 1; ++percentage) {
		kk = (type == img_color ? depth - 1 : depth) * height * percentage / 100 - (type == img_color ? j - 1 : j) * height;
		if(kk > (int)height) kk = height;
		for(; k < kk; ++k) {
			#ifdef DEBUGOUT
				cout << k << " ";
			#endif
			for(int l = 0; l < (int)width; ++l) {
				prediction = predictor->computePrediction(Point3i(l, k, j));
				variance = predictor->computeVariance();
				dof = predictor->computeDegreesOfFreedom();
				if(encoding < 2) { // not only prediction
					#ifdef ARITHMETIC_CODING
						if(sparsify_distribution > 0) distributionMaker.getDistributionFunction(0)->setParameters(
							(Mat_<double>(7, 1) << (1.0 - regDistRatio), prediction, variance, dof, (double)j, (double)k, (double)l));
						else distributionMaker.getDistributionFunction(0)->setParameters((Mat_<double>(4, 1) << (1.0 - regDistRatio), prediction, variance, dof));
						distributionMaker.getDistributionFunction(1)->setParameters((Mat_<double>(3, 1) << regDistRatio, prediction, regDistVar), maxval);
					#elif defined GOLOMB_CODING
						entropyCoder->setParameters(prediction, variance);
					#endif
					entropyCoder->code(image.at<double>(j, k, l), (bool)encoding);
				} else { // prediction only
					predictionImage.at<double>(j, k, l) = prediction;
					varianceImage.at<double>(j, k, l) = variance;
					dofImage.at<double>(j, k, l) = dof;
				}
				#ifdef OBSERVEENCODING
					if(encoding) {
						#ifdef SHOW_PREDICTION
							residualImage.at<double>(imageDirection ? l : k, imageDirection ? k : l) = prediction / maxval;
						#elif defined SHOW_PREDICTION_ERROR
							residualImage.at<double>(imageDirection ? l : k, imageDirection ? k : l) = (image.at<double>(j, k, l) - prediction) / maxval + 0.5;
						#elif defined SHOW_ABS_PREDICTION_ERROR
							residualImage.at<double>(imageDirection ? l : k, imageDirection ? k : l) = abs((image.at<double>(j, k, l) - prediction) / maxval);
						#elif defined SHOW_STANDARD_DEVIATION
							residualImage.at<double>(imageDirection ? l : k, imageDirection ? k : l) = sqrt(variance) / maxval;
						#elif defined SHOW_DOF
							residualImage.at<double>(imageDirection ? l : k, imageDirection ? k : l) = dof;
						#elif defined SHOW_BITS
							residualImage.at<double>(imageDirection ? l : k, imageDirection ? k : l) = entropyCoder->costs(image.at<double>(j, k, l)) / bitdepth;
						#endif
					} else	residualImage.at<double>(imageDirection ? l : k, imageDirection ? k : l) = image.at<double>(j, k, l) / maxval; // image
				#endif
			}
			#ifdef OBSERVEENCODING
				if(!((k + 1) % OBSERVATION_UPDATE_INTERVAL)) {
					imshow("Prediction observation window", residualImage);
					waitKey(50);
				}
			#endif
		}
		#ifndef DEBUGOUT
			if(verbose && kk != height) cout << percentage << " ";
		#endif
		}
		#ifdef DEBUGOUT
			cout << height << endl;
		#endif
		#ifdef OBSERVEENCODING
			RawIO rawWriter;
			rawWriter.imwrite("debug.raw", residualImage);
			imshow("Prediction observation window", residualImage);
			waitKey(1);
		#endif
	}
	#ifndef DEBUGOUT
		if(verbose) cout << "100] ";
	#endif
	if(encoding == 1) entropyCoder->finalize();
	#ifdef OBSERVEENCODING
		if(encoding < 2) {
			int pressedKey = -1;
			cout << "Press Q or <ESC> to quit." << endl;
			while(pressedKey != config->get<int>("keycode_q") && pressedKey != config->get<int>("keycode_esc")) pressedKey = waitKey();
		}
	#endif
} // end Coder::code

} // end namespace vanilc

