#pragma once

#include <panda/types/Gradient.h>

class asIScriptEngine;

namespace panda
{

	void registerTypes(asIScriptEngine* engine);

	// Gradient wrapper class
	class GradientWrapper
	{
	public:
		static GradientWrapper* factory()
		{ return new GradientWrapper(); }

		static GradientWrapper* create(const panda::types::Gradient& gradient)
		{ return new GradientWrapper(gradient); }

		GradientWrapper() {}
		GradientWrapper(const panda::types::Gradient& gradient)
			: m_gradient(gradient) {}

		const panda::types::Gradient& gradient() const
		{ return m_gradient; }

		void addRef() { m_refCount++; }
		void release() { if (--m_refCount == 0) delete this; }

		void clear() { m_gradient.clear(); }
		void add(float pos, panda::types::Color col) { m_gradient.add(pos, col); }
		panda::types::Color get(float pos) { return m_gradient.get(pos); }

	private:
		panda::types::Gradient m_gradient;
		int m_refCount = 1;
	};

} // namespace panda
