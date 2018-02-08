#include <Client/GUI3/Models/StringTableDataModel.hpp>

namespace gui3
{

StringTableDataModel::StringTableDataModel()
{
}

StringTableDataModel::~StringTableDataModel()
{
}

CallbackHandle<StringTableDataModel::Event> StringTableDataModel::addEventCallback(
	EventFunc<Event> func, int typeFilter, int order)
{
	return myCallbackManager.addCallback(func, typeFilter, order);
}

void StringTableDataModel::fireEvent(Event event)
{
	myCallbackManager.fireCallback(event.type, event);
}

}
