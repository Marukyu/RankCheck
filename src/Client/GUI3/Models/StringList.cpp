#include <Client/GUI3/Models/StringList.hpp>

namespace gui3
{

StringList::StringList()
{
}

StringList::StringList(std::vector<std::string> strings) :
	myStrings(strings)
{
}

StringList::~StringList()
{
}

void StringList::setStrings(std::vector<std::string> strings)
{
	myStrings = strings;
	fireEvent(Event(Event::TableDataChanged));
}

const std::vector<std::string> & StringList::getStrings() const
{
	return myStrings;
}

void StringList::setString(std::size_t index, std::string string)
{
	if (index < getSize())
	{
		myStrings[index] = string;
		fireEvent(Event(Event::CellChanged, index, 0));
	}
}

std::string StringList::getString(std::size_t index) const
{
	if (index < getSize())
	{
		return myStrings[index];
	}
	else
	{
		return "";
	}
}

void StringList::resize(std::size_t size)
{
	myStrings.resize(size);
	fireEvent(Event(Event::TableDataChanged));
}

std::size_t StringList::getSize() const
{
	return myStrings.size();
}

std::string StringList::getCell(std::size_t row, std::size_t column) const
{
	if (column != 0)
	{
		return "";
	}
	else
	{
		return getString(row);
	}
}

std::size_t StringList::getRowCount() const
{
	return getSize();
}

std::size_t StringList::getColumnCount() const
{
	return 1;
}

}
