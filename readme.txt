License
-------
Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

The directory "3rdParty" contains source code under different licenses. For more information, please read the license information in the individual files.

Purpose
-------
The name Vanilc is an acronym for "Volumetric, Artificial, and Natural Image Lossless Coder". As the name suggests, the program is able to compress various types of images losslessly, i. e., without any distortion of the image quality, ranging from very fast to very efficient compression (in terms of bits per pixel). The main purpose of this application is to serve as a prove-of-concept for the methods described in [1] regarding the distribution estimation step in least-squares prediction. However, as some effort was invested in order to build a fast and clean object-oriented open source C++ implementation, it may certainly also serve as a research framework for pixel-wise image and volume prediction for lossless image coding.

Since CMake [2] was used as a meta build system, the software should be more or less platform independent. Development was done on a SuSe Linux system but basic functionality was also tested on Microsoft Windows 7 with Visual Studio 2010. The implementation depends on OpenCV [3] as well as on the Boost [4] library. The original package from the repository is packaged with parts of these libraries such that no other downloads are necessary. However, if the libraries are already installed on the system, the implementation can also be linked to these system-libraries.


Installation
------------
1. If not already available, first install CMake [2].

2. On Linux, install GCC (tested with version 4.7), as well as the standard development packages for C++. If you would like to show images, also install the development package for libgtk 2.0. On Windows 7, install Visual Studio 7.

3. If you would like to use the system-versions of OpenCV and Boost, first make sure that these are correctly installed on your system.

4. Create the build files using CMake: On Linux start a command line and change to the "bin" sub-directory of this package. Then type "cmake .." (without the quotes) or "ccmake .." for a ncurses interface where you can set custom options. On Windows you may use the GUI version of CMake and then set the main directory of this package as the source code folder and the "bin" sub-directory as the folder where to build the binaries. For the ncurses as well as for the GUI version, first hit "configure", then set your options, again hit "configure" and finally "generate". You can also deactivate "VANILIC_USE_ATTACHED_BOOST" and "VANILIC_USE_ATTACHED_OPENCV" in order to rather use system libraries. In order to save compile time, you may also want to remove the support for some specific image formats like JPEG 2000 (JASPER), TIFF, and/or PNG (PGM and PPM is always available).

5. Compile the project: On Linux type "make", on Windows start Visual Studio, open the solution file, select "Release" as the build type and click on "Build Solution". After successful compilation you will find a file called "vanilc" or "vanilc.exe" in the "bin" or "bin/Release" directory.

6. (only for Windows:) If using an own installation of OpenCV, make sure that the OpenCV build directory containing the DLL files is in your PATH or, alternatively, copy the files "opencv_core249.dll", "opencv_highgui249.dll", and "opencv_imgproc249.dll" to "bin/Release".


Usage
-----
In order to start Vanilc with its default configuration, open the command line, change to the "bin" (or "bin\Release") directory with the "vanilc(.exe)" executable and type

(Linux): ./vanilc -i=/path/to/the/image/to/be/compressed.pgm -b=/path/to/the/compressed/binary/file.vanilc
(Windows): vanilc.exe -i=C:\path\to\the\image\to\be\compressed.pgm -b=C:\path\to\the\compressed\binary\file.vanilc

to compress an image and

(Linux): ./vanilc -b=/path/to/the/compressed/binary/file.vanilc -o=/path/to/the/reconstruction/image/filename.pgm
(Windows): vanilc.exe -b=C:\path\to\the\compressed\binary\file.vanilc -o=C:\path\to\the\reconstruction\image\filename.pgm

to reconstruct it (replace paths with existing paths and filenames). With the "-r" flag it is also possible to show the image in a graphical window.

Attention: the default configuration is an efficient one so it may take several minutes or even hours to compress one image!

For an overview on basic command line options just specify the "-h" option ("./vanilc -h"). For a full list specify "-hh".

All command line options can also be specified in a YAML configuration file. By default, Vanilc searches for "config.yml" in the current directory and the two directories above the current directory. Other files / directories may be specified using "-c=/path/to/configFile.yml". Example config files with all possible options and their descriptions, e. g., for "fast", "efficient", and "3D" coding, as well as for the "mean" and the "NLM" predictors can be found in the "configs" directory. The "fast" and the "efficient" configurations implement the two methods tested in [1]. The corresponding mean value predictors were presented in [5] and [6], respectively. "NLM" was described in [7].

