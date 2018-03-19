#include "Material.hpp"

/* Create hash function for pipeline key */
namespace Components::Materials {
	std::unordered_map<PipelineKey, PipelineParameters> Components::Materials::PipelineSettings;
}