#include <panda/object/ObjectFactory.h>
#include <panda/types/Polygon.h>
#include <panda/types/Rect.h>

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
		, m_contour(initData("contour", "Input contour"))
		, m_innerPaths(initData("inner", "Input inner paths"))
		, m_output(initData("polygon", "Output polygon"))
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
	}

protected:
	Data< Path > m_contour;
	Data< std::vector<Path> > m_innerPaths;
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
		, m_input(initData("polygon", "Input polygon"))
		, m_contour(initData("contour", "Output contour"))
		, m_innerPaths(initData("inner", "Output inner paths"))
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
	}

protected:
	Data< Polygon > m_input;
	Data< Path > m_contour;
	Data< std::vector<Path> > m_innerPaths;
};

int Polygon_DecomposeClass = RegisterObject<Polygon_Decompose>("Generator/Polygon/Decompose polygon").setDescription("Extract the contour and the inner paths of a polygon");

//****************************************************************************//

class Polygon_DecomposeToPaths : public PandaObject
{
public:
	PANDA_CLASS(Polygon_DecomposeToPaths, PandaObject)

	Polygon_DecomposeToPaths(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("polygon", "Input polygon"))
		, m_output(initData("paths", "Output paths"))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();
		output.clear();
		for(const auto& poly : input)
		{
			output.push_back(poly.contour);
			for(const auto& hole : poly.holes)
				output.push_back(hole);
		}
	}

protected:
	Data< std::vector<Polygon> > m_input;
	Data< std::vector<Path> > m_output;
};

int Polygon_DecomposeToPathsClass = RegisterObject<Polygon_DecomposeToPaths>("Generator/Polygon/Polygon to paths").setDescription("Extract all the paths in a polygon");

} // namespace Panda


