#include <panda/PandaDocument.h>
#include <panda/helper/algorithm.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Path.h>

#include <cmath>

namespace panda {

using types::Point;
using types::Path;

class PathMath_Translation : public PandaObject
{
public:
	PANDA_CLASS(PathMath_Translation, PandaObject)

	PathMath_Translation(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Path to translate"))
		, m_delta(initData("translation", "Value of the translation"))
		, m_output(initData("output", "Translated path"))
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
		output.clear();

		int nbA = input.size(), nbB = delta.size();
		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = std::max(nbA, nbB);
			output.resize(nb);

			for(int i=0; i<nb; ++i)
				output[i] = input[i%nbA] + delta[i%nbB];
		}
	}

protected:
	Data< std::vector<Path> > m_input, m_output;
	Data< std::vector<Point> > m_delta;
};

int PathMath_TranslationClass = RegisterObject<PathMath_Translation>("Math/Path/Translation").setName("Translate path").setDescription("Translate a path");

//****************************************************************************//

class PathMath_Scale : public PandaObject
{
public:
	PANDA_CLASS(PathMath_Scale, PandaObject)

	PathMath_Scale(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Path to scale"))
		, m_scale(initData("scale", "Value of the scale"))
		, m_output(initData("output", "Scaled path"))
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
		output.clear();

		int nbA = input.size(), nbB = scale.size();
		if(nbA && nbB)
		{
			if(nbA < nbB && nbA > 1)		nbB = nbA;	// Either equal nb of A & B, or one of them is 1
			else if(nbB < nbA && nbB > 1)	nbA = nbB;
			int nb = std::max(nbA, nbB);
			output.resize(nb);

			for(int i=0; i<nb; ++i)
				output[i] = input[i%nbA] * scale[i%nbB];
		}
	}

protected:
	Data< std::vector<Path> > m_input, m_output;
	Data< std::vector<float> > m_scale;
};

int PathMath_ScaleClass = RegisterObject<PathMath_Scale>("Math/Path/Scale").setName("Scale path").setDescription("Scale a path");

//****************************************************************************//

class PathMath_Rotate : public PandaObject
{
public:
	PANDA_CLASS(PathMath_Rotate, PandaObject)

	PathMath_Rotate(PandaDocument *doc)
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
		output.clear();

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
				nb = std::max(nbA, nbC);
			}

			output.resize(nb);

			float PI180 = static_cast<float>(M_PI) / 180;
			for(int i=0; i<nb; ++i)
				output[i] = types::rotated(input[i%nbP], center[i%nbC], angle[i%nbA] * PI180);
		}
	}

protected:
	Data< std::vector<Path> > m_input, m_output;
	Data< std::vector<Point> > m_center;
	Data< std::vector<float> > m_angle;
};

int PathMath_RotateClass = RegisterObject<PathMath_Rotate>("Math/Path/Rotate").setName("Rotate path").setDescription("Rotate a path");

//****************************************************************************//

class PathMath_Length : public PandaObject
{
public:
	PANDA_CLASS(PathMath_Length, PandaObject)

	PathMath_Length(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("path", "Path to analyse"))
		, nbPoints(initData("# of points", "Number of points in the path"))
		, length(initData("length", "Length of the path"))
	{
		addInput(input);

		addOutput(nbPoints);
		addOutput(length);
	}

	void update()
	{
		const std::vector<Path>& paths = input.getValue();
		int nb = paths.size();

		auto nbPtsList = nbPoints.getAccessor();
		auto lengthList = length.getAccessor();
		nbPtsList.resize(nb);
		lengthList.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			const auto& points = paths[i].points;
			int nbPts = points.size();
			nbPtsList[i] = nbPts;
			if(nbPts > 1)
			{
				Point pt1 = points[0];
				float l = 0.0;
				for(int j=1; j<nbPts; ++j)
				{
					const Point& pt2 = points[j];
					l += (pt2-pt1).norm();
					pt1 = pt2;
				}
				lengthList[i] = l;
			}
			else
				lengthList[i] = 0;
		}
	}

protected:
	Data< std::vector<Path> > input;
	Data< std::vector<int> > nbPoints;
	Data< std::vector<float> > length;
};

int PathMath_LengthClass = RegisterObject<PathMath_Length>("Math/Path/Curve length").setDescription("Compute the length of a series of segments");

//****************************************************************************//

class PathMath_GetPoint : public PandaObject
{
public:
	PANDA_CLASS(PathMath_GetPoint, PandaObject)

