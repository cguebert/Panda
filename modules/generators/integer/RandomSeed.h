#ifndef RANDOMSEED_H
#define RANDOMSEED_H

#include <panda/PandaObject.h>

#include <random>

namespace panda {

class PandaDocument;

class GeneratorInteger_RandomSeed : public PandaObject
{
	Q_OBJECT
public:
	PANDA_CLASS(GeneratorInteger_RandomSeed, PandaObject)

	GeneratorInteger_RandomSeed(PandaDocument *doc);

	virtual void reset();

private slots:
	void timeChanged();

protected:
	void newValue();

	std::mt19937 gen;
	std::uniform_int_distribution<int> dist;
	Data<int> seed, value;
};

} // namespace Panda

#endif // RANDOMSEED_H