Command line options always have priority over config file definitions. When not enough input / output is specified either on command line or in the config file, an "interactive mode" is invoked. Here you can specify input and output files in an interactive way on the command line. This is especially useful if you only type "./vanilc" or simply "click" on the executable in a graphical environment.


FAQ
---
----- Usage Problems -----
1) OpenCV cannot be found by CMake.
	The sub-directory "3rdParty" should contain a directory called "opencv". If this directory is available, the option "VANILC_USE_ATTACHED_OPENCV" should be active in CMake. If the version of the included OpenCV does not work you need to install OpenCV manually. On most Linux systems you can simply install the OpenCV developer files using your package manager (e. g., using "sudo apt-get install libopencv-dev" in Ubuntu). On Windows and if you do not have root privileges on Linux, you can download it either in a source code version or as final build. Note that we have only tested the implementation with OpenCV in version 2.4.9, so in case of problems make sure that you use exactly this version. The source code version needs to be configured using CMake and then be compiled. For more information refer to [3]. In either case you must then deactivate "VANILC_USE_ATTACHED_OPENCV", configure, and set the "OpenCV_DIR" variable to the OpenCV build path which contains the file "OpenCVConfig.cmake" or "opencv-config.cmake". After another configure, OpenCV should have been found.

2) Boost cannot be found by CMake.
	The sub-directory "3rdParty" should contain a directory called "boost". If this directory is available, the option "VANILC_USE_ATTACHED_BOOST" should be active in CMake. If the version of the included Boost does not work you need to install Boost manually. On most Linux systems you can simply install the Boost developer files using your package manager (e. g., using "sudo apt-get install libboost-all-dev" in Ubuntu). On Windows and if you do not have root privileges on Linux, you can download it from [4]. Note that we have only tested the implementation with Boost in versions 1.55 and 1.56, so in case of problems make sure that you use exactly one of these versions. Vanilc does not make use of any Boost module which needs to be compiled. You must then deactivate "VANILC_USE_ATTACHED_BOOST", configure, and set the "Boost_INCLUDE_DIR" variable to the path that contains the "boost" directory with the header files. After another configure, Boost should have been found.

3) When I click on the .exe file, a console window opens and closes again immediately.
	When you click on the executable in a graphical environment, the command line opens, the program runs and when it is finished this window closes immediately. If the default config is used, the click should start the "interactive mode", waiting for an user input. However if there is an error or if another config is used, the output disappears immediately in this way. In order to see what happened, you should rather first open a command line window, change to the directory which contains the executable, and run it using the command given in section "Usage". If indeed an error occurred, you should see a detailed description now. Otherwise see question (5).

4) Upon program start, I get a message that a DLL file is not available.
	After Vanilc has been compiled, three OpenCV DLL files for the three modules "core", "highgui", and "imgproc" should also reside in the "Release" directory (see also section "Installation", item 6). If these are missing, you should either set the PATH to the directory containing them, or copy them to the directory with the "vanilc.exe" file.

5) A runtime error occurs and I don't know what to do.
	Typically, an error message should give you a hint what the problem is. If not, the situation is more sophisticated. Since this software was built for research, it was not possible to make thorough tests, especially on Microsoft Windows. If the error happens only for special input data or special configurations, you may have found a bug. It would be nice if you could send a report to the E-Mail address given on the download page.
	The following information should be included: Which Operating System did you use? In which version? What version of Vanilc did you use? How and with which compiler did you compile it? What is the command line call that leads to the crash? It would be most helpful if you could also send the config file you used as well as the image file you tried to compress. We will do our best to fix the error!
	One important exception is the following error upon decoding: "terminate called after throwing an instance of 'vanilc::EndOfBitstreamException'". This probably means that there is a so-called encoder-decoder mismatch, i. e., the decoder probably does something else than the encoder. Here you should make sure that exactly the same configuration is used for encoding and decoding!

