#include <Client/GUI3/Models/TableViewModel.hpp>
#include <algorithm>

namespace gui3
{

TableViewModel::TableViewModel() :
	myParentWidget(nullptr)
{
}

TableViewModel::~TableViewModel()
{
}

std::string TableViewModel::getColumnName(std::size_t column) const
{
	return "";
}

TableViewModel::ColumnSizeHint TableViewModel::getColumnSizeHint(std::size_t column) const
{
	return ColumnSizeHint(ColumnSizeHint::Relative, 1.f / getColumnCount());
}

float TableViewModel::getRowYPosition(std::size_t row) const
{
	return getRowHeight(0) * row;
}

void TableViewModel::onHideRow(std::size_t row)
{
}

void TableViewModel::onSelectRow(std::size_t row)
{
}

void TableViewModel::onDeselectRow(std::size_t row)
{
}

void TableViewModel::setParentWidget(Widget* parent)
{
	if (myParentWidget != parent)
	{
		std::swap(myParentWidget, parent);
		onParentChanged(parent);
	}
}

Widget* TableViewModel::getParentWidget() const
{
	return myParentWidget;
}

void TableViewModel::onParentChanged(Widget* oldParent)
{
}

void TableViewModel::setColumnSize(std::size_t column, float size)
{
	if (column >= myColumnSizes.size())
	{
		myColumnSizes.resize(column + 1);
	}
	myColumnSizes[column] = size;
	onUpdateColumnSize(column);
}

float TableViewModel::getColumnSize(std::size_t column) const
{
	if (column >= myColumnSizes.size())
	{
		return 0.f;
	}
	else
	{
		return myColumnSizes[column];
	}
}

void TableViewModel::onUpdateColumnSize(std::size_t column)
{
}

CallbackHandle<TableViewModel::Event> TableViewModel::addEventCallback(EventFunc<Event> func, int typeFilter, int order)
{
	return myCallbackManager.addCallback(func, typeFilter, order);
}

void TableViewModel::fireEvent(Event event)
{
	myCallbackManager.fireCallback(event.type, event);
}

}
