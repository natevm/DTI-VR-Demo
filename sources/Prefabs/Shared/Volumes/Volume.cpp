#include "Volume.hpp"

namespace Prefabs {
	
	using namespace std;
	Volume::Volume(string filename, glm::vec3 rawDimensions, int bytesPerPixel, int samples) : Model() {
		this->filename = filename;
		this->rawDimensions = rawDimensions;
		this->bytesPerPixel = bytesPerPixel;
		this->samples = samples;
	}

	void Volume::setTransferFunction(std::shared_ptr<Components::Textures::Texture2D> transferFunction) {
		this->transferFunction = transferFunction;
	}

	bool Volume::isVolumeLoaded() {
		return volumeLoaded;
	}

	std::shared_ptr<Components::Textures::Texture2D> Volume::getHistogramTexture() {
		return histogramTexture;
	}

	void Volume::setNumSamples(int newNumSamples) {
		samples = newNumSamples;
	}

	int Volume::getNumSamples() {
		return samples;
	}

	inline unsigned short getVoxel(glm::vec3 pos, glm::vec3 size, int bytesPerPixel, const std::vector<GLubyte> &volume) {
		if (bytesPerPixel == 1)
			return volume[pos.x + pos.y * size.x + pos.z * size.x * size.y];
		else {
			unsigned short high = volume[(pos.x + pos.y * size.x + pos.z * size.x * size.y) * 2];
			unsigned short low = volume[(pos.x + pos.y * size.x + pos.z * size.x * size.y) * 2 + 1];

			return high << 8 | low;
		}
	}

	inline unsigned short getVoxel(int pos, int bytesPerPixel, const std::vector<GLubyte> &volume) {
		if (bytesPerPixel == 1)
			return volume[pos];
		else {
			unsigned short high = volume[pos * 2];
			unsigned short low = volume[pos * 2 + 1];

			return high << 8 | low;
		}
	}

	void computeGradientMagnitudeAtPoint(glm::vec3 pos, glm::vec3 size, int bytesPerPixel, const std::vector<GLubyte> &input, std::vector<GLfloat> &output) {
		if ((pos.x == 0 || pos.y == 0 || pos.z == 0) || (pos.x == size.x - 1 || pos.y == size.y - 1 || pos.z == size.z - 1)) {
			output[pos.x + pos.y * size.x + pos.z * size.x * size.y] = 0;
		}
		else {
			double dx = getVoxel(pos + glm::vec3(1, 0, 0), size, bytesPerPixel, input) - getVoxel(pos - glm::vec3(1, 0, 0), size, bytesPerPixel, input);
			double dy = getVoxel(pos + glm::vec3(0, 1, 0), size, bytesPerPixel, input) - getVoxel(pos - glm::vec3(0, 1, 0), size, bytesPerPixel, input);
			double dz = getVoxel(pos + glm::vec3(0, 0, 1), size, bytesPerPixel, input) - getVoxel(pos - glm::vec3(0, 0, 1), size, bytesPerPixel, input);
			double udx = abs(dx); // for large negative numbers
			double udy = abs(dy);
			double udz = abs(dz);

			output[pos.x + pos.y * size.x + pos.z * size.x * size.y] = sqrt((udx*udx + udy*udy + udz*udz));
			assert(output[pos.x + pos.y * size.x + pos.z * size.x * size.y] >= 0);// = sqrt(dx*dx + dy*dy + dz*dz);
		}
	}

	void equalizeHistogram(int* pdata, int width, int height, int max_val = 255)
	{
		int total = width*height;
		int n_bins = max_val + 1;

		// Compute histogram
		vector<int> hist(n_bins, 0);
		for (int i = 0; i < total; ++i) {
			hist[pdata[i]]++;
		}

		// Build LUT from cumulative histrogram

		// Find first non-zero bin
		int i = 0;
		while (!hist[i]) ++i;

		if (hist[i] == total) {
			for (int j = 0; j < total; ++j) {
				pdata[j] = i;
			}
			return;
		}

		// Compute scale
		float scale = (n_bins - 1.f) / (total - hist[i]);

		// Initialize lut
		vector<int> lut(n_bins, 0);
		i++;

		int sum = 0;
		for (; i < hist.size(); ++i) {
			sum += hist[i];
			// the value is saturated in range [0, max_val]
			lut[i] = std::max(0, std::min(int(round(sum * scale)), max_val));
		}

		// Apply equalization
		for (int i = 0; i < total; ++i) {
			pdata[i] = lut[pdata[i]];
		}
	}

