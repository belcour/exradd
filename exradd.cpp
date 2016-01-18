// Include STL
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <exception>

using namespace std;

// Include TinyEXR
#define TINYEXR_IMPLEMENTATION
#include "tinyexr/tinyexr.h"

/* Exception type when loading EXRImages.
 */
struct ExceptionEXR : public std::exception {
	ExceptionEXR(const std::string& filename, const std::string& error) {
		msg = std::string("Error with file \'") + filename +
				std::string("\': ") +	error;
	}

	const char* what() const noexcept {
		return msg.c_str();
	}

	std::string msg;
};


/* Loading an EXR image using tinyexr. The image needs to be located at
 * filename `name`.
 *
 * Return the EXRImage if the image can be loaded. If the image cannot
 * be loaded, an ExceptionEXR is thrown.
 */
EXRImage LoadImage(const std::string& name) {
	const char* input = name.c_str();
	const char* err;

	EXRImage image;
	InitEXRImage(&image);

	int ret = ParseMultiChannelEXRHeaderFromFile(&image, input, &err);
	if (ret != 0) {
		std::string err_str(err);
		throw ExceptionEXR(name, err_str);
	}

	for (int i = 0; i < image.num_channels; i++) {
		image.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
	}

	ret = LoadMultiChannelEXRFromFile(&image, input, &err);
	if (ret != 0) {
		std::string err_str(err);
		throw ExceptionEXR(name, err_str);
	}

	return image;
}

int main(int argc, char** argv)
{
	std::string input_file(argv[1]) ;
	std::string ref_file(argv[2]) ;

   // Open files
   auto qryImg = LoadImage(input_file);
   auto refImg = LoadImage(ref_file);

   // Check if the images are coherent
   // Width, Height, and Number of channels must match
   if(qryImg.width        != refImg.width        ||
      qryImg.height       != refImg.height       ||
      qryImg.num_channels != refImg.num_channels) {
      throw ExceptionEXR(ref_file + std::string(" and ") + input_file,
                         std::string("Files do not match"));
   }

   for(int k=0; k<qryImg.num_channels; ++k) {
      if(strcmp(qryImg.channel_names[k], refImg.channel_names[k]) != 0) {
         throw ExceptionEXR(ref_file + std::string(" and ") + input_file,
                            std::string("Have different color channels"));
      }
   }

   // Create the metric object
   std::vector<float> pixelQry, pixelRef;
   pixelQry.reserve(qryImg.num_channels);
   pixelRef.reserve(qryImg.num_channels);

   // Loop over the pixels and perform a metric between the query
   // and the reference pixel.
   const int N = qryImg.width * qryImg.height;
   const int channel = 0;
   for(int i=0; i<N; ++i) {
      for(int k=0; k<qryImg.num_channels; ++k) {
         // Convert unsigned int pointers to float pointers before
         // accessing the values.
         float* qry = (float*)qryImg.images[k];
         float* ref = (float*)refImg.images[k];
         qry[i] += ref[i];
      }
   }

   // Save file
   const char* err = nullptr;
   int res = SaveMultiChannelEXRToFile(&qryImg, argv[3], &err);
   if(res != 0 && err != nullptr) {
      std::cout << "Error: " << err << std::endl;
   }

	return EXIT_SUCCESS;
}