----- Feature Questions -----
6) Is it possible to use Vanilc without OpenCV and Boost?
	OpenCV is the basic software framework Vanilc is built on, so OpenCV is mandatory.
	Yet, it is possible to use it without Boost. To do so, open the file include/vanilcDefinitions.h, comment the line "#define BOOST" by adding two slashes in front of it ("//#define BOOST"), and compile Vanilc again. In order to prevent CMake errors, you need to activate "VANILC_USE_ATTACHED_BOOST" in CMake. However it is not advisable to renounce Boost since this will prevent you from using t distribution functions and on Windows also from using normal distribution functions and thus remarkably hurt the compression ratio!

7) Which image file types are supported? Can I compress my JPEG images better using Vanilc?
	The supported file types depend exclusively on the OpenCV version in use. PGM and PPM files are supported in any case. JPEG 2000, TIFF, and PNG are also activated by default. For 3-D volume images an additional support for RAW files was implemented (see command line option "-d" / "dimensions") that may not contain a header but only the raw binary pixel data.
	Since Vanilc is a lossless coder, you should be careful using lossy image file formats like JPEG: If you use such lossy files as an input, you will probably achieve no additional compression but you will rather "loose compression" (the files will become larger). This is due to the fact that the JPEG system introduces some "loss" (degrades image quality) exactly in a way that enables the JPEG coder to achieve an extra compression gain. However, Vanilc is not able to exploit such a loss in the same way for its own compression system since it does not know how this loss exactly looks like (it rather needs to be able to compress all types of images - note, that in comparison to Vanilc, the JPEG coder knows the original image before the loss / distortion and then has the freedom to decide how to introduce distortion in order to achieve maximum compression performance!).
	If you use lossy file formats like JPEG as an output, the advantage of lossless compression will be lost. If you want to do such a compression to a lossy format afterwards, you will probably do better in not using Vanilc at all but rather directly using only the lossy coder (e. g., JPEG).

8) Can the program handle images with higher color depth than 8 bit / 24 bit?
	Yes! The basic algorithm has no restriction in bit depth. Though, we decided to restrict the current implementation to a maximum of 16 bits per pixel (per channel) since the arithmetic coder gets really slow and memory consuming when using even more gray levels / colors. For your 16 bit input images you can use, e. g., the PGM format (for gray level images) or the PPM format (for color images).

9) How can I compress 3-D volume images?
	For 3-D volume images, e. g., used in medical computed tomography or magnetic resonance imaging, you first need to convert the input volume to a raw image format. The file shall have no header and contain the voxels (3-D pixels) row by row and then slice by slice in unsigned 8 bit integer or unsigned 16 bit integer format (little endian). Afterwards you can compress the image with a call like "./vanilc -i=/path/to/the/image/to/be/compressed.raw -b=/path/to/the/compressed/binary/file.vanilc -d=8x512x512x10" for an unsigned 8 bit integer volume with 10 slices each built out of 512-by-512 voxels (this raw file should have a size of exactly 2621440 bytes). The decoder call is the same as for 2-D images. If you want to exploit the 3-D structure of such volumes for an additional compression gain, you should consider to use a specialized config file: have a look on "config_3d.yml" in the "configs" directory!

10) Is it possible to compress near infrared and multi-channel images?
	Yes! Due to its inter-channel prediction capability, Vanilc should also perform very well on these types of images. Internally, there is no difference between 3-D volume images and multi-channel images. However, to make full use of these inter-channel prediction capabilities, you need to use an OpenCV file format which supports more than three channels. When using RAW files as described in question (9), a different prediction system will be used that is not much suitable for multi-channel images!
	Note, that due to this equal processing of 3-D volume images and multi-channel / color images, it is currently not supported to compress 3-D color volumes in an efficient way. You should either use a non-3-D config file here or compress each channel volume on its own.

11) Can I do other nice things with Vanilc?
	Yes! Instead of or in addition to defining an output file name for the decoded image file you can show the decoded image in a window using the "-s" option (to display in original size) or the "-r" option (to display rescaled to the current window size).
	When you only specify input ("-i") and one of the "show" options, no coding is done and thus Vanilc can be used as a simple image viewer.
	When you instead specify input ("-i") and output ("-o"), also no coding is done and thus Vanilc can be used as a simple image format conversion tool.
	Finally, you can of course batch-process a larger number of images. If you want to suppress the command line output, use the ("-q") option. For example, in BASH a whole directory of PPM images can be compressed by using "for i in *; do /path/to/vanilc -i=${i} -b=$(basename ${i} .ppm).vanilc -q; done".

