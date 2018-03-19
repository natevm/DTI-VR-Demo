#pragma once

#include "Vector/vec.h"

#include "Prefabs/Shared/Volumes/Volume.hpp"


using namespace std;

namespace Prefabs {
	class SlicedVolume : public Volume {
	public:
		void prerender(glm::mat4 model, glm::mat4 view, glm::mat4 projection);
		void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection);
		void handleKeys();
		void update();
		SlicedVolume(string filename, glm::vec3 rawDimesions, int bytesPerPixel, shared_ptr<Entity> perceivingEntity, int samples);
		

	protected:
		static int count;
		void updateVBO();

	private:
		GLuint VAO;
		cl_GLuint VBO;
		cl::BufferGL VBO_cl;
		size_t VBOSize = 0;

		/* Sample points*/
		bool renderSamplePoints = false;
		GLuint samplePointsVAO;
		cl_GLuint samplePointsVBO;
		cl::BufferGL samplePointsVBO_cl;
		size_t numSamplePoints = 0;

		/* Edge points*/
		bool renderEdgePoints = false;
		bool renderFaces = false;
		bool renderSlices = true;
		bool interpolate = true;
		GLuint edgePointsVAO;
		cl_GLuint pointsVBO;
		cl::BufferGL edgePointsVBO_cl;
		size_t numPoints = 0;

		shared_ptr<Entity> perceivingEntity;
		
		float4 offset;
	};
}