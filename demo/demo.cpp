//      demo.cpp
//
//		Here is an example on how to use the descriptor presented in the following paper:
//		A. Alahi, R. Ortiz, and P. Vandergheynst. FREAK: Fast Retina Keypoint. In IEEE Conference on Computer Vision and Pattern Recognition, 2012.
//
//
//		Copyright (C) 2011-2012  Signal processing laboratory 2, EPFL,
//	    Raphael Ortiz (raphael.ortiz@a3.epfl.ch),
//		Kirell Benzi (kirell.benzi@epfl.ch)
//		Alexandre Alahi (alexandre.alahi@epfl.ch)
//		and Pierre Vandergheynst (pierre.vandergheynst@epfl.ch)
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/legacy/legacy.hpp>

#include "freak.h"
#include "hammingseg.h"

using namespace cv;

static const std::string kResPath = "../../resources/";

int main( int argc, char** argv ) {
    // check http://opencv.itseez.com/doc/tutorials/features2d/table_of_content_features2d/table_of_content_features2d.html
    // for OpenCV general detection/matching framework details

    // Load images
    Mat imgA = imread(kResPath + "images/graf/img1.ppm", CV_LOAD_IMAGE_GRAYSCALE );
    if( !imgA.data ) {
        std::cout<< " --(!) Error reading images " << std::endl;
        return -1;
    }

    Mat imgB = imread(kResPath + "images/graf/img3.ppm", CV_LOAD_IMAGE_GRAYSCALE );
    if( !imgA.data ) {
        std::cout << " --(!) Error reading images " << std::endl;
        return -1;
    }

    std::vector<KeyPoint> keypointsA, keypointsB;
    Mat descriptorsA, descriptorsB;

    std::vector< DMatch>   matches;

    // DETECTION
    // Any openCV detector such as
    SurfFeatureDetector detector(2000,4);

    // DESCRIPTOR
    // Our propose FREAK descriptor
    // (roation invariance, scale invariance, pattern radius corresponding to SMALLEST_KP_SIZE, number of octaves, file containing list of selected pairs)
    FreakDescriptorExtractor extractor(true,true,22,4, kResPath + "selected_pairs.bin");

    // MATCHER
    // The standard Hamming distance can be used such as
    // BruteForceMatcher<Hamming> matcher;
    // or the proposed cascade of hamming distance
#ifdef USE_SSE
    BruteForceMatcher< HammingSeg<30,4> > matcher;
#else
    BruteForceMatcher<Hamming> matcher;
#endif

    // detect
    double t = (double)getTickCount();
    detector.detect( imgA, keypointsA );
    detector.detect( imgB, keypointsB );
    t = ((double)getTickCount() - t)/getTickFrequency();
    std::cout << "detection time [s]: " << t/1.0 << std::endl;

    // extract
    t = (double)getTickCount();
    extractor.compute( imgA, keypointsA, descriptorsA );
    extractor.compute( imgB, keypointsB, descriptorsB );
    t = ((double)getTickCount() - t)/getTickFrequency();
    std::cout << "extraction time [s]: " << t << std::endl;

    // match
    t = (double)getTickCount();
    matcher.match(descriptorsA, descriptorsB, matches);
    t = ((double)getTickCount() - t)/getTickFrequency();
    std::cout << "matching time [s]: " << t << std::endl;

    // Draw matches
    Mat imgMatch;
    drawMatches( imgA, keypointsA, imgB, keypointsB, matches, imgMatch);


    namedWindow("matches", CV_WINDOW_KEEPRATIO);
    imshow("matches", imgMatch);
    waitKey(0);

    /////////////////////////////////////////////////
    //
    //PAIRS SELECTION
    //FREAK is available with a set of pairs learned off-line. Researchers can run a training process to learn their own set of pair.
    //For more details read section 4.2 in:
    //A. Alahi, R. Ortiz, and P. Vandergheynst. FREAK: Fast Retina Keypoint. In IEEE Conference on Computer Vision and Pattern Recognition, 2012.

    //We notice that for keypoint matching applications, image content has little effect on the selected pairs unless very specific
    //what does matter is the detector type (blobs, corners,...) and the options used (scale/rotation invariance,...)
    //reduce corrTresh if not enough pairs are selected (43 points --> 903 possible pairs)
    // Un-comment the following lines if you want to run the training process to learn the best pairs:
    /*
    std::vector<string> filenames;
    filenames.push_back(kResPath + "images/train/1.jpg");
    filenames.push_back(kResPath + "images/train/2.jpg");

    std::vector<Mat> images(filenames.size());
    std::vector< std::vector<KeyPoint> > keypoints(filenames.size());

    for( size_t i = 0; i < filenames.size(); ++i ) {
        images[i] = imread( filenames[i].c_str(), CV_LOAD_IMAGE_GRAYSCALE );
        if( !images[i].data ) {
            std::cout<< " --(!) Error reading images " << std::endl;
            return -1;
        }
        detector.detect( images[i], keypoints[i] );
    }
    extractor.selectPairs(images, keypoints, kResPath + "selected_pairs2", 0.7);
    */
}
