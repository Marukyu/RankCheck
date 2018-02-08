#ifndef SRC_CLIENT_GUI3_MODELS_TABLEVIEWMODEL_HPP_
#define SRC_CLIENT_GUI3_MODELS_TABLEVIEWMODEL_HPP_

#include <Client/GUI3/Events/Callback.hpp>
#include <Client/GUI3/Types.hpp>
#include <Client/GUI3/Widget.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>
#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

namespace gui3
{

class TableViewModel
{
public:

	class Event
	{
	public:

		enum Type
		{
			CellDataChanged = 1 << 0,
			TableDataChanged = 1 << 1,
			ColumnCountChanged = 1 << 2,
			ColumnNameChanged = 1 << 3,
			RowLayoutChanged = 1 << 4,

			Any = 0x7fffffff
		};

		Event(Type type) :
			type(type),
			row(0),
			column(0)
		{
			assert(type != CellDataChanged && type != ColumnNameChanged);
		}

		Event(Type type, std::size_t column) :
			type(type),
			row(0),
			column(column)
		{
			assert(type == ColumnNameChanged);
		}

		Event(Type type, std::size_t row, std::size_t column) :
			type(type),
			row(row),
			column(row)
		{
			assert(type == CellDataChanged);
		}

		const Type type;
		const std::size_t row;
		const std::size_t column;
	};

	struct ColumnSizeHint
	{
		enum Mode
		{
			// Size is interpreted as pixel count (at identity transformation).
			Absolute,

			// Size is interpreted as fraction of non-absolute table width.
			Relative
		};

		ColumnSizeHint() :
			mode(Absolute),
			size(0)
		{
		}

		ColumnSizeHint(Mode mode, float size) :
			mode(mode),
			size(size)
		{
		}

		Mode mode;
		float size;
	};

	TableViewModel();
	virtual ~TableViewModel();

	virtual std::size_t getColumnCount() const = 0;
	virtual std::size_t getRowCount() const = 0;
	virtual std::string getColumnName(std::size_t column) const;
	virtual ColumnSizeHint getColumnSizeHint(std::size_t column) const;

	virtual float getRowYPosition(std::size_t row) const;
	virtual float getRowHeight(std::size_t row) const = 0;

	virtual Ptr<Widget> generateRow(std::size_t row, bool selected) = 0;
	virtual void onHideRow(std::size_t row);

	virtual void onSelectRow(std::size_t row);
	virtual void onDeselectRow(std::size_t row);

	void setParentWidget(Widget * parent);
	Widget * getParentWidget() const;

	void setColumnSize(std::size_t column, float size);
	float getColumnSize(std::size_t column) const;

	CallbackHandle<Event> addEventCallback(EventFunc<Event> func, int typeFilter, int order = 0);
	void fireEvent(Event event);

protected:

	virtual void onParentChanged(Widget * oldParent);
	virtual void onUpdateColumnSize(std::size_t column);

private:

	Widget * myParentWidget;
	std::vector<float> myColumnSizes;

	CallbackManager<Event> myCallbackManager;
};

}

#endif
