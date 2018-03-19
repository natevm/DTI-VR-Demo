#include "RaycastVolume.hpp"

#include <algorithm>
#include "glm/gtx/string_cast.hpp"

#include "Components/Materials/Surface/UnlitSurface.hpp"
#include "Components/Materials/Wireframe/Wireframe.hpp"
#include "Components/Materials/Volume/BasicRaycastVolume.hpp"

namespace Prefabs {

	int RaycastVolume::count = 0;
	
	RaycastVolume::RaycastVolume(
		string filename,
		int3 rawDimensions,
		int bytesPerPixel,
		shared_ptr<Entity> perceivingEntity,
		int samples) : Volume(filename, rawDimensions, bytesPerPixel, samples)
	{
		using namespace Components::Materials;
		this->perceivingEntity = perceivingEntity;
		this->focalLength = focalLength;
		this->windowWidth = windowWidth;
		count++;
		
		//updateVBO();
		updateImage(filename, rawDimensions, bytesPerPixel);

		if (volumeLoaded)
			compute2DHistogram();

		/* Add wireframe material */
		//std::shared_ptr<WireframeMaterial> redWire = dynamic_pointer_cast<WireframeMaterial>(Scene::MaterialList["RedWire"]);
		//redWire->setAttributeData(pointsVBO, 12 * 3);
		//this->materials.push_back(redWire);

		/* Add basic volume material */
		//std::shared_ptr<BasicRaycastVolumeMaterial> volumeMaterial = dynamic_pointer_cast<BasicRaycastVolumeMaterial>(Scene::MaterialList["VolumeMaterial"]);
		//volumeMaterial->setAttributeData(pointsVBO, 12 * 3);
		//volumeMaterial->setUniformData(samples, false, volumeTextureID);
		//this->materials.push_back(volumeMaterial);
	}

	//void RaycastVolume::updateVBO() {
	//	using namespace glm;
	//	using namespace std;

	//	/* Create cube to render volume in */

	//	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	//	const GLfloat cubePoints[] = {
	//		-1.0f,-1.0f,-1.0f,  -1.0f,-1.0f, 1.0f,  -1.0f, 1.0f, 1.0f, //t1
	//		1.0f, 1.0f,-1.0f,1.0f,  -1.0f,-1.0f,-1.0f,1.0f,  -1.0f, 1.0f,-1.0f,1.0f, //t2
	//		1.0f,-1.0f, 1.0f,1.0f,  -1.0f,-1.0f,-1.0f,1.0f,  1.0f,-1.0f,-1.0f,1.0f, //t3
	//		1.0f, 1.0f,-1.0f,1.0f,  1.0f,-1.0f,-1.0f,1.0f,  -1.0f,-1.0f,-1.0f,1.0f, //t4
	//		-1.0f,-1.0f,-1.0f,1.0f,  -1.0f, 1.0f, 1.0f,1.0f,  -1.0f, 1.0f,-1.0f,1.0f, //t5
	//		1.0f,-1.0f, 1.0f,1.0f,  -1.0f,-1.0f, 1.0f,1.0f,  -1.0f,-1.0f,-1.0f,1.0f, //t6
	//		-1.0f, 1.0f, 1.0f,1.0f,  -1.0f,-1.0f, 1.0f,1.0f,  1.0f,-1.0f, 1.0f,1.0f, //t7
	//		1.0f, 1.0f, 1.0f,1.0f,  1.0f,-1.0f,-1.0f,1.0f,  1.0f, 1.0f,-1.0f,1.0f, //t8
	//		1.0f,-1.0f,-1.0f,1.0f,  1.0f, 1.0f, 1.0f,1.0f,  1.0f,-1.0f, 1.0f,1.0f, //t9
	//		1.0f, 1.0f, 1.0f,1.0f,  1.0f, 1.0f,-1.0f,1.0f,  -1.0f, 1.0f,-1.0f,1.0f, //t10
	//		1.0f, 1.0f, 1.0f,1.0f,  -1.0f, 1.0f,-1.0f,1.0f,  -1.0f, 1.0f, 1.0f,1.0f, //t11
	//		1.0f, 1.0f, 1.0f,1.0f,  -1.0f, 1.0f, 1.0f,1.0f,  1.0f,-1.0f, 1.0f,1.0f //t12
	//	};
	//	pointsVBOSize = sizeof(cubePoints);

