#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/Polygon.h>
#include <panda/types/Rect.h>
#include <QVector>

#include <cmath>
#include <algorithm>

namespace panda {

using types::Point;
using types::Path;
using types::Polygon;

class Polygon_Compose : public PandaObject
{
public:
	PANDA_CLASS(Polygon_Compose, PandaObject)

	Polygon_Compose(PandaDocument *doc)
		: PandaObject(doc)
		, m_contour(initData(&m_contour, "contour", "Input contour"))
		, m_innerPaths(initData(&m_innerPaths, "inner", "Input inner paths"))
		, m_output(initData(&m_output, "polygon", "Output polygon"))
	{
		addInput(m_contour);
		addInput(m_innerPaths);
		addOutput(m_output);
	}

	void update()
	{
		auto output = m_output.getAccessor();
		output->contour = m_contour.getValue();
		output->holes = m_innerPaths.getValue();

		cleanDirty();
	}

protected:
	Data< Path > m_contour;
	Data< QVector<Path> > m_innerPaths;
	Data< Polygon > m_output;
};

int Polygon_ComposeClass = RegisterObject<Polygon_Compose>("Generator/Polygon/Compose polygon").setDescription("Create a polygon from a contour and a list of inner paths");

//****************************************************************************//

class Polygon_Decompose : public PandaObject
{
public:
	PANDA_CLASS(Polygon_Decompose, PandaObject)

	Polygon_Decompose(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData(&m_input, "polygon", "Input polygon"))
		, m_contour(initData(&m_contour, "contour", "Output contour"))
		, m_innerPaths(initData(&m_innerPaths, "inner", "Output inner paths"))
	{
		addInput(m_input);
		addOutput(m_contour);
		addOutput(m_innerPaths);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		m_contour.getAccessor() = input.contour;
		m_innerPaths.getAccessor() = input.holes;

		cleanDirty();
	}

protected:
	Data< Polygon > m_input;
	Data< Path > m_contour;
	Data< QVector<Path> > m_innerPaths;
};

int Polygon_DecomposeClass = RegisterObject<Polygon_Decompose>("Generator/Polygon/Decompose polygon").setDescription("Extract the contour and the inner paths of a polygon");

//****************************************************************************//

} // namespace Panda


