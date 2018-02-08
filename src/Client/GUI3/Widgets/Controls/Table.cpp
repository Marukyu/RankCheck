#include <Client/GUI3/Events/Key.hpp>
#include <Client/GUI3/Events/KeyEvent.hpp>
#include <Client/GUI3/Rendering/Primitives/Box.hpp>
#include <Client/GUI3/Utils/Canvas.hpp>
#include <Client/GUI3/Widgets/Controls/Table.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/Error.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>
#include <Shared/Utils/MiscMath.hpp>
#include <algorithm>
#include <utility>

namespace gui3
{

Table::Table() :
	myIsMultiSelectAllowed(false),
	myIsMultiSelectKeyPressed(false),
	myBackground(this)
{
	addStateCallback([this](StateEvent event)
	{
		handleStateEvent(event);
	}, StateEvent::Any);

	myScrollBar.addEventCallback([this](Slider::Event event)
	{
		updateVisibleRows();
	}, Slider::Event::Changed);

	addKeyboardCallback([this](KeyEvent event)
	{
		if (event.key.isControl())
		{
			if (event.type == KeyEvent::Press)
			{
				myIsMultiSelectKeyPressed = true;
			}
			else if (event.type == KeyEvent::Release)
			{
				myIsMultiSelectKeyPressed = false;
			}
		}
	}, KeyEvent::Press | KeyEvent::Release);

	myBackground.setZPosition(-1);
	myScrollBar.linkWithWidget(myPanel);
	myScrollBar.setDirection(Slider::Vertical);

	addWidget(myBackground);
	addWidget(myPanel);
	addWidget(myScrollBar);
}

Table::~Table()
{

}

void Table::setViewModel(Ptr<TableViewModel> model)
{
	if (myModel != model)
	{
		myRowWidgets.clear();

		myModel = model;

		if (model != nullptr)
		{
			model->setParentWidget(this);
			model->addEventCallback([this](TableViewModel::Event event)
			{
				handleModelEvent(event);
			}, TableViewModel::Event::Any);
		}

		// Clean up selection set from entries past the row count.
		updateSelectionSet();

		updateColumnSizes();
		updateVisibleRows();
	}
}

TableViewModel* Table::getViewModel() const
{
	return myModel.get();
}

void Table::select(std::size_t row)
{
	if (!isMultipleSelectionAllowed())
	{
		deselect();
	}

	if (getViewModel() != nullptr && row < getViewModel()->getRowCount())
	{
		if (mySelectedRows.insert(row).second)
		{
			getViewModel()->onSelectRow(row);
		}
	}
}

void Table::deselect()
{
	SelectionSet selection;
	std::swap(mySelectedRows, selection);

	if (getViewModel() != nullptr)
	{
		for (auto row : selection)
		{
			getViewModel()->onDeselectRow(row);
		}
	}
}

void Table::deselect(std::size_t row)
{
	if (getViewModel() != nullptr && row < getViewModel()->getRowCount())
	{
		if (mySelectedRows.erase(row))
		{
			getViewModel()->onDeselectRow(row);
		}
	}
}

std::size_t Table::getSelectedRow() const
{
	if (getSelectedRows().empty())
	{
		return NO_SELECTION;
	}
	else if (getSelectedRows().size() == 1)
	{
		return *getSelectedRows().begin();
	}
	else
	{
		return MULTIPLE_SELECTION;
	}
}

const Table::SelectionSet & Table::getSelectedRows() const
{
	return mySelectedRows;
}

bool Table::isRowSelected(std::size_t row) const
{
	return getSelectedRows().count(row);
}

void Table::setMultipleSelectionAllowed(bool multiSelect)
{
	myIsMultiSelectAllowed = multiSelect;

	if (getSelectedRow() == MULTIPLE_SELECTION)
	{
		deselect();
	}
}

bool Table::isMultipleSelectionAllowed() const
{
	return myIsMultiSelectAllowed;
}

void Table::setVerticalScrollOffset(float offsetY)
{
	myScrollBar.setValue(offsetY);
}

float Table::getVerticalScrollOffset() const
{
	return myScrollBar.getValue();
}

void Table::setBackgroundVisible(bool visible)
{
	if (visible != isBackgroundVisible())
	{
		myBackground.setVisible(visible);

		// Panel does not have margin in background-less mode; needs update.
		updatePanelSize();
	}
}

bool Table::isBackgroundVisible() const
{
	return myBackground.isVisible();
}

void Table::setScrollBarVisible(bool visible)
{
	if (visible != isScrollBarVisible())
	{
		myScrollBar.setVisible(visible);

		// Panel size depends on scroll bar visibility; needs update.
		updatePanelSize();
	}
}

bool Table::isScrollBarVisible() const
{
	return myScrollBar.isVisible();
}

void Table::handleStateEvent(StateEvent event)
{
	switch (event.type)
	{
	case StateEvent::ConfigChanged:
		updateConfig();
		break;

	case StateEvent::Resized:
		updatePanelSize();
		break;

	default:
		break;
	}
}

void Table::handleModelEvent(TableViewModel::Event event)
{
	switch (event.type)
	{
	case TableViewModel::Event::CellDataChanged:
		break;

	case TableViewModel::Event::TableDataChanged:

		// Clean up selection set from entries past the row count (in case row count changed).
		updateSelectionSet();

		// Fallthrough...
	case TableViewModel::Event::RowLayoutChanged:

		// Update range of displayed rows (in case row height changed).
		updateScrollbarBounds();
		updateVisibleRows();
		break;

	default:
		break;
	}
}

void Table::handleRowMouseEvent(std::size_t row, MouseEvent event)
{
	switch (event.type)
	{
	case MouseEvent::ButtonDown:
		if (event.button == MouseEvent::Left)
		{
			if (myIsMultiSelectKeyPressed)
			{
				if (isRowSelected(row))
				{
					deselect(row);
				}
				else
				{
					select(row);
				}
			}
			else
			{
				deselect();
				select(row);
			}
		}
		break;

	default:
		break;
	}
}

void Table::updateConfig()
{
	static cfg::Float configScrollBarSpeed("gui.widgets.table.scrollBar.speed");
	static cfg::Bool configScrollBarSmooth("gui.widgets.table.scrollBar.smooth");

	myScrollBar.setScrollSpeed(config().get(configScrollBarSpeed));
	myScrollBar.setSmoothScrollingEnabled(config().get(configScrollBarSmooth));

	updatePanelSize();
}

void Table::updatePanelSize()
{
	static cfg::Float configMargin("gui.widgets.table.margin");
	static cfg::Float configScrollBarSize("gui.widgets.table.scrollBar.size");
	float margin = 0;
	float scrollBarSize = 0;

	// Only enable margin if table is drawn with a background.
	if (isBackgroundVisible())
	{
		margin = config().get(configMargin);
	}

	// Only reserve space for scroll bar if it is visible.
	if (isScrollBarVisible())
	{
		scrollBarSize = config().get(configScrollBarSize);
	}

	myPanel.setRect(margin, margin, getSize().x - margin - scrollBarSize, getSize().y - 2 * margin);
	myScrollBar.setRect(getSize().x - scrollBarSize, 0, scrollBarSize, getSize().y);

	updateColumnSizes();
	updateScrollbarBounds();
	updateVisibleRows();
}

void Table::updateColumnSizes()
{
	if (getViewModel())
	{
		// Assume full table width as initial space for relative-sized columns.
		float relativeSpace = myPanel.getSize().x;

		// Loop over all absolute table columns (first pass to determine actual relative space).
		for (std::size_t column = 0; column < getViewModel()->getColumnCount(); ++column)
		{
			// Get size preference info for current column.
			auto sizeHint = getViewModel()->getColumnSizeHint(column);

			// Only check absolute columns in this pass.
			if (sizeHint.mode == TableViewModel::ColumnSizeHint::Absolute)
			{
				// Subtract absolute column width from available space for relative columns.
				relativeSpace -= sizeHint.size;
			}
		}

		// Don't allow negative values for relative space.
		relativeSpace = std::max(0.f, relativeSpace);

		// Loop over all table columns (second pass to assign actual sizes).
		for (std::size_t column = 0; column < getViewModel()->getColumnCount(); ++column)
		{
			auto sizeHint = getViewModel()->getColumnSizeHint(column);
			float columnSize = 0.f;

			switch (sizeHint.mode)
			{
			case TableViewModel::ColumnSizeHint::Absolute:
				// Treat size info as absolute size in pixels.
				columnSize = sizeHint.size;
				break;

			case TableViewModel::ColumnSizeHint::Relative:
				// Treat size info as normalized percentage (0.0 - 1.0).
				columnSize = relativeSpace * sizeHint.size;
				break;

			default:
				break;
			}

			getViewModel()->setColumnSize(column, columnSize);
		}
	}
}

void Table::updateSelectionSet()
{
	std::size_t rowCount = getViewModel() ? getViewModel()->getRowCount() : 0;

	for (auto it = mySelectedRows.begin(); it != mySelectedRows.end();)
	{
		if (*it > rowCount)
		{
			it = mySelectedRows.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Table::updateScrollbarBounds()
{
	myScrollBar.setBounds(0, getListHeight() - myPanel.getSize().y);
}

float Table::getVisibleRangeTop() const
{
	return getVerticalScrollOffset();
}

float Table::getVisibleRangeBottom() const
{
	return getVerticalScrollOffset() + myPanel.getSize().y;
}

float Table::getListHeight() const
{
	if (getViewModel() && getViewModel()->getRowCount() != 0)
	{
		return getViewModel()->getRowYPosition(getViewModel()->getRowCount() - 1)
			+ getViewModel()->getRowHeight(getViewModel()->getRowCount() - 1);
	}
	else
	{
		return 0;
	}
}

void Table::updateVisibleRows()
{
	if (getViewModel() == nullptr)
	{
		myVisibleRows.start = 0;
		myVisibleRows.end = 0;
	}
	else
	{
		std::size_t hint = myVisibleRows.middle();

		auto newVisibleRows = locateVisibleRowRange(Range<float>(getVisibleRangeTop(), getVisibleRangeBottom()), hint);

		if (newVisibleRows.compare(myVisibleRows) == 0)
		{
			// Intersection: this needs to be handled with one big loop.
			auto mergedRange = myVisibleRows.merge(newVisibleRows);

			for (std::size_t row = mergedRange.start; row < mergedRange.end; ++row)
			{
				bool visibleOld = myVisibleRows.contains(row);
				bool visibleNew = newVisibleRows.contains(row);

				if (visibleOld && visibleNew)
				{
					continue;
				}

				if (visibleNew)
				{
					// Row is now visible. It needs to be generated.
					showRow(row);
				}
				else if (visibleOld)
				{
					// Row is no longer visible. It needs to be hidden.
					hideRow(row);
				}
			}
		}
		else
		{
			// Handle non-intersecting visibility ranges with two loops.
			for (std::size_t row = myVisibleRows.start; row < myVisibleRows.end; ++row)
			{
				hideRow(row);
			}

			for (std::size_t row = newVisibleRows.start; row < newVisibleRows.end; ++row)
			{
				showRow(row);
			}
		}

		myVisibleRows = newVisibleRows;
	}

	updateAllRowWidgetRects();
}

void Table::showRow(std::size_t row)
{
	RowWidgetData data;
	data.widget = getViewModel()->generateRow(row, isRowSelected(row));
	data.widget->setVisible(true);
	data.mouseCallback = data.widget->addMouseCallback([this, row](MouseEvent event)
	{
		handleRowMouseEvent(row, event);
	}, MouseEvent::Any);

	myPanel.add(data.widget);

	myRowWidgets[row] = std::move(data);
}

void Table::hideRow(std::size_t row)
{
	getViewModel()->onHideRow(row);

	auto it = myRowWidgets.find(row);
	if (it != myRowWidgets.end())
	{
		myPanel.remove(*it->second.widget);
		it->second.widget->setVisible(false);
		myRowWidgets.erase(row);
	}
}

void Table::updateRowWidgetRect(RowIterator rowIt)
{
	if (rowIt != myRowWidgets.end())
	{
		rowIt->second.widget->setRect(0, getViewModel()->getRowYPosition(rowIt->first) - getVerticalScrollOffset(),
			myPanel.getSize().x, getViewModel()->getRowHeight(rowIt->first));
	}
}

void Table::updateAllRowWidgetRects()
{
	for (auto it = myRowWidgets.begin(); it != myRowWidgets.end(); ++it)
	{
		updateRowWidgetRect(it);
	}
}

Range<std::size_t> Table::locateVisibleRowRange(Range<float> targetRange) const
{
	if (getViewModel() == nullptr)
	{
		return Range<std::size_t>();
	}

	std::size_t visibleRow = locateVisibleRow(targetRange, Range<std::size_t>(0, getViewModel()->getRowCount()),
		targetRange.middle() / getListHeight());
	return expandRowToVisibleRange(targetRange, visibleRow);
}

Range<std::size_t> Table::locateVisibleRowRange(Range<float> targetRange, std::size_t hint) const
{
	if (getViewModel() == nullptr)
	{
		return Range<std::size_t>();
	}

	// Try hint first.
	if (compareRowToVisibleRange(targetRange, hint) == 0)
	{
		return expandRowToVisibleRange(targetRange, hint);
	}

	return locateVisibleRowRange(targetRange);
}

std::size_t Table::locateVisibleRow(Range<float> targetRange, Range<std::size_t> searchRange, float splitHint) const
{
	if (searchRange.size() == 0)
	{
		return searchRange.start;
	}

	if (searchRange.start > searchRange.end)
	{
		throw Error("Invalid bounds for Table::locateVisibleRow");
	}

	// Instead of simply taking middle index, use hint (based on current results) to approximate index.
	std::size_t mid = searchRange.start * (1.f - splitHint) + searchRange.end * splitHint;
	mid = clamp<std::size_t>(searchRange.start, mid, searchRange.end - 1);

	Range<float> midRange;
	midRange.start = getViewModel()->getRowYPosition(mid);
	midRange.end = midRange.start + getViewModel()->getRowHeight(mid);

	float newHint = midRange.middle() / getListHeight();
	int comp = midRange.compare(targetRange);

	if (comp > 0)
	{
		// Row is too low (greater Y-coordinate).
		return locateVisibleRow(targetRange, Range<std::size_t>(searchRange.start, mid), newHint);
	}
	else if (comp < 0)
	{
		// Row is too high (smaller Y-coordinate).
		return locateVisibleRow(targetRange, Range<std::size_t>(mid + 1, searchRange.end), newHint);
	}
	else
	{
		// Row is within search section.
		return mid;
	}
}

Range<std::size_t> Table::expandRowToVisibleRange(Range<float> targetRange, std::size_t row) const
{
	std::size_t start = row;
	std::size_t end = row;

	// Check if starting row itself is visible.
	bool initialRowVisible = (compareRowToVisibleRange(targetRange, row) == 0);

	if (initialRowVisible)
	{
		// Expand backwards.
		while (start != 0 && compareRowToVisibleRange(targetRange, start - 1) == 0)
		{
			--start;
		}

		// Expand forwards.
		while (end < getViewModel()->getRowCount() - 1 && compareRowToVisibleRange(targetRange, end + 1) == 0)
		{
			++end;
		}

		// Return inclusive range.
		return Range<std::size_t>(start, end + 1);
	}
	else
	{
		// If the initial row is not visible, return empty range.
		return Range<std::size_t>(row, row);
	}
}

int Table::compareRowToVisibleRange(Range<float> targetRange, std::size_t row) const
{
	Range<float> rowRange;
	rowRange.start = getViewModel()->getRowYPosition(row);
	rowRange.end = rowRange.start + getViewModel()->getRowHeight(row);
	return rowRange.compare(targetRange);
}

Table::Background::Background(Table * table) :
	myTable(table)
{
	myTable->addStateCallback([this](StateEvent event)
	{
		update();
	}, StateEvent::Resized | StateEvent::FocusGained | StateEvent::FocusLost);

	update();
}

Table::Background::~Background()
{
}

void Table::Background::update()
{
	setRect(myTable->getBaseRect());
	repaint();
}

void Table::Background::onRepaint(Canvas & canvas)
{
	int flags = primitives::Box::Background | primitives::Box::Dark;

	if (myTable->isFocused())
	{
		flags |= primitives::Box::Pressed | primitives::Box::Focused;
	}

	canvas.draw(primitives::Box(getBaseRect(), flags));
}

}