	//	// One color for each vertex. They were generated randomly.
	//	static const GLfloat cubeColors[] = {
	//		1.083f,  0.771f,  0.014f,  1.0f,  0.609f,  0.115f,  0.436f,  1.0f,  0.327f,  0.483f,  0.844f,  1.0f,
	//		0.822f,  1.069f,  0.201f,  1.0f,  0.435f,  0.602f,  0.223f,  1.0f,  0.310f,  0.747f,  0.185f,  1.0f,
	//		1.097f,  0.770f,  0.761f,  1.0f,  1.059f,  0.436f,  0.730f,  1.0f,  0.359f,  1.083f,  0.152f,  1.0f,
	//		0.483f,  1.096f,  0.789f,  1.0f,  1.059f,  0.861f,  0.639f,  1.0f,  0.195f,  1.048f,  0.859f,  1.0f,
	//		0.014f,  0.184f,  1.076f,  1.0f,  0.771f,  0.328f,  0.970f,  1.0f,  0.406f,  0.615f,  0.116f,  1.0f,
	//		0.676f,  0.977f,  0.133f,  1.0f,  0.971f,  1.072f,  0.833f,  1.0f,  0.140f,  0.616f,  0.489f,  1.0f,
	//		0.997f,  1.013f,  0.064f,  1.0f,  0.945f,  0.719f,  1.092f,  1.0f,  1.043f,  0.021f,  0.978f,  1.0f,
	//		0.279f,  0.317f,  1.005f,  1.0f,  0.167f,  0.620f,  0.077f,  1.0f,  0.347f,  0.857f,  0.137f,  1.0f,
	//		0.055f,  0.953f,  0.042f,  1.0f,  0.714f,  1.005f,  0.345f,  1.0f,  0.783f,  0.290f,  0.734f,  1.0f,
	//		0.722f,  0.645f,  0.174f,  1.0f,  0.302f,  0.455f,  0.848f,  1.0f,  0.225f,  1.087f,  0.040f,  1.0f,
	//		1.017f,  0.713f,  0.338f,  1.0f,  0.053f,  0.959f,  0.120f,  1.0f,  0.393f,  0.621f,  0.362f,  1.0f,
	//		0.673f,  0.211f,  0.457f,  1.0f,  0.820f,  0.883f,  0.371f,  1.0f,  0.982f,  0.099f,  0.879f,  1.0f
	//	};
	//	colorsVBOSize = sizeof(cubeColors);

	//	/* Upload the points */
	//	glGenBuffers(1, &pointsVBO);
	//	glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
	//	glBufferData(GL_ARRAY_BUFFER, pointsVBOSize, cubePoints, GL_STATIC_DRAW);

	//	/* Upload the colors */
	//	glGenBuffers(1, &colorsVBO);
	//	glBindBuffer(GL_ARRAY_BUFFER, colorsVBO);
	//	glBufferData(GL_ARRAY_BUFFER, colorsVBOSize, cubeColors, GL_STATIC_DRAW);
	//}

	//void RaycastVolume::updateVAO() {

	//	glGenVertexArrays(1, &linesVAO);
	//	glBindVertexArray(linesVAO);

	//	glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
	//	glEnableVertexAttribArray(Shaders::lineProgram->position_id);
	//	glVertexAttribPointer(Shaders::lineProgram->position_id, 4,
	//		GL_FLOAT, GL_FALSE, sizeof(float4), 0);

	//	glBindBuffer(GL_ARRAY_BUFFER, colorsVBO);
	//	glEnableVertexAttribArray(Shaders::lineProgram->color_id);
	//	glVertexAttribPointer(Shaders::lineProgram->color_id, 4,
	//		GL_FLOAT, GL_FALSE, sizeof(float4), 0);

	//	glGenVertexArrays(1, &VAO);
	//	glBindVertexArray(VAO);

	//	glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
	//	glEnableVertexAttribArray(Shaders::raycastVolProgram->position_id);
	//	glVertexAttribPointer(Shaders::raycastVolProgram->position_id, 4,
	//		GL_FLOAT, GL_FALSE, sizeof(float4), 0);
	//	print_gl_error();

