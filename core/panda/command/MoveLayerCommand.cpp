#include <panda/document/ObjectsList.h>
#include <panda/object/Layer.h>
#include <panda/command/MoveLayerCommand.h>

namespace panda
{

MoveLayerCommand::MoveLayerCommand(ObjectsList& objectsList,
								   PandaObject* layer,
								   int pos)
	: m_objectsList(objectsList)
	, m_layer(layer)
{
	m_prevPos = m_objectsList.getObjectPosition(layer);

	if(!pos) // Insert at the front of the list
		m_newPos = 0;
	else // Find the position after the xth Layer
	{
		const auto& objects = m_objectsList.get();
		m_newPos = objects.size() - 1;
		int nbLayer = 0;
		for(int i=0, nb=objects.size(); i<nb; ++i)
		{
			auto object = objects[i].get();
			if(object != layer && dynamic_cast<BaseLayer*>(object))
			{
				++nbLayer;
				if(nbLayer == pos)
				{
					m_newPos = i+1;
					break;
				}
			}
		}
	}

	if(m_newPos < m_prevPos)
		++m_prevPos;

	setText("move layer");
}

void MoveLayerCommand::redo()
{
	m_objectsList.reinsertObject(m_layer, m_newPos);
}

void MoveLayerCommand::undo()
{
	m_objectsList.reinsertObject(m_layer, m_prevPos);
}

} // namespace panda