	PathMath_GetPoint(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("path", "Input Path"))
		, position(initData("position", "Position of the point on the curve"))
		, abscissa(initData("abscissa", "Abscissa of the point to find"))
		, rotation(initData("rotation", "Rotation of the point on the curve"))
	{
		addInput(input);
		addInput(abscissa);

		addOutput(position);
		addOutput(rotation);
	}

	void update()
	{
		const auto& curve = input.getValue().points;
		const std::vector<float>& listAbscissa = abscissa.getValue();
		unsigned int nbPts = curve.size();
		unsigned int nbAbscissa = listAbscissa.size();

		auto listPos = position.getAccessor();
		auto listRot = rotation.getAccessor();

		if(nbPts > 1 && nbAbscissa)
		{
			listPos.resize(nbAbscissa);
			listRot.resize(nbAbscissa);

			// Some precomputation
			float totalLength = 0.0;
			std::vector<float> lengths, starts, ends;
			lengths.resize(nbPts - 1);
			starts.resize(nbPts - 1);
			ends.resize(nbPts - 1);
			Point pt1 = curve[0];
			for(unsigned int i=0; i<nbPts-1; ++i)
			{
				starts[i] = totalLength;
				const Point& pt2 = curve[i+1];
				float l = (pt2-pt1).norm();
				lengths[i] = l;
				pt1 = pt2;
				totalLength += l;
				ends[i] = totalLength;
			}

			const float pi = static_cast<float>(M_PI);
			for(unsigned int i=0; i<nbAbscissa; ++i)
			{
				float a = helper::bound<float>(0.0, listAbscissa[i], totalLength - 1e-3f);
				std::vector<float>::iterator iter = std::upper_bound(ends.begin(), ends.end(), a);

				unsigned int index = iter - ends.begin();
				float p = 0.0;
				if(lengths[index] > 0.1)
					p = (a - starts[index]) / lengths[index];
				const Point& pt1 = curve[index];
				const Point& pt2 = curve[index+1];
				listPos.wref()[i] = pt1 * (1.f - p) + pt2 * p;
				listRot[i] = atan2(pt2.y-pt1.y, pt2.x-pt1.x) * 180.f / pi;
			}
		}
		else
		{
			listPos.clear();
			listRot.clear();
		}
	}

protected:
	Data< Path > input;
	Data< std::vector<Point> > position;
	Data< std::vector<float> > abscissa, rotation;
};

int PathMath_GetPointClass = RegisterObject<PathMath_GetPoint>("Math/Path/Point on curve").setDescription("Get the position and the rotation of a point on a curve based on his abscissa");

//****************************************************************************//

class PathMath_Centroid : public PandaObject
{
public:
	PANDA_CLASS(PathMath_Centroid, PandaObject)

	PathMath_Centroid(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("path", "Path to analyse"))
		, m_output(initData("centroid", "Centroid of the path"))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();

		int nb = input.size();
		output.resize(nb);
		for(int i=0; i<nb; ++i)
			output[i] = types::centroidOfPolygon(input[i]);
	}

protected:
	Data< std::vector<Path> > m_input;
	Data< std::vector<Point> > m_output;
};

int PathMath_CentroidClass = RegisterObject<PathMath_Centroid>("Math/Path/Centroid")
							 .setName("Centroid of path")
							 .setDescription("Compute the centroid of a closed path");

//****************************************************************************//

class PathMath_Area : public PandaObject
{
public:
	PANDA_CLASS(PathMath_Area, PandaObject)

	PathMath_Area(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("path", "Path to analyse"))
		, m_output(initData("area", "Area of the path"))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();

		int nb = input.size();
		output.resize(nb);
		for(int i=0; i<nb; ++i)
			output[i] = fabs(types::areaOfPolygon(input[i]));
	}

protected:
	Data< std::vector<Path> > m_input;
	Data< std::vector<float> > m_output;
};

int PathMath_AreaClass = RegisterObject<PathMath_Area>("Math/Path/Area")
						 .setName("Area of path")
						 .setDescription("Compute the area of a closed path");

//****************************************************************************//

class PathMath_RemoveSmallSegments : public PandaObject
{
public:
	PANDA_CLASS(PathMath_RemoveSmallSegments, PandaObject)

