#include <panda/types/Gradient.h>

class asIScriptEngine;
class aatc::container::templated::vector;

namespace panda 
{
	class GradientWrapper;
	GradientWrapper* createGradientWrapper(const panda::types::Gradient& gradient);
	const panda::types::Gradient& getGradient(const GradientWrapper* wrapper);

	aatc::container::templated::vector* createGradientVectorWrapper(const std::vector<panda::types::Gradient>& gradients, asIScriptEngine* engine);
	std::vector<panda::types::Gradient> getGradients(const aatc::container::templated::vector* vec);
}