12) Can I watch the program do the predictive compression?
	Yes! However, as this would introduce more computational complexity to the basic implementation, you need to re-compile Vanilc with this option enabled: open "include/vanilcDefinitions.h" with a text editor. Remove the two slashes in front of the line "#define OBSERVEENCODING" and in front of one of the lines "#define SHOW_*". Depending on this choice one of these images is shown and updated in a window as the compression progresses. With the option "OBSERVATION_UPDATE_INTERVAL" you may also define after how much lines the image is updated. Then compile again and run Vanilc. After the decoding has finished, press <ESC> or "Q" (possibly you need to adjust the keycodes for these keys in "config.yml" - remove the slashes in front of "#define DEBUGOUT" and press them while an image window is open in order to find out which ones need to be used). Afterwards, you will also find a file "debug.raw" in the current directory which contains the shown image in RAW double format. You can display it, e. g., using ImageJ [8].

----- Algorithmic / Performance Questions -----
13) How to understand the background of this work and what do all the compression options mean?
	The background of the methods used in this implementation is described in [1] and the literature referenced there.
	Here we give a rough overview on Vanilc and the most important configuration options: Vanilc is based on a pixel-predictive algorithm that walks through the image pixel-by-pixel and then line-by-line where it compresses (reduces the amount of storage space occupied of) one pixel after the other. To do so, a forecast / prediction is made for each pixel intensity value based on already compressed ones to the left and above. Which pixels are used to predict the current pixel is defined by an elliptic region using the config values "neighborhood_top", "neighborhood_left", and "neighborhood_right" (the values give the maximum distance in terms of pixels in the corresponding direction). When a linear prediction using the least-squares (LS) method is applied (config options "WLS", "LS", or "FASTLS"), the way in which to predict is trained in a larger rectangular training region also of already processed pixels. Its size (in pixels) is defined by the config option "training_size". For 3-D volumes, three other size options exist for the 3rd dimension. For a good compression it is also required to estimate the certainty of the prediction made. This is done using a variance estimator. For the one proposed in [1], use "variance: "LS"". For a more traditional estimation, use the other options. The better the estimations, the better the compression result from the entropy coder will be. Note, that a larger neighborhood / training region does not necessarily mean better compressions since pixels far away from the current one are probably not very similar to it and thus make the estimation worse. "adaptive_transposition" will guess whether it is better to walk through the image column-by-column and maybe do so if enabled. "inter_channel_prediction" will extend the neighborhood to other color channels and so exploit inter-channel similarities for compression. "sparsify_distribution" will save storage space by assuming that gray values that have not been observed very often in the previous image, are unlikely to occur at the current pixel even if the prediction says so. This is especially useful for artificial images where you could try a value of about 2 for this option. On natural images it will not make a big difference, however, note that a value larger than zero makes the compression slower. For "distribution" the t distribution will lead to the best compression ratios [1]. Yet, a traditional normal distribution is a bit faster and almost as good. On the other hand, for the "regularization_distribution", Laplace performs best. Suitable configurations are given in the "configs" directory, so these may serve as a good starting point for own experiments.

14) Why is the compressed file size so much worse than JPEG and all the other standard image types?
	Most common image and video compression systems work in a lossy way. That means, they degrade the image quality. Vanilc, on the other hand, preserves exact pixel values so that you can be sure to see exactly the same image after decoding as before encoding. Unfortunately, this design decision has its price. Especially in noisy images, the compression ratio is much worse than for lossy methods since image noise is typically pure random (unpredictable from neighboring pixels) and therefore very costly if you want it to be preserved. Note, however, that Vanilc does typically still perform much better than general-purpose archiving systems like ZIP, 7ZIP, RAR, or GZIP.

15) Why is it not possible to do lossy compression then?
	The simple answer is: it was a design decision because pixel-predictive methods are not much suitable for lossy compression. The typical way to go is to round the current intensity value towards the prediction in order to save bits. However, since this degrades image quality, it also degrades the accuracy of following predictions which are dependent on a good image quality. Thus, the saved bit rate must more or less be paid back in following parts of the image. Most lossy methods apply a completely different compression scheme.

