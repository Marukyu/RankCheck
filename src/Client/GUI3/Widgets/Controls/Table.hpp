#ifndef SRC_CLIENT_GUI3_WIDGETS_CONTROLS_TABLE_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_CONTROLS_TABLE_HPP_

#include <Client/GUI3/Events/Callback.hpp>
#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Models/TableViewModel.hpp>
#include <Client/GUI3/Types.hpp>
#include <Client/GUI3/Widgets/Controls/Slider.hpp>
#include <Client/GUI3/Widgets/Panels/Panel.hpp>
#include <Shared/Utils/Range.hpp>
#include <cstddef>
#include <limits>
#include <map>
#include <set>

namespace gui3
{
class Canvas;
}

namespace gui3
{

/**
 * Table class for displaying arbitrary tabular data based on a view model.
 */
class Table : public Container
{
public:

	using SelectionSet = std::set<std::size_t>;

	static constexpr std::size_t NO_SELECTION = std::numeric_limits<std::size_t>::max();
	static constexpr std::size_t MULTIPLE_SELECTION = std::numeric_limits<std::size_t>::max() - 1;

	/**
	 * Default constructor.
	 */
	Table();

	/**
	 * Destructor.
	 */
	virtual ~Table();

	/**
	 * Assigns a view model to the table. This determines which data is displayed in the table, as well as how it is
	 * displayed. Passing a null pointer results in an empty table.
	 */
	void setViewModel(Ptr<TableViewModel> model);

	/**
	 * Returns the model currently assigned to the table.
	 */
	TableViewModel * getViewModel() const;

	/**
	 * Single-select: Selects a row, deselecting the previously selected row if one is selected.
	 * Multi-select: Adds a row to the selection.
	 */
	void select(std::size_t row);

	/**
	 * Deselects all currently selected rows. Does nothing if no row is currently selected.
	 */
	void deselect();

	/**
	 * If the specified row is currently selected, deselects it.
	 */
	void deselect(std::size_t row);

	/**
	 * If 0 rows are selected, returns NO_SELECTION.
	 * If 1 row is selected, returns the index of the selected row.
	 * If 2 or more rows are selected, returns MULTIPLE_SELECTION.
	 * 
	 * Use getSelectedRows() to handle multiple selected rows.
	 */
	std::size_t getSelectedRow() const;

	/**
	 * Returns the set of currently selected rows.
	 */
	const SelectionSet & getSelectedRows() const;

	/**
	 * Returns true if the specified row is currently selected, false otherwise.
	 */
	bool isRowSelected(std::size_t row) const;

	/**
	 * Changes whether multiple rows can be selected.
	 */
	void setMultipleSelectionAllowed(bool multiSelect);

	/**
	 * Returns whether multiple rows can be selected.
	 */
	bool isMultipleSelectionAllowed() const;

	/**
	 * Changes the number of pixels this table has been scrolled down by.
	 */
	void setVerticalScrollOffset(float offsetY);

	/**
	 * Returns the number of pixels this table has been scrolled down by.
	 */
	float getVerticalScrollOffset() const;

	/**
	 * Changes the visibility of the table background.
	 */
	void setBackgroundVisible(bool visible);

	/**
	 * Returns the visibility of the table background.
	 */
	bool isBackgroundVisible() const;

	/**
	 * Changes the visibility of the table's scrollbar.
	 */
	void setScrollBarVisible(bool visible);

	/**
	 * Returns the visibility of the table's scrollbar.
	 */
	bool isScrollBarVisible() const;

private:

	struct RowWidgetData
	{
		Ptr<Widget> widget;
		Callback<MouseEvent> mouseCallback;
	};

	using RowWidgetMap = std::map<std::size_t, RowWidgetData>;
	using RowIterator = RowWidgetMap::iterator;

	void handleStateEvent(StateEvent event);
	void handleModelEvent(TableViewModel::Event event);
	void handleRowMouseEvent(std::size_t row, MouseEvent event);

	void updateConfig();
	void updatePanelSize();
	void updateColumnSizes();
	void updateSelectionSet();
	void updateScrollbarBounds();

	float getVisibleRangeTop() const;
	float getVisibleRangeBottom() const;
	float getListHeight() const;

	void updateVisibleRows();
	void showRow(std::size_t row);
	void hideRow(std::size_t row);
	void updateRowWidgetRect(RowIterator rowIt);
	void updateAllRowWidgetRects();

	Range<std::size_t> locateVisibleRowRange(Range<float> targetRange) const;
	Range<std::size_t> locateVisibleRowRange(Range<float> targetRange, std::size_t hint) const;

	std::size_t locateVisibleRow(Range<float> targetRange, Range<std::size_t> searchRange, float splitHint = .5f) const;
	Range<std::size_t> expandRowToVisibleRange(Range<float> targetRange, std::size_t row) const;
	int compareRowToVisibleRange(Range<float> targetRange, std::size_t row) const;

	class Background : public Widget
	{
	public:
		Background(Table * table);
		virtual ~Background();

		void update();

	private:
		virtual void onRepaint(Canvas & canvas) override;

		Table * myTable;
	};

	Ptr<TableViewModel> myModel;

	bool myIsMultiSelectAllowed;
	bool myIsMultiSelectKeyPressed;

	SelectionSet mySelectedRows;
	Range<std::size_t> myVisibleRows;

	Panel myPanel;
	Slider myScrollBar;
	Background myBackground;
	RowWidgetMap myRowWidgets;
};

}

#endif
