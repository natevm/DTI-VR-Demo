#pragma once

#include "Vector/vec.h"
#include "System/System.hpp"
#include "Entities/Model/Model.hpp"

using namespace std;

namespace Prefabs {
	class Volume : public Model {
	
	public:
		bool isVolumeLoaded();
		void setTransferFunction(std::shared_ptr<Components::Textures::Texture2D> transferFunction);
		std::shared_ptr<Components::Textures::Texture2D> getHistogramTexture();
		Volume(string filename, glm::vec3 rawDimensions, int bytesPerPixel, int samples);
		void setNumSamples(int newNumSamples);
		int getNumSamples();

	protected:
		void computeGradientMagnitudeVolume();
		void computeHistogram();
		void computeHistogram8();
		void computeHistogram16();
		void compute2DHistogram();
		void updateImage(string filename, glm::vec3 rawDimensions, int bytesPerPixel);

		int bytesPerPixel;
		int samples;
		glm::vec3 rawDimensions;
		bool volumeLoaded = false;	
		std::string filename;
		std::shared_ptr<Components::Textures::Texture2D> transferFunction;
		std::shared_ptr<Components::Textures::Texture2D> histogramTexture;
		vector<GLubyte> volume;
		vector<GLfloat> gradientMagnitudeVolume;
		glm::vec2 minTransferFunctionCoord = glm::vec2(0, 0);
		glm::vec2 maxTransferFunctionCoord = glm::vec2(0, 0);

		GLuint volumeTextureID;
		GLuint gradientTextureID;
	};
}