	void Volume::computeGradientMagnitudeVolume() {
		gradientMagnitudeVolume.resize(rawDimensions.x * rawDimensions.y * rawDimensions.z);
		for (int z = 0; z < rawDimensions.z; ++z) {
			for (int y = 0; y < rawDimensions.y; ++y) {
				for (int x = 0; x < rawDimensions.x; ++x) {
					computeGradientMagnitudeAtPoint(glm::vec3(x, y, z), rawDimensions, bytesPerPixel, volume, gradientMagnitudeVolume);
				}
			}
		}

		/* Normalize gradient magnitudes */
		float maxMagnitude = 0;
		for (int i = 0; i < gradientMagnitudeVolume.size(); ++i) {
			maxMagnitude = std::max(maxMagnitude, gradientMagnitudeVolume[i]);
		}

		/*for (int i = 0; i < gradientMagnitudeVolume.size(); ++i) {
		gradientMagnitudeVolume[i] = gradientMagnitudeVolume[i] / maxMagnitude;
		}*/

	}

	void Volume::computeHistogram() {
		if (bytesPerPixel == 1)
			computeHistogram8();
		else
			computeHistogram16();
	}

	void Volume::computeHistogram8() {
		/* ONLY WORKS FOR 1 BYTE ATM...*/
		int width = 256;
		vector<int> histogram(width);

		int height = 0;
		for (int i = 0; i < volume.size(); ++i) {
			int address = volume.at(i);
			histogram.at(address) += 1;
			height = std::max(histogram.at(volume.at(i)), height);
		}

		vector<GLubyte> image(256 * width);
		for (int y = 0; y < 256; ++y) {
			for (int x = 0; x < width; ++x) {
				float normalized = histogram.at(volume.at(x)) / (float)height; // potential bug here
				image.at(x + (255 - y) * width) = ((255 * normalized) < y) ? 255 : 0;
			}
		}
		histogramTexture = make_shared<Components::Textures::Texture2D>(width, 256, image, true);
	}

	void Volume::computeHistogram16() {
		/* ONLY WORKS FOR 2 BYTE ATM...*/
		int width = 65536;
		vector<int> histogram(256); // 16 bit lut is too big. use 8 bits for now.

		int height = 0;
		for (int i = 0; i < volume.size(); i += 2) {
			int address = volume.at(i);
			address |= volume.at(i + 1) << 8;

			float normalized = address / 65536.0;
			histogram.at(normalized * 255) += 1;
			height = std::max(histogram.at(normalized * 255), height);
		}

		vector<GLubyte> image(256 * 256);
		for (int y = 0; y < 256; ++y) {
			for (int x = 0; x < 256; ++x) {
				float normalized = histogram.at(x) / (float)height; // potential bug here
				image.at(x + (255 - y) * 256) = ((255 * normalized) < y) ? 255 : 0;
			}
		}
		histogramTexture = make_shared<Components::Textures::Texture2D>(256, 256, image, true);
	}