16) Why is the decoded image file not identical to the original input file?
	Vanilc is to be seen as a compression format for images like lossless PNG or TIFF. It should not be confused with a general-purpose file archiver like ZIP or GZIP. Thus, Vanilc will not try to compress meta data such as comments in PPM image files. Note, that this does in no way impair image quality. Even if the image header changes, the pixels in the image remain exactly the same!
	For support of various file types we make use of the OpenCV image reading and writing capabilities. In this way it is no problem to encode a TIFF image and afterwards decode it to a PPM image. Also from this point of view, it is not useful to remember the original file format and its header information in the compressed file. If you want the same files, we suggest that you write a script for your specific file format that extracts the header information, stores them using a general-purpose archiver and then, after decoding, writes them back to the decoded image file.

17) Is this the best lossless coder available at the moment or is there a better one?
	That depends solely on the definition of "best": Do you mean "best" in terms of compression ratio? In terms of encoding speed? In terms of decoding speed? In terms of features? In terms of usability? In terms of platform independence? Actually, an overview and comparison with some other state-of-the-art methods can be found in [1]. To summarize: Vanilc is certainly not the fastest coder available (but also not the slowest). Its compression ratio with the "WLS" predictor is among the currently best state-of-the-art coders. Compared to most other lossless coders, it provides 3-D, 16 bit, multi-channel, and support for various file types. It was implemented as a fast, clean, and easily extensible C++ project, published as open source. Due to CMake it is platform independent and may thus be used on various architectures and operating systems (although, only tested on Linux and Windows, yet). We hope that the command line interface, standard YAML config files, an interactive mode, and few library dependencies make it easy to use.

18) Why are the file sizes larger than mentioned in [1]?
	Probably you have used a different version of Vanilc, a different configuration, or different input images. In order to reproduce the results in the paper, please use the implementation in version 1.0 "as is", combined with the config files "configs/config_efficient.yml" and "configs/config_fast.yml". The original input images can be found in the internet and are referenced in [1].

19) Why is decoding slower than encoding?
	In the current implementation the (non-binary) arithmetic entropy coder needs to do a binary search among all possible symbols (intensity values) for each pixel in order to find the encoded symbol. This effect strengthens with increasing bit depth / increasing number of possible intensity values.
	It is also possible to use Rice-Golomb coding instead of arithmetic coding which does not suffer from this binary search. This will also decrease encoder complexity, so you will realize a significant speed gain especially for fast prediction methods (several seconds per image, depending on image size). On the other hand, Rice-Golomb coding causes a decrease of compression performance because of two reasons: First, it always assumes a Laplace probability distribution which does not match the theory very well (see [1]). Second, it performs a 1-to-1 mapping from symbols to bit sequences. Therefore, it is not possible to approach the entropy - which is a non-integer number of bits - very well. This carries weight particularly in images with low noise like computer-generated images where a pixel often would consume less than a bit with arithmetic coding. You can switch to Rice-Golomb coding in "include/vanilcDefinitions.h" by removing the slashes in front of "#define GOLOMB_CODING" and adding them in front of "#define ARITHMETIC_CODING". Make sure to re-compile Vanilc.

20) Why does Vanilc run slower on my PC than mentioned in [1]?
	Naturally, the execution speed highly depends on the hardware used in the testing system. Thus, the numbers in the paper should only be seen in comparison with the other implementations. If the speed is very slow, first make sure that Vanilc was compiled in "Release" mode and all instruction extensions of your CPU are active in CMake. Furthermore, make sure that enough random access memory is available in order to prevent swapping (otherwise try with smaller images). Please also see the comments on question (18). When the PC is idling, you are not using interactive mode, do not show the image in a window and "Overall time" and "CPU time" are much different from each other, you should try to read and write the images from/to a ram disc in order to make sure that a slow mass storage medium is not causing the large overall time. Also notice that the implementation might run slower on Microsoft Windows than on our Linux test system.

