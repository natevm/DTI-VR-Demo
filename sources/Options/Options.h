/*******************************************************
 ** Generalized Voronoi Diagram Project               **
 ** Copyright (c) 2015 John Martin Edwards            **
 ** Scientific Computing and Imaging Institute        **
 ** 72 S Central Campus Drive, Room 3750              **
 ** Salt Lake City, UT 84112                          **
 **                                                   **
 ** For information about this project contact        **
 ** John Edwards at                                   **
 **    edwardsjohnmartin@gmail.com                    **
 ** or visit                                          **
 **    sci.utah.edu/~jedwards/research/gvd/index.html **
 *******************************************************/

#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>

 //------------------------------------------------------------------------------
 // Options
 //------------------------------------------------------------------------------
namespace Options {
  extern int computeDevice;
  extern std::vector<std::string> filenames;
	extern std::string volumeLocation;
	extern int volumeWidth, volumeHeight, volumeDepth;
	extern int bytesPerPixel;
	extern int totalSamples;
  extern bool series;

  // Render settings
  extern bool showObjectVertices;
	extern bool showTransferFunctionGUI;

	extern bool debug;
  extern bool benchmarking;
  extern std::string cl_options;
	extern bool fullScreen;

  bool ProcessArg(int& i, char** argv);
	int ProcessArgs(int argc, char** argv);
};