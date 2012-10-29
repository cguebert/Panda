#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

template <class T>
class ListBuffer : public PandaObject
{
public:
	ListBuffer(PandaDocument *doc)
		: PandaObject(doc)
		, prevControl(-1.0)
		, control(initData(&control, 0.0, "control", "The buffer will be updated each time this value changes"))
		, initialValue(initData(&initialValue, "init", "Value to put in the buffer when the control is equal to zero"))
		, input(initData(&input, "input", "Value to put in the buffer at each timestep"))
		, output(initData(&output, "output", "Value stored in the buffer"))
	{
		addInput(&control);
		addInput(&initialValue);
		addInput(&input);

		addOutput(&output);
	}

	void update()
	{
		double newControl = control.getValue();
		if(prevControl != newControl)
		{
			prevControl = newControl;
			QVector<T>& outVal = *output.beginEdit();
			if(newControl)
				outVal = input.getValue();
			else
				outVal = initialValue.getValue();
			output.endEdit();
		}
		this->cleanDirty();
	}

	void setDirtyValue()
	{
		if(prevControl != control.getValue())
			PandaObject::setDirtyValue();
	}

protected:
	double prevControl;
	Data<double> control;
	Data< QVector<T> > initialValue, input, output;
};

int ListBufferIntClass = RegisterObject("List/Integer/Buffer").setClass< ListBuffer<int> >().setName("Buffer").setDescription("Memorize a integer value and update it when the control value changes");
int ListBufferRealClass = RegisterObject("List/Real/Buffer").setClass< ListBuffer<double> >().setName("Buffer").setDescription("Memorize a real value and update it when the control value changes");
int ListBufferColorClass = RegisterObject("List/Color/Buffer").setClass< ListBuffer<QColor> >().setName("Buffer").setDescription("Memorize a color value and update it when the control value changes");
int ListBufferPointClass = RegisterObject("List/Point/Buffer").setClass< ListBuffer<QPointF> >().setName("Buffer").setDescription("Memorize a point value and update it when the control value changes");
int ListBufferRectClass = RegisterObject("List/Rectangle/Buffer").setClass< ListBuffer<QRectF> >().setName("Buffer").setDescription("Memorize a rectangle value and update it when the control value changes");
int ListBufferStringClass = RegisterObject("List/Text/Buffer").setClass< ListBuffer<QString> >().setName("Buffer").setDescription("Memorize a text value and update it when the control value changes");
//int ListBufferImageClass = RegisterObject("List/Image/Buffer").setClass< ListBuffer<QImage> >().setName("Buffer").setDescription("Memorize an image value and update it when the control value changes");

} // namespace Panda