21) Why does Vanilc run faster with the provided OpenCV code compared to my pre-installed system (e. g., packaged Linux distribution) version?
	One possible reason is that we decided to disable the CV_Assert Macro in modules/core/include/opencv2/core/core.hpp for release mode compiled with the NDEBUG definition. That is the only change we made in the OpenCV source code since it will save several percents of runtime.
	Another possibility is that the system OpenCV code is probably pre-compiled for legacy computer systems whereas a self-compiled version, like the one delivered, will be optimized for your specific hardware.

22) How can I get the smallest compressed file size? How can I get the fastest speed?
	Basically you always need to make a trade-off between file size and execution speed. 
	If you want to have the smallest file size, the "config_efficient.yml" file is a good point to start from. For 3-D images you should choose "config_3d.yml". Especially for arificial images but sometimes also for others, it often makes sense to play around with other values for "sparsify_distribution" and possibly also "max_to_min_regularization_ratio". Sometimes also another configuration for "neighborhood_*" or for "training_size" could improve the compression ratio. In a few cases it make sense to turn off the "adaptive_transposition". If you use color or multi-channel images with other channel orders than red-green-blue, you should consider changing the "CHANNEL_ORDER" in "include/vanilcDefinitions.h" (don't forget to re-compile).
	If you want to have the fastest execution time, you probably should not compress at all. However, if you only want to become a bit "faster", you can try the config files "config_fast.yml", "config_nlm.yml", and "config_mean.yml" (in this order the compression becomes faster). In this case you should definitely also consider to use the Rice-Golomb coder instead of the arithmetic coder (see question (19)). Otherwise, "distribution: "NORMAL"" instead of "distribution: "T"" makes up a good trade-off for arithmetic coding. "sparsify_distribution" should be set to zero. You can always save additional complexity by reducing the "neighborhood_*" sizes.

23) What is the difference between "LS" and "FASTLS"?
	LS uses the same implementation as WLS but without weighting. Up to some arithmetic differences, it should lead to very similar compression results compared to FASTLS since FASTLS is only a faster implementation for LS, so FASTLS should be preferred in most cases. One other advantage of FASTLS is that it is much less dependent on the training size. However, since in contrast to WLS no weighting can be used, the compression results become worse if training- and / or neighborhood size are chosen too large.

----- Other Questions -----
24) May I use parts of the source code for my own program?
	Yes, as long as you follow the MIT license conditions stated at the beginning of this file. Particularly, this means to keep the license text in the source files and to attache the license-MIT.txt file, even if you distribute a closed-source program.
25) I want to contribute. How to start?
	Your contribution is very welcome! The first step would be to become a bit familiar with least-squares prediction and pixel-wise predictive coders in general. As a starting point [1] and the references there might be interesting for you. It is also a good idea to become familiar with the basic OpenCV programing model. Afterwards you might have a look at "src/vanilc.cpp", "include/vanilcCoder.h", and "src/vanilcCoder.cpp" which contain the basic structure of the implementation. The core algorithms described in [1] can be found in "src/vanilcWLSPredictor.cpp" and "src/vanilcFastLSPredictor.cpp".
	Much remains to be done! Other file types might be supported. Other predictors and entropy coding methods from literature might be implemented. Other coder configurations need to be tested, both in the config file, as well as in the compression pipeline implemented in vanilcCoder.cpp (e. g., try other structuring elements and weightings, especially for color and 3-D images). Also, faster LS implementations are to be done. A good near-lossless option would be nice for some other applications. Some people might find a graphical user interface helpful, too.


References
----------
[1] A. Weinlich et at., "Probability distribution estimation for autoregressive pixel-predictive image coding," IEEE Trans. Image Process., to be published.
[2] CMake build system, http://www.cmake.org/
[3] OpenCV computer vision library, http://opencv.org/
[4] Boost C++ libraries, http://www.boost.org/
[5] X. Wu et al., "Piecewise 2d autoregression for predictive image coding," in Proc. IEEE Int. Conf. Image Process., Nov. 1998, pp. 901-905.
[6] H. Ye et al., "A weighted least squares method for adaptive prediction in lossless image compression," in Proc. Picture Coding Symp., Sep. 2003, pp. 489-493.
[7] E. Wige et al., "Pixel-based averaging predictor for HEVC lossless coding," in Proc. IEEE Int. Conf. Image Process., Sep. 2013, pp. 1806-1810.
[8] ImageJ image processing and analysis tool, http://imagej.nih.gov/ij/

