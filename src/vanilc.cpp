// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "vanilcDefinitions.h"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>

#include "vanilcConfig.h"
#include "vanilcCoder.h"
#include "vanilcPredictor.h"
#include "vanilcTimer.h"
#include "vanilcRawIO.h"

using namespace cv;
using namespace vanilc;
using namespace std;

int main(int argc, char* argv[]) {
	Timer timer;
	Config config;
	try { config.parse(argc, argv); } // parse input arguments
	catch (InfoShownException&) { return EXIT_SUCCESS; } // help or version information has been printed
	catch (Exception& e) { cerr << e.what() << endl; return EXIT_FAILURE; } // input arguments not valid
	bool verbose = !config.get<bool>("quiet");
	if(verbose) cout.setf(ios::unitbuf); // unbuffered output: show cout commands instantaneosly
	#ifdef DEBUGOUT
		cout << "The coder has been configured as follows: ";
		config.printConfig();
		cout << endl;
	#endif

	Mat image;

	if(config.get<string>("input") != "") { // load image
		if(config.get<string>("dimensions") == "")
			image = imread(config.get<string>("input"), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
		else {
			RawIO rawReader;
			image = rawReader.imread(config.get<string>("input"), config.get<string>("dimensions"));
		}
		if(!image.data) {
			cerr << "It was not possible to read the image. Note that for Linux, ~ for the home directory does not work currently." << endl;
			return EXIT_FAILURE;
		}
		if(config.get<string>("bitstream") != "") { // encode image to bitstream
			Coder vanilccoder(config);
			vanilccoder.setImage(image);
			if(verbose) cout << "Encoding ";
			vanilccoder.code(1);
			unsigned int filesize = vanilccoder.writeBitstreamToFile(config.get<string>("bitstream")); // returns size in bytes
			double bpp = (double)filesize * 8.0 / (double)image.total() / (double)image.channels();
			if(verbose) {
				cout << "Encoding successfully finished." << endl << endl;
				ifstream fs(config.get<string>("input").c_str(), ios::binary | ios::ate);
				cout << "Average bits per pixel: " << bpp << " (compression factor " << (double)fs.tellg() / (double)filesize << ")" << endl;
				fs.close();
				cout << "Compressed file size: " << filesize  << " B / " << (double)filesize / 1024.0 << " kiB / " << (double)filesize / (1024.0 * 2024.0) << " MiB" << endl;
				cout << endl;
			}
		}
	}

	if(config.get<string>("bitstream") != "" && (config.get<string>("output") != "" || config.get<bool>("show"))) { // decode bitstream to image
		Coder vanilccoder(config);
		vanilccoder.readBitstreamFromFile(config.get<string>("bitstream"));
		if(verbose) cout << "Decoding ";
		vanilccoder.code(0); // decode image
		vanilccoder.getImage(image);
		if(verbose) cout << "Decoding successfully finished." << endl << endl;
	}

	int delay = 0;
	if(config.get<string>("output") != "") // write image to output file
		try {
			if(image.dims == 3) {
				RawIO rawWriter;
				rawWriter.imwrite(config.get<string>("output"), image);
			} else imwrite(config.get<string>("output"), image);
		} catch (Exception&) {
			cerr << "It was not possible to write the image. Trying to show it on screen for five minutes." << endl;
			config.set("resize", true); config.set("show", true); // assume that the user wants to show the image
			delay = 300000; // restrict to 5 minutes so that test series scripts continue to run when it was impossible to write one image
		}

	if(verbose) cout << "Overall time: " << timer.getREALtime() << "s, CPU time: " << timer.getCPUtime() << "s" << endl;

	if(config.get<bool>("show")) { // show image
		string title = config.get<string>("input");
		if(title == "") title = config.get<string>("bitstream");
		if(config.get<bool>("resize")) namedWindow(title, CV_WINDOW_NORMAL);
		else namedWindow(title, CV_WINDOW_AUTOSIZE);
		if(image.depth() == CV_16U) {
			double minval, maxval;
			minMaxIdx(image, &minval, &maxval);
			#ifdef WIN32
				imshow(title, image * (ushort)(1 << (16 - (unsigned int)(log(maxval) / log(2.0) + 1.0))));
			#else
				imshow(title, image * (ushort)(1 << (16 - (unsigned int)(log2(maxval) + 1.0))));
			#endif
		} else imshow(title, image);
		int pressedKey = -2;
		cout << "Press Q or <ESC> to quit." << endl;
		while(pressedKey != -1 && pressedKey != config.get<int>("keycode_q") && pressedKey != config.get<int>("keycode_esc")) {
			pressedKey = waitKey(delay);
			#ifdef DEBUGOUT
				cout << "Keypress detected - keycode: " << pressedKey << endl;
			#endif
		}
	}

	return EXIT_SUCCESS;
}