	void Volume::compute2DHistogram() {
		int totalVoxels = rawDimensions.x * rawDimensions.y * rawDimensions.z;

		/* Determine max gradient magnitude & data value */
		float maxDataValue = 0;
		float maxMagnitude = 0;
		for (int i = 0; i < gradientMagnitudeVolume.size(); ++i) {
			maxMagnitude = std::max(maxMagnitude, gradientMagnitudeVolume[i]);
			maxDataValue = std::max(maxDataValue, (float)getVoxel(i, bytesPerPixel, volume));
		}

		int textureWidth = 256;
		int textureHeight = 256;
		vector<int> TwoDimHistogram(textureWidth * textureHeight);

		/* Add data value and gradient magnitude to cooresponding bins */
		int maxBinHeight = 0;
		for (int i = 0; i < totalVoxels; ++i) {
			unsigned int xaddress = getVoxel(i, bytesPerPixel, volume);
			unsigned int yaddress = gradientMagnitudeVolume.at(i);

			xaddress = (xaddress / maxDataValue) * (textureWidth - 1);
			yaddress = (yaddress / maxMagnitude) * (textureHeight - 1);

			TwoDimHistogram.at(xaddress + (yaddress * textureWidth)) += 1;
			maxBinHeight = std::max(TwoDimHistogram.at(xaddress + (yaddress * textureWidth)), maxBinHeight);
		}

		//maxBinHeight = TwoDimHistogram.size();
		//for (int i = 0; i < TwoDimHistogram.size(); ++i) {
		//	TwoDimHistogram.at(i) = i;
		//}

		/* Equilize that */
		equalizeHistogram(TwoDimHistogram.data(), textureWidth, textureHeight, maxBinHeight);


		/* Now use that as a lookup table */
		vector<GLubyte> image(textureHeight * textureWidth);
		for (int y = 0; y < textureHeight; ++y) {
			for (int x = 0; x < textureWidth; ++x) {
				float normalized = TwoDimHistogram.at(x + (textureWidth * y)) / (float)maxBinHeight;
				image.at(x + (textureWidth * ((textureHeight-1) - y))) = normalized * 255;
			}
		}

		histogramTexture = make_shared<Components::Textures::Texture2D>(textureWidth, textureHeight, image, true);
		System::TextureList["histogramTexture"] = histogramTexture;

		transferFunction = make_shared<Components::Textures::Texture2D>(pow(2, 12), pow(2, 12));
		System::TextureList["transferFunction"] = transferFunction;

		minTransferFunctionCoord = glm::vec2(0, 0);
		int maxValue = (bytesPerPixel == 1) ? 255 : 65535;
		maxTransferFunctionCoord = glm::vec2(1, maxMagnitude);
	}

	void Volume::updateImage(string filename, glm::vec3 rawDimensions, int bytesPerVoxel) {
		/* Loads and uploads a 3D texture to the GPU */
		const int sizeInBytes = rawDimensions.x * rawDimensions.y * rawDimensions.z * bytesPerVoxel;
		FILE *pFile = fopen(std::string(ResourcePath + std::string("/") + filename).c_str(), "rb");
		if (NULL == pFile) {
			/* Try alternative path*/
			pFile = fopen(filename.c_str(), "rb");
			if (pFile == NULL) {
				cout << "ERROR LOADING VOLUME " << filename << endl;
				return;
			}
		}

		volume.resize(rawDimensions.x * rawDimensions.y * rawDimensions.z * bytesPerVoxel);
		fread(volume.data(), sizeof(GLubyte), sizeInBytes, pFile);
		fclose(pFile);
		
		if (bytesPerVoxel == 1) {
			computeGradientMagnitudeVolume();

			compute2DHistogram();

			/* Upload gradient magnitude*/
			glGenTextures(1, &gradientTextureID);
			print_gl_error();

			glBindTexture(GL_TEXTURE_3D, gradientTextureID);
			print_gl_error();

			glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, rawDimensions.x, rawDimensions.y, rawDimensions.z, 0, GL_RED,
				GL_FLOAT, gradientMagnitudeVolume.data());
		}

		//glGenerateMipmap(GL_TEXTURE_3D);

		print_gl_error();

		/* Upload data values */
		glGenTextures(1, &volumeTextureID);
		print_gl_error();

		glBindTexture(GL_TEXTURE_3D, volumeTextureID);
		print_gl_error();

		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLenum format = GL_RED;
		if (bytesPerVoxel == 2)
			format = GL_R16;
		if (bytesPerVoxel == 3)
			format = GL_BGR; // TODO: add format as flag

		GLenum internalFormat = GL_R8;
		if (bytesPerVoxel == 3)
			internalFormat = GL_RGB;

		GLenum type = GL_UNSIGNED_BYTE;
		if (bytesPerVoxel == 2) type = GL_UNSIGNED_SHORT;

		glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, rawDimensions.x, rawDimensions.y, rawDimensions.z, 0, format, type, volume.data());

		//glGenerateMipmap(GL_TEXTURE_3D);

		print_gl_error();
		volumeLoaded = true;
	}
}