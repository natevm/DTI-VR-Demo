#pragma once

#include "Vector/vec.h"

#include "Prefabs/Shared/Volumes/Volume.hpp"


using namespace std;

namespace Prefabs {
	class RaycastVolume : public Volume {
	public:
		RaycastVolume(string filename, int3 size, int bytesPerPixel, shared_ptr<Entity> perceivingEntity, int samples);
		void update();
		//void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection);
		void handleKeys();

	protected:
		static int count;
		//void updateVBO();
		//void updateVAO();

	private:
		//GLuint linesVAO, VAO;
		//cl_GLuint pointsVBO;
		//cl_GLuint colorsVBO;
		//size_t pointsVBOSize = 0;
		//size_t colorsVBOSize = 0;

		bool interpolate = true;
		bool hide = false;

		bool perturbation = false;
		shared_ptr<Entity> perceivingEntity;
		float focalLength;
		float windowWidth;
	};
}