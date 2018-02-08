#ifndef SRC_CLIENT_GUI3_MODELS_STRINGTABLEDATAMODEL_HPP_
#define SRC_CLIENT_GUI3_MODELS_STRINGTABLEDATAMODEL_HPP_

#include <Client/GUI3/Events/Callback.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>
#include <cassert>
#include <cstddef>
#include <string>

namespace gui3
{

class StringTableDataModel
{
public:

	class Event
	{
	public:

		enum Type
		{
			CellChanged = 1 << 0,
			ColumnCountChanged = 1 << 1,
			TableDataChanged = 1 << 2,

			Any = 0x7fffffff
		};

		Event(Type type) :
			type(type),
			row(0),
			column(0)
		{
			assert(type != CellChanged);
		}

		Event(Type type, std::size_t row, std::size_t column) :
			type(type),
			row(row),
			column(row)
		{
			assert(type == CellChanged);
		}

		const Type type;
		const std::size_t row;
		const std::size_t column;
	};

	StringTableDataModel();
	virtual ~StringTableDataModel();

	virtual std::string getCell(std::size_t row, std::size_t column) const = 0;
	virtual std::size_t getRowCount() const = 0;
	virtual std::size_t getColumnCount() const = 0;

	CallbackHandle<Event> addEventCallback(EventFunc<Event> func, int typeFilter, int order = 0);
	void fireEvent(Event event);

private:

	CallbackManager<Event> myCallbackManager;
};

}

#endif
