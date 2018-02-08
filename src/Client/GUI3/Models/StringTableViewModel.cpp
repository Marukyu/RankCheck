#include <Client/GUI3/Application.hpp>
#include <Client/GUI3/Models/StringTableViewModel.hpp>
#include <SFML/Graphics/Color.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>
#include <utility>

namespace gui3
{

StringTableViewModel::StringTableViewModel()
{
}

StringTableViewModel::~StringTableViewModel()
{
}

void StringTableViewModel::setDataModel(Ptr<DataModel> dataModel)
{
	if (myDataModel != dataModel)
	{
		myCellDataCallback.remove();
		myTableDataCallback.remove();
		myColumnCountCallback.remove();

		myDataModel = dataModel;

		if (dataModel)
		{
			myCellDataCallback = dataModel->addEventCallback([this](DataModel::Event event)
			{
				updateCellData(event.row, event.column);
				fireEvent(Event(Event::CellDataChanged, event.row, event.column));
			}, DataModel::Event::CellChanged);

			myTableDataCallback = dataModel->addEventCallback([this](DataModel::Event event)
			{
				updateTableData();
				fireEvent(Event(Event::TableDataChanged));
			}, DataModel::Event::TableDataChanged);

			myColumnCountCallback = dataModel->addEventCallback([this](DataModel::Event event)
			{
				updateColumnCount();
				fireEvent(Event(Event::ColumnCountChanged));
			}, DataModel::Event::ColumnCountChanged);
		}

		updateColumnCount();
	}
}

StringTableViewModel::DataModel * StringTableViewModel::getDataModel() const
{
	return myDataModel.get();
}

std::size_t StringTableViewModel::getColumnCount() const
{
	return myDataModel ? myDataModel->getColumnCount() : 0;
}

std::size_t StringTableViewModel::getRowCount() const
{
	return myDataModel ? myDataModel->getRowCount() : 0;
}

void StringTableViewModel::setColumnName(std::size_t column, std::string name)
{
	while (myColumnNames.size() < column)
	{
		myColumnNames.push_back("");
	}

	myColumnNames[column] = name;

	while (!myColumnNames.empty() && myColumnNames.back().empty())
	{
		myColumnNames.pop_back();
	}

	fireEvent(Event(Event::ColumnNameChanged, column));
}

std::string StringTableViewModel::getColumnName(std::size_t column) const
{
	if (myColumnNames.size() < column)
	{
		return myColumnNames[column];
	}
	else
	{
		return "";
	}
}

float StringTableViewModel::getRowYPosition(std::size_t row) const
{
	return getRowHeight(0) * row;
}

float StringTableViewModel::getRowHeight(std::size_t row) const
{
	static cfg::Float rowHeight("gui.models.table.string.rowHeight");
	if (getParentWidget() && getParentWidget()->getParentApplication())
	{
		return getParentWidget()->getParentApplication()->getConfig().get(rowHeight);
	}
	else
	{
		return 0;
	}
}

Ptr<Widget> StringTableViewModel::generateRow(std::size_t row, bool selected)
{
	Ptr<Row> rowWidget = myCache.make(row, this);

	rowWidget->setColumnCount(getColumnCount());
	for (std::size_t column = 0; column < getColumnCount(); ++column)
	{
		rowWidget->setColumnSize(column, getColumnSize(column));
	}

	updateRowData(row);

	if (selected)
	{
		onSelectRow(row);
	}

	updateRowFade(row);

	return rowWidget;
}

void StringTableViewModel::onHideRow(std::size_t row)
{
	myCache.remove(row);
}

void StringTableViewModel::onSelectRow(std::size_t row)
{
	if (myCache.has(row))
	{
		myCache.get(row)->setSelected(true);
	}
}

void StringTableViewModel::onDeselectRow(std::size_t row)
{
	if (myCache.has(row))
	{
		myCache.get(row)->setSelected(false);
	}
}

std::unique_ptr<StringTableViewModel::Cell> StringTableViewModel::generateCell(std::size_t column)
{
	return makeUnique<TextCell>();
}

void StringTableViewModel::onParentChanged(Widget* oldParent)
{
	myParentCallback.remove();

	if (getParentWidget())
	{
		myParentCallback = getParentWidget()->addStateCallback([this](StateEvent event)
		{
			handleParentStateEvent(event);
		}, StateEvent::Any);
	}

	updateAllRowFades();
	updateConfig();
}

void StringTableViewModel::onUpdateColumnSize(std::size_t column)
{
	for (auto & row : myCache)
	{
		row.second->setColumnSize(column, getColumnSize(column));
	}
}

void StringTableViewModel::handleParentStateEvent(StateEvent event)
{
	switch (event.type)
	{
	case StateEvent::ConfigChanged:
		updateConfig();
		break;

	case StateEvent::FocusGained:
	case StateEvent::FocusLost:
		updateAllRowFades();
		break;

	default:
		break;
	}
}

void StringTableViewModel::updateConfig()
{
	fireEvent(Event(Event::RowLayoutChanged));
}

StringTableViewModel::TextCell::TextCell()
{
	myText.make();
	myText->setTextAlignment(Text::ALIGN_LEFT, Text::ALIGN_CENTER);
}

Ptr<Widget> StringTableViewModel::TextCell::getWidget() const
{
	return myText;
}

void StringTableViewModel::TextCell::setData(std::string data)
{
	myText->setText(std::move(data));
}

StringTableViewModel::Row::Row(StringTableViewModel* parentModel)
{
	myParentModel = parentModel;

	addStateCallback([this](StateEvent event)
	{
		updateConfig();
	}, StateEvent::ConfigChanged);

	addStateCallback([this](StateEvent event)
	{
		updateColumns();
	}, StateEvent::Resized);

	mySelection.setDirection(Gradient::Vertical);
	mySelection.setZPosition(-1);
	mySelection.setVisible(false);

	addWidget(mySelection);

	updateConfig();
}

void StringTableViewModel::Row::setColumnCount(std::size_t count)
{
	std::size_t oldCount = myColumns.size();
	myColumns.resize(count);

	if (count > oldCount)
	{
		for (std::size_t i = oldCount; i < count; ++i)
		{
			auto & column = myColumns[i];
			column.size = 0;
			column.cell = myParentModel->generateCell(i);
			addWidget(*column.cell->getWidget());
		}
	}
}

void StringTableViewModel::Row::setColumnText(std::size_t column, std::string text)
{
	if (column >= myColumns.size())
	{
		return;
	}

	myColumns[column].cell->setData(text);
}

void StringTableViewModel::Row::setColumnSize(std::size_t column, float size)
{
	if (column >= myColumns.size())
	{
		return;
	}

	myColumns[column].size = size;
	updateColumns();
}

void StringTableViewModel::Row::setSelected(bool selected)
{
	mySelection.setVisible(selected);
}

void StringTableViewModel::Row::setFaded(bool faded)
{
	if (myIsFaded != faded)
	{
		myIsFaded = faded;
		updateGradients();
	}
}

static cfg::Float leftSpacing("gui.models.table.string.columnSpacing.left");
static cfg::Float rightSpacing("gui.models.table.string.columnSpacing.right");
static cfg::Float betweenSpacing("gui.models.table.string.columnSpacing.between");

void StringTableViewModel::Row::updateColumns()
{
	mySelection.setRect(0, 0, getSize().x, getSize().y);

	for (const auto & column : myColumns)
	{
		column.cell->getWidget()->setSize((int) (column.size * getScaleFactor()), getSize().y);
	}

	float position = config().get(leftSpacing);

	for (const auto & column : myColumns)
	{
		column.cell->getWidget()->setPosition((int) position, 0);
		position += column.size * getScaleFactor() + config().get(betweenSpacing);
	}
}

void StringTableViewModel::Row::updateConfig()
{
	updateGradients();
	updateColumns();
}

void StringTableViewModel::Row::updateGradients()
{
	// TODO: improve config API to simplify this mess without impacting performance.
	static const std::string configPrefix = "gui.models.table.string.selection.focused.";
	static const std::string configPrefixFaded = "gui.models.table.string.selection.unfocused.";

	sf::Color gradientTop;
	sf::Color gradientBottom;
	sf::Color outlineColor;
	float outlineThickness = 0;

	if (myIsFaded)
	{
		static cfg::Color configGradientTop(configPrefixFaded + "topColor");
		static cfg::Color configGradientBottom(configPrefixFaded + "bottomColor");
		static cfg::Color configOutlineColor(configPrefixFaded + "outlineColor");
		static cfg::Float configOutlineThickness(configPrefixFaded + "outlineThickness");

		gradientTop = config().get(configGradientTop);
		gradientBottom = config().get(configGradientBottom);
		outlineColor = config().get(configOutlineColor);
		outlineThickness = config().get(configOutlineThickness);
	}
	else
	{
		static cfg::Color configGradientTop(configPrefix + "topColor");
		static cfg::Color configGradientBottom(configPrefix + "bottomColor");
		static cfg::Color configOutlineColor(configPrefix + "outlineColor");
		static cfg::Float configOutlineThickness(configPrefix + "outlineThickness");

		gradientTop = config().get(configGradientTop);
		gradientBottom = config().get(configGradientBottom);
		outlineColor = config().get(configOutlineColor);
		outlineThickness = config().get(configOutlineThickness);
	}

	mySelection.setFirstColor(gradientTop);
	mySelection.setSecondColor(gradientBottom);

	mySelection.setOutlineColor(outlineColor);
	mySelection.setOutlineThickness(outlineThickness);
}

float StringTableViewModel::Row::getScaleFactor() const
{
	return (getSize().x - config().get(leftSpacing) - config().get(rightSpacing)
		- ((int) myColumns.size() - 1) * config().get(betweenSpacing)) / getSize().x;
}

void StringTableViewModel::updateCellData(std::size_t row, std::size_t column)
{
	if (myCache.has(row))
	{
		myCache.get(row)->setColumnText(column, getDataModel()->getCell(row, column));
	}
}

void StringTableViewModel::updateRowData(std::size_t row)
{
	for (std::size_t column = 0; column < getColumnCount(); ++column)
	{
		updateCellData(row, column);
	}
}

void StringTableViewModel::updateTableData()
{
	for (auto & row : myCache)
	{
		updateRowData(row.first);
	}
}

void StringTableViewModel::updateColumnCount()
{
	onUpdateColumnSize(0);
	updateTableData();
}

void StringTableViewModel::updateRowFade(std::size_t row)
{
	if (myCache.has(row))
	{
		bool faded = getParentWidget() != nullptr && !getParentWidget()->isFocused();
		myCache.get(row)->setFaded(faded);
	}
}

void StringTableViewModel::updateAllRowFades()
{
	for (auto & row : myCache)
	{
		updateRowFade(row.first);
	}
}

}