	//	/*glBindBuffer(GL_ARRAY_BUFFER, colorsVBO);
	//	print_gl_error();
	//	glEnableVertexAttribArray(Shaders::raycastVolProgram->color_id);
	//	print_gl_error();
	//	glVertexAttribPointer(Shaders::raycastVolProgram->color_id, 4,
	//		GL_FLOAT, GL_FALSE, sizeof(float4), 0);*/
	//	print_gl_error();


	//}

	

	//void RaycastVolume::render(mat4 model, mat4 view, mat4 projection) {
	//	if (!volumeLoaded) return;
	//	if (hide == true) return;
	//	
	//	mat4 modelView = view * model * transform.LocalToParentMatrix();
	//	vec4 worldSpaceCameraPosition = getCameraFromView(view);
	//	mat4 inverseView = glm::inverse(view);
	//	worldSpaceCameraPosition = inverseView[3];
	//	glm::mat4 inverseModelMatrix = glm::inverse(model * transform.LocalToParentMatrix());
	//	glm::vec4 modelSpaceCameraPosition = inverseModelMatrix * worldSpaceCameraPosition;

	//	/* 1. Use program */
	//	Shaders::raycastVolProgram->use();

	//	/* 2. Use VAO */
	//	glBindVertexArray(VAO);

	//	/* 3. Update uniforms */
	//	glUniformMatrix4fv(Shaders::raycastVolProgram->model_view_id, 1, 0, &(modelView[0].x));
	//	glUniformMatrix4fv(Shaders::raycastVolProgram->projection_id, 1, 0, &(projection[0].x));
	//	glUniform3fv(Shaders::raycastVolProgram->ray_origin_id, 1, &modelSpaceCameraPosition.x);
	//	glUniform1i(Shaders::raycastVolProgram->samples_id, samples);
	//	glUniform1i(Shaders::raycastVolProgram->perturbation_id, perturbation);
	//	glUniform2fv(Shaders::raycastVolProgram->texmaxcoord_id, 1, &maxTransferFunctionCoord.x);
	//	glUniform2fv(Shaders::raycastVolProgram->texmincoord_id, 1, &minTransferFunctionCoord.x);
	//	print_gl_error();

	//	/* Volume data */
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_3D, volumeTextureID);
	//	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, (interpolate) ? GL_LINEAR : GL_NEAREST);
	//	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, (interpolate) ? GL_LINEAR : GL_NEAREST);
	//	glUniform1i(Shaders::raycastVolProgram->texture0_id, 0);
	//	print_gl_error();
	//	
	//	/* Transfer function texture */
	//	glActiveTexture(GL_TEXTURE1);
	//	glBindTexture(GL_TEXTURE_2D, transferFunction->textureID);
	//	glUniform1i(Shaders::raycastVolProgram->texture1_id, 1);
	//	print_gl_error();

	//	/* Gradient magnitude data */
	//	glActiveTexture(GL_TEXTURE2);
	//	glBindTexture(GL_TEXTURE_3D, gradientTextureID);
	//	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, (interpolate) ? GL_LINEAR : GL_NEAREST);
	//	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, (interpolate) ? GL_LINEAR : GL_NEAREST);
	//	glUniform1i(Shaders::raycastVolProgram->texture2_id, 2);
	//	print_gl_error();
	//	
	//	/* Current depth buffer */
	////	glActiveTexture(GL_TEXTURE12);
	//	//glBindTexture(GL_TEXTURE_2D, display->fb_descs[i].depth_texture);

	//	/* 4. Draw */
	//	glDrawArrays(GL_TRIANGLES, 0, pointsVBOSize / sizeof(float4));
	//	print_gl_error();

	//	//Entity::render(parent_matrix, projection);
	//}

	void RaycastVolume::handleKeys()
	{
		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_N)) {
			interpolate = false;
		}
		else if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_L)) {
			interpolate = true;
		}
		
		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_Y)) {
			samples = 32;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_U)) {
			samples = 128;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_I)) {
			samples = 512;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_O)) {
			samples = 1024;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_P)) {
			samples = 2048;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_COMMA)) {
			perturbation = true;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_PERIOD)) {
			perturbation = false;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_W)) {
			hide = true;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_S)) {
			hide = false;
		}
	}

	void RaycastVolume::update() {
		if (!volumeLoaded) return;
	
		//this->transform.AddRotation(glm::angleAxis(0.001f, glm::vec3(1.0, 1.0, 1.0)));

		handleKeys();
	}
}