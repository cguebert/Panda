#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Polygon.h>
#include <QVector>

#include <cmath>
#include <algorithm>

namespace panda {

using types::Point;
using types::Polygon;

class PolygonMath_Translation : public PandaObject
{
public:
	PANDA_CLASS(PolygonMath_Translation, PandaObject)

	PolygonMath_Translation(PandaDocument *doc)
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
	Data< QVector<Polygon> > m_input, m_output;
	Data< QVector<Point> > m_delta;
};

int PolygonMath_TranslationClass = RegisterObject<PolygonMath_Translation>("Math/Polygon/Translation").setName("Translate polygon").setDescription("Translate a polygon");

//****************************************************************************//

class PolygonMath_Scale : public PandaObject
{
public:
	PANDA_CLASS(PolygonMath_Scale, PandaObject)

	PolygonMath_Scale(PandaDocument *doc)
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
	Data< QVector<Polygon> > m_input, m_output;
	Data< QVector<PReal> > m_scale;
};

int PolygonMath_ScaleClass = RegisterObject<PolygonMath_Scale>("Math/Polygon/Scale").setName("Scale polygon").setDescription("Scale a polygon");

//****************************************************************************//

class PolygonMath_Rotate : public PandaObject
{
public:
	PANDA_CLASS(PolygonMath_Rotate, PandaObject)

	PolygonMath_Rotate(PandaDocument *doc)
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
	Data< QVector<Polygon> > m_input, m_output;
	Data< QVector<Point> > m_center;
	Data< QVector<PReal> > m_angle;
};

int PolygonMath_RotateClass = RegisterObject<PolygonMath_Rotate>("Math/Polygon/Rotate").setName("Rotate polygon").setDescription("Rotate a polygon");

} // namespace Panda


