#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Mesh.h>

#include <cmath>
#include <algorithm>

namespace panda {

using types::Point;
using types::Mesh;

class MeshMath_Translation : public PandaObject
{
public:
	PANDA_CLASS(MeshMath_Translation, PandaObject)

	MeshMath_Translation(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Mesh to translate"))
		, m_delta(initData("translation", "Value of the translation"))
		, m_output(initData("output", "Translated mesh"))
	{
		addInput(m_input);
		addInput(m_delta);

		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		const auto& delta = m_delta.getValue();
		auto output = m_output.getAccessor();

		int nbA = input.size(), nbB = delta.size();
		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = qMax(nbA, nbB);
			output.resize(nb);

			for(int i=0; i<nb; ++i)
				output[i] = translated(input[i%nbA], delta[i%nbB]);
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Mesh> > m_input, m_output;
	Data< std::vector<Point> > m_delta;
};

int MeshMath_TranslationClass = RegisterObject<MeshMath_Translation>("Math/Mesh/Translation").setName("Translate mesh").setDescription("Translate a mesh");

//****************************************************************************//

class MeshMath_Scale : public PandaObject
{
public:
	PANDA_CLASS(MeshMath_Scale, PandaObject)

	MeshMath_Scale(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Mesh to scale"))
		, m_scale(initData("scale", "Value of the scale"))
		, m_output(initData("output", "Scaled mesh"))
	{
		addInput(m_input);
		addInput(m_scale);

		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		const auto& scale = m_scale.getValue();
		auto output = m_output.getAccessor();

		int nbA = input.size(), nbB = scale.size();
		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = qMax(nbA, nbB);
			output.resize(nb);

			for(int i=0; i<nb; ++i)
				output[i] = scaled(input[i%nbA], scale[i%nbB]);
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Mesh> > m_input, m_output;
	Data< std::vector<PReal> > m_scale;
};

int MeshMath_ScaleClass = RegisterObject<MeshMath_Scale>("Math/Mesh/Scale").setName("Scale mesh").setDescription("Scale a mesh");

//****************************************************************************//

class MeshMath_Rotate : public PandaObject
{
public:
	PANDA_CLASS(MeshMath_Rotate, PandaObject)

	MeshMath_Rotate(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Path to rotate"))
		, m_center(initData("center", "Center of the rotation"))
		, m_angle(initData("angle", "Angle of the rotation"))
		, m_output(initData("output", "Rotated path"))
	{
		addInput(m_input);
		addInput(m_center);
		addInput(m_angle);

		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		const auto& center = m_center.getValue();
		const auto& angle = m_angle.getValue();
		auto output = m_output.getAccessor();

		int nbP = input.size(), nbC = center.size(), nbA = angle.size();
		if(nbP && nbC && nbA)
		{
			int nb = nbP;
			if(nbP > 1)
			{
				if(nbC != nbP) nbC = 1;
				if(nbA != nbA) nbA = 1;
			}
			else
			{
				if(nbC > nbA && nbA > 1)		nbC = nbA;
				else if(nbA > nbC && nbC > 1)	nbA = nbC;
				nb = qMax(nbA, nbC);
			}

			output.resize(nb);

			PReal PI180 = static_cast<PReal>(M_PI) / static_cast<PReal>(180.0);
			for(int i=0; i<nb; ++i)
				output[i] = types::rotated(input[i%nbP], center[i%nbC], angle[i%nbA] * PI180);
		}

		cleanDirty();
	}

protected:
	Data< std::vector<Mesh> > m_input, m_output;
	Data< std::vector<Point> > m_center;
	Data< std::vector<PReal> > m_angle;
};

int MeshMath_RotateClass = RegisterObject<MeshMath_Rotate>("Math/Mesh/Rotate").setName("Rotate mesh").setDescription("Rotate a mesh");

} // namespace Panda


