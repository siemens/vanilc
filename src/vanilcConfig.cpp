// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcConfig.h"

namespace vanilc {

Config::Config() {
	parameters.insert(pair<string, GenericParameter*>("version", new Parameter<bool>(false, 'v',
		"Show version of the program")));
	parameters.insert(pair<string, GenericParameter*>("help", new Parameter<bool>(false, 'h',
		"Show usage instructions")));
	parameters.insert(pair<string, GenericParameter*>("hh", new Parameter<bool>(false, 0,
		"Show all usage instructions")));
	parameters.insert(pair<string, GenericParameter*>("config", new Parameter<string>("", 'c',
		"YAML configuration file (default: config.yml)")));
	parameters.insert(pair<string, GenericParameter*>("show", new Parameter<bool>(false, 's',
		"Show image; if necessary first decode bitstream")));
	parameters.insert(pair<string, GenericParameter*>("resize", new Parameter<bool>(false, 'r',
		"Image is resized to fill the whole window (implies -s)")));
	parameters.insert(pair<string, GenericParameter*>("input", new Parameter<string>("", 'i',
		"Input image to be encoded or shown")));
	parameters.insert(pair<string, GenericParameter*>("output", new Parameter<string>("", 'o',
		"Output image to be written after decoding (use a lossless storage format!)")));
	parameters.insert(pair<string, GenericParameter*>("bitstream", new Parameter<string>("", 'b',
		"Binary file where encoded bitstream is written to or from which bitstream for decoding is read}")));
	parameters.insert(pair<string, GenericParameter*>("dimensions", new Parameter<string>("", 'd',
		"Image dimensions (size) for reading RAW 3-D images given in format BITDEPTHxWIDTHxHEIGHTxDEPTH, e.g., 8x512x512x10}")));
	parameters.insert(pair<string, GenericParameter*>("quiet", new Parameter<bool>(false, 'q',
		"Do not show any command line output")));
} // end Config::Config

Config::~Config() {
	for(map<string, GenericParameter*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
		delete it->second;
} // end Config::~Config

void Config::insertMoreParameters() {
	parameters.insert(pair<string, GenericParameter*>("keycode_q", new Parameter<int>(1048689, 0,
		"Keycode for Q key to close window.")));
	parameters.insert(pair<string, GenericParameter*>("keycode_esc", new Parameter<int>(1048603, 0,
		"Keycode for ESC key to close window.")));
	parameters.insert(pair<string, GenericParameter*>("predictor", new Parameter<string>("WLS", 0,
		"For least-squares, choose between efficient 'WLS' encoder (default), 'LS' (without weighting), and faster 'FASTLS' predictor. Other predictors contain 'NLM' (non-local means), 'MED' (LOCO-I median predictor), and 'MEAN' (average of neighborhood pixels) predictors.")));
	parameters.insert(pair<string, GenericParameter*>("neighborhood_top", new Parameter<double>(2.5, 0,
		"Size of ellipse neighborhood (top).")));
	parameters.insert(pair<string, GenericParameter*>("neighborhood_left", new Parameter<double>(3.0, 0,
		"Size of ellipse neighborhood (left).")));
	parameters.insert(pair<string, GenericParameter*>("neighborhood_right", new Parameter<double>(3.5, 0,
		"Size of ellipse neighborhood (right).")));
	parameters.insert(pair<string, GenericParameter*>("neighborhood_bottom", new Parameter<double>(0.0, 0,
		"Size of ellipse neighborhood (bottom).")));
	parameters.insert(pair<string, GenericParameter*>("neighborhood_front", new Parameter<double>(0.0, 0,
		"Size of ellipse neighborhood (front).")));
	parameters.insert(pair<string, GenericParameter*>("training_size", new Parameter<int>(16, 0,
		"Size of training region (maximum pixel distance for values incorporated to training).")));
	parameters.insert(pair<string, GenericParameter*>("training_size_3D", new Parameter<int>(0, 0,
		"For 3-D training region: configure number of slices to include for training: 0 := only 2-D training region.")));
	parameters.insert(pair<string, GenericParameter*>("variance", new Parameter<string>("LS", 0,
		"Estimation of pixel intensities variance: RESIDUAL (from prediction error context), EXPONENTIAL (fast, default), or LS (analytically with LS).")));
	parameters.insert(pair<string, GenericParameter*>("variance_radius", new Parameter<double>(4.5, 0,
		"For variance == RESIDUAL define size of context region from which to estimate.")));
	parameters.insert(pair<string, GenericParameter*>("adaptive_transposition", new Parameter<bool>(1, 0,
		"Code transposed image if edges are rather horizontal.")));
	parameters.insert(pair<string, GenericParameter*>("max_image_size", new Parameter<int>(8192, 0,
		"Defines the maximum pixel extent of an image in x-, y-, and z-direction: for best performance choose about eight times the typical image size.")));
	parameters.insert(pair<string, GenericParameter*>("inter_channel_prediction", new Parameter<int>(2, 0,
		"For multi-channel images, include corresponding pixel positions in previously transmitted channels into the prediction neighborhood.")));
	parameters.insert(pair<string, GenericParameter*>("wls_variance_equation", new Parameter<int>(1, 0,
		"Equation to be used for variance prediction in WLS: 0 is equivalent to LS derivation, 1 uses squared weights (achieves better compression ratios)")));
	parameters.insert(pair<string, GenericParameter*>("other_matching_neighborhood", new Parameter<double>(0.0, 0,
		"If larger than zero, use another neighborhood size (circle neighborhood) for matching to compute weights in WLS. This is useful if the image contains recurring structures. Attention: This has only an effect if it is greater than neighborhood_XXX sizes!")));
	parameters.insert(pair<string, GenericParameter*>("max_training_vectors", new Parameter<int>(0, 0,
		"If larger than zero, allow no more than this number of weights to be larger than zero in WLS. This is useful for non-local training (large training_size).")));
	parameters.insert(pair<string, GenericParameter*>("solver", new Parameter<int>(3, 0,
		"Choose algorithm for solving linear system of equations: DECOMP_CHOLESKY (3) leads to faster solutions than DECOMP_QR (4) but sometimes decreases compression efficiency.")));
	parameters.insert(pair<string, GenericParameter*>("border_regularization", new Parameter<double>(1.0, 0,
		"Choose Tikhonov regularization strength for border image pixels.")));
	parameters.insert(pair<string, GenericParameter*>("inner_regularization", new Parameter<double>(0.1, 0,
		"Choose Tikhonov regularization strength for inner image pixels (inner regularization is only possible if also border regularization is done).")));
	parameters.insert(pair<string, GenericParameter*>("neighborhood_buffer", new Parameter<bool>(1, 0,
		"Buffer already computed neighborhoods: increases speed (especially for LS and WLS) but requires significantly more memory during execution (especially with large images).")));
	parameters.insert(pair<string, GenericParameter*>("sparsify_distribution", new Parameter<double>(0.5, 0,
		"Make the distribution sparse, i. e., remove high probabilities for intensities that were observed not very often previously: recommended for non-natural images.")));
	parameters.insert(pair<string, GenericParameter*>("sparsification_size", new Parameter<double>(60.0, 0,
		"When sparsify_distribution is used, define radius of context from whitch to estimate sparsity: larger contexts increase computational complexity.")));
	parameters.insert(pair<string, GenericParameter*>("distribution", new Parameter<string>("T", 0,
		"Use the more precise T-Distribution for better compression ratio like derived in the paper or a Laplace distribution which performs worse for least-squares.")));
	parameters.insert(pair<string, GenericParameter*>("regularization_distribution", new Parameter<string>("LAPLACE", 0,
		"Additive regularization distribution is necessary to prevent too low probabilities that could cause infinitely long code words if the symbol occurs (LAPLACE suggested).")));
	parameters.insert(pair<string, GenericParameter*>("max_bits_per_pixel", new Parameter<int>(30, 0,
		"Upper bound for the number of bits one compressed pixel may occupy: necessary to prevent extremely large storage demands for improbable intensities. Attention: Typically the mean bits per pixel performance will drop when defining this value too big or too small. Attention: Do not use more than bits of (RANGETYPE - 2) or more than 50.")));
	parameters.insert(pair<string, GenericParameter*>("max_to_min_regularization_ratio", new Parameter<double>(10000.0, 0,
		"Closely related to MAX_BITS_PER_PIXEL: defines the storage demand ratio between improbable intensities and most improbable intensities.")));
} // end Config::insertMoreParameters

void Config::checkConfig() {
	// check if config seems valid
	if((get<string>("input") == "" && get<string>("bitstream") == "") || // at most an output filename or "show" was specified
		(get<string>("bitstream") == "" && get<string>("output") == "" && !get<bool>("show")) || // at most an input filename was specified
		(get<string>("input") == "" && get<string>("output") == "" && !get<bool>("show"))) { // at most a bitstream filename was specified
			cout << "Input and output filenames are neither given in the config file nor as command line arguments. Starting interactive mode." << endl << endl;
			cout << "If you would like to encode and/or show an image, please specify its filename. (otherwise directly press <return>):" << endl << ">> ";
			string buffer; getline(cin, buffer); set("input", buffer);
			if(get<string>("input") == "") // no input -> assume decoding
				cout << endl << "Please specify the filename of the bitstream to be decoded:" << endl << ">> ";
			else cout << endl << "If you would like to encode the image, please specify the filename of the encoded bitstream (otherwise directly press <return>):" << endl << ">> ";
			getline(cin, buffer); set("bitstream", buffer);
			if(get<string>("bitstream") == "") {
				if(get<string>("input") == "") { // still, at most an output filename or "show" was specified
					cerr << endl << "Neither the filename of an input image nor the filename of a bitstream was given. Nothing is to be done." << endl;
					throw ConfigNotValidException();
				}
			} else {
				cout << endl << "If you would like to decode and write the decoded image to a file, please specify the filename (otherwise directly press <return>):" << endl << ">> ";
				getline(cin, buffer); set("output", buffer);
			}
			cout << endl;
			if(get<string>("output") == "" && (get<string>("bitstream") == "" || get<string>("input") == "")) {
					cout << "No filename has been specified. The image will be shown on screen." << endl;
					set("resize", true); set("show", true); // assume that the user wants to show the image
			}
				
	}
	if(get<string>("predictor") != "MEAN" && get<string>("predictor") != "MED" && get<string>("predictor") != "NLM" && get<string>("predictor") != "FASTLS" && get<string>("predictor") != "LS" && get<string>("predictor") != "WLS") {
		cerr << "Predictor not known." << endl;
		throw ConfigNotValidException();
	}
	if(get<string>("predictor") == "NLM" && (get<double>("neighborhood_top") != 1.5 || get<double>("neighborhood_left") != 1.5 || get<double>("neighborhood_right") != 1.5 || get<double>("neighborhood_bottom") != 0.0 || get<double>("neighborhood_front") != 0.0))
		cout << "Warning: you are not using the originally suggested neighborhood sizes of 1.5 for the NLM predictor (zero for bottom and front)." << endl;
	if(get<string>("predictor") == "MED" && (get<double>("neighborhood_top") != 1.5 || get<double>("neighborhood_left") != 1.5 || get<double>("neighborhood_right") != 0.5 || get<double>("neighborhood_bottom") != 0.0 || get<double>("neighborhood_front") != 0.0)) {
		cerr << "For the MED predictor you need to set the neighborhood sizes to neighborhood_top=1.5 / neighborhood_left=1.5 / neighborhood_right=0.5 pixels (zero for bottom and front)." << endl;
		throw ConfigNotValidException();
	}
	if(get<string>("variance") != "RESIDUAL" && get<string>("variance") != "EXPONENTIAL" && get<string>("variance") != "LS") {
		cerr << "Variance estimator not known." << endl;
		throw ConfigNotValidException();
	}
	if(get<int>("max_image_size") > 40000)
		cout << "Warning: is is not guaranteed that images with a size larger than 40000 pixels can be coded without problems." << endl;
	if(get<int>("inter_channel_prediction") && get<double>("neighborhood_front") > 0.0 && get<int>("training_size_3D")) {
		cout << "Warning: inter_channel_prediction is not possible with 3D neighborhood and/or training region. Setting to zero." << endl;
		set("inter_channel_prediction", 0);
	}
	if(get<double>("other_matching_neighborhood") > 0.0 && (get<double>("other_matching_neighborhood") < get<double>("neighborhood_top") || get<double>("other_matching_neighborhood") < get<double>("neighborhood_left") || get<double>("other_matching_neighborhood") < get<double>("neighborhood_right"))) {
		cout << "Warning: other_matching_neighborhood is smaller than neighborhood_XXX but larger than zero - this is not supported, yet. Deactivating other_matching_neighborhood." << endl;
		set("other_matching_neighborhood", 0.0);
	}
	if(get<string>("distribution") != "NORMAL" && get<string>("distribution") != "T" && get<string>("distribution") != "LAPLACE" && get<string>("distribution") != "UNIFORM") {
		cerr << "Distribution not known." << endl;
		throw ConfigNotValidException();
	}
	if(get<string>("regularization_distribution") != "UNIFORM" && get<string>("regularization_distribution") != "LAPLACE" && get<string>("regularization_distribution") != "NORMAL" && get<string>("regularization_distribution") != "T") {
		cerr << "Regularization distribution not known." << endl;
		throw ConfigNotValidException();
	}
	#ifndef BOOST
		#ifdef WIN32
			if(get<string>("distribution") == "T" || get<string>("distribution") == "NORMAL") {
				cout << "Warning: Boost library support has been deactivated in vanilcConfig.cpp. Using Laplace distribution instead. This will impair the compression ratio!" << endl;
				set("distribution", "LAPLACE");
			}
			if(get<string>("regularization_distribution") == "T" || get<string>("regularization_distribution") == "NORMAL") {
				cout << "Warning: Boost library support has been deactivated in vanilcConfig.cpp. Using Laplace regularization distribution instead. This will impair the compression ratio!" << endl;
				set("regularization_distribution", "LAPLACE");
			}
		#else
			if(get<string>("distribution") == "T") {
				cout << "Warning: Boost library support has been deactivated in vanilcConfig.cpp. Using normal distribution instead. This will impair the compression ratio!" << endl;
				set("distribution", "NORMAL");
			}
			if(get<string>("regularization_distribution") == "T") {
				cout << "Warning: Boost library support has been deactivated in vanilcConfig.cpp. Using normal regularization distribution instead. This will impair the compression ratio!" << endl;
				set("regularization_distribution", "NORMAL");
			}
		#endif
	#endif
} // end Config::checkConfig

void Config::parse(int argc, char* argv[]) {
	// initialize command line parser
	string keys = "{";
	for(map<string, GenericParameter*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
		keys += string("{") + (it->second->getLetter() ? string("") + it->second->getLetter() : string(""))
			+ "|" + it->first + "|" + it->second->getValue() + "|" + it->second->getDescription() + "}";
	keys += "}";
	CommandLineParser parser(argc, argv, keys.c_str());

	if(parser.get<bool>("version")) {
		cout << "Version 1.0" << endl;
		throw InfoShownException();
	}

	if(parser.get<bool>("help")) {
		parser.printParams();
		throw InfoShownException();
	}

	insertMoreParameters();

	// read YAML configuration file
	FileStorage fs; bool isOpen = false;
	if(parser.get<string>("config") != "") {
		isOpen = fs.open(parser.get<string>("config"), FileStorage::READ);
	} else {
		isOpen = fs.open("config.yml", FileStorage::READ);
	}
	if(!isOpen) isOpen = fs.open("../config.yml", FileStorage::READ);
	if(!isOpen) isOpen = fs.open("../../config.yml", FileStorage::READ);
	if(isOpen) {
		for(map<string, GenericParameter*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
			it->second->read(fs, it->first);
		fs.release();
	} else cout << "Warning: config file not available. Using default configuration." << endl << endl;

	// parse the command line again in order to replace default values from config file with values from the command line
	keys = "{{}";
	for(map<string, GenericParameter*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
		keys += string("{") + (it->second->getLetter() ? string("") + it->second->getLetter() : string(""))
			+ "|" + it->first + "|" + it->second->getValue() + "|" + it->second->getDescription() + "}";
	keys += "}";
	parser = CommandLineParser(argc, argv, keys.c_str());

	if(parser.get<bool>("hh")) {
		parser.printParams();
		throw InfoShownException();
	}

	for(map<string, GenericParameter*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
		it->second->parse(parser, it->first);

	if(get<bool>("resize")) set("show", true);

	checkConfig();
} // end Config::parse

void Config::printConfig() {
	for(map<string, GenericParameter*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
		cout << it->first << " = " << it->second->getValue() << ", ";
} // end Config::printConfig

} // end namespace vanilc

