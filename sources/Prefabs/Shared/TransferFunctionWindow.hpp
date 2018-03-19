#pragma once
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/intersect.hpp>

#include <iostream>
#include "cldk.hpp"
#include "gldk.hpp"

#include "Options/Options.h"

#include "System/System.hpp"

#include "Entities/Cameras/OrbitCamera.hpp"
#include "Entities/Model/Model.hpp"

#include "Components/Materials/Material.hpp"
#include "Components/Meshes/Mesh.hpp"

#include "TransferFunctionWidget.hpp"

namespace Prefabs {
	class TransferFunctionWindow : public Entity {
	public:
		/*TODO: Add list of widgets */
		std::shared_ptr<Model> exampleWidget;
		std::vector<std::shared_ptr<Prefabs::TransferFunctionWidget>> widgets;
		TransferFunctionWindow() {
			exampleWidget = std::make_shared<Model>();
			exampleWidget->addMaterial(System::MaterialList["BlueSurface"]);
			//cube->addMaterial(System::MaterialList["RedWire"]);
			exampleWidget->setMesh(System::MeshList["Sphere"]);
			exampleWidget->transform.SetPosition(-.5, 0.0, 0);
			exampleWidget->transform.SetScale(.1, .1, .1);
			addObject("1", exampleWidget);
		}

		void update() {
			//exampleWidget->transform.AddRotation(glm::quat(glm::angleAxis(.01f, glm::vec3(1.0, 1.0, 1.0))));
		}

		/* TODO: Add raycast handler */
		void raycast(glm::vec4 point, glm::vec4 direction) {
			exampleWidget->transform.SetPosition(glm::vec3(point.x, point.y, 0.0));
		}

		/* TODO: Add ability to add/remove a widget */

		/* TODO: Add ability to add/remove a point from a widget */

		/* TODO: Add ability to change color of selected widget's selected point's color */
	};
}