	PathMath_RemoveSmallSegments(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "Input path"))
		, m_output(initData("output", "Output path"))
		, m_threshold(initData(1, "threshold", "Segments with a length inferior to this are removed"))
		, m_method(initData(0, "method", "How to replace small segments"))
	{
		addInput(m_input);
		addInput(m_threshold);
		addInput(m_method);

		m_method.setWidget("enum");
		m_method.setWidgetData("First;Last;Mean");

		addOutput(m_output);
	}

	void update()
	{
		const std::vector<Path>& listInput = m_input.getValue();
		const float threshold = m_threshold.getValue();
		const int method = m_method.getValue();
		int nbInputs = listInput.size();

		auto outputList = m_output.getAccessor();
		outputList.clear();
		outputList.resize(nbInputs);
		for (int i = 0; i < nbInputs; ++i)
		{
			const auto& points = listInput[i].points;

			if (points.empty())
				continue;

			auto& outputPts = outputList[i].points;
			int nbPts = points.size();

			outputPts.push_back(points[0]);
			if (nbPts == 1)
				continue;

			Point pt1 = points[0];
			for (int j = 1; j < nbPts;)
			{
				Point pt2 = points[j];
				float l = (pt2 - pt1).norm();
				if (l < threshold)
				{
					std::vector<Point> tmp;
					tmp.push_back(pt1);
					tmp.push_back(pt2);

					// Continue searching for the first point outside of the threshold
					for (++j; j < nbPts; ++j)
					{
						Point pt3 = points[j];
						l = (pt3 - pt1).norm();
						if (l > threshold)
							break;

						tmp.push_back(pt3);
					}

					switch (method)
					{
					case 0: // First
						pt2 = tmp.front();
						break;
					case 1: // Last
						pt2 = tmp.back();
						break;
					case 2: // Mean
					{
						pt2 = Point();
						for (const auto& p : tmp)
							pt2 += p;
						pt2 /= static_cast<float>(tmp.size());
						break;
					}
					}

					outputPts.back() = pt2;
				}
				else
				{
					outputPts.push_back(pt2);
					++j;
				}

				pt1 = pt2;
			}
		}
	}

protected:
	Data< std::vector<Path> > m_input, m_output;
	Data< float > m_threshold;
	Data< int > m_method;
};

int PathMath_RemoveSmallSegmentsClass = RegisterObject<PathMath_RemoveSmallSegments>("Math/Path/Remove small segments").setDescription("Remove the small segments from a path");

//****************************************************************************//

class PathMath_IsClosed : public PandaObject
{
public:
	PANDA_CLASS(PathMath_IsClosed, PandaObject)

	PathMath_IsClosed(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("path", "Path to analyse"))
		, m_output(initData("is closed", "1 if the path is closed, 0 if it is open"))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();

		int nb = input.size();
		output.resize(nb);
		for (int i = 0; i < nb; ++i)
		{
			const auto& pts = input[i].points;
			output[i] = (pts.size() > 2 && pts.front() == pts.back());
		}
	}

protected:
	Data< std::vector<Path> > m_input;
	Data< std::vector<int> > m_output;
};

int PathMath_IsClosedClass = RegisterObject<PathMath_IsClosed>("Math/Path/Is closed")
						 .setName("Is path closed")
						 .setDescription("Detect if the path is closed or open");

//****************************************************************************//

class PathMath_IsConvex : public PandaObject
{
public:
	PANDA_CLASS(PathMath_IsConvex, PandaObject)

	PathMath_IsConvex(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("path", "Path to analyse"))
		, m_output(initData("is convex", "1 if the path is convex, 0 otherwise"))
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void update()
	{
		const auto& input = m_input.getValue();
		auto output = m_output.getAccessor();

		int nb = input.size();
		output.resize(nb);
		for (int i = 0; i < nb; ++i)
		{
			const auto& pts = input[i].points;
			const int nb = pts.size();
			if (nb < 3)
				output[i] = false;

			output[i] = true;
			float prevCross = 0;
			for (int j = 0; j < nb; ++j)
			{
				auto s1 = pts[(j + 2) % nb] - pts[(j + 1) % nb];
				auto s2 = pts[j % nb] - pts[(j + 1) % nb];
				auto c = s1.cross(s2);

				if (prevCross == 0)
					prevCross = c;
				else if(prevCross * c < 0)
				{
					output[i] = false;
					break;
				}
			}
		}
	}

protected:
	Data< std::vector<Path> > m_input;
	Data< std::vector<int> > m_output;
};

int PathMath_IsConvexClass = RegisterObject<PathMath_IsConvex>("Math/Path/Is convex")
						 .setName("Is path convex")
						 .setDescription("Detect if the path is convex");

} // namespace Panda
