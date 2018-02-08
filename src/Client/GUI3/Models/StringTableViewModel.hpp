#ifndef SRC_CLIENT_GUI3_MODELS_STRINGTABLEVIEWMODEL_HPP_
#define SRC_CLIENT_GUI3_MODELS_STRINGTABLEVIEWMODEL_HPP_

#include <Client/GUI3/Container.hpp>
#include <Client/GUI3/Events/Callback.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Models/StringTableDataModel.hpp>
#include <Client/GUI3/Models/TableViewModel.hpp>
#include <Client/GUI3/Types.hpp>
#include <Client/GUI3/Utils/ViewModelCache.hpp>
#include <Client/GUI3/Widgets/Controls/Text.hpp>
#include <Client/GUI3/Widgets/Graphics/Gradient.hpp>
#include <cstddef>
#include <string>
#include <vector>

namespace gui3
{
class StringTableDataModel;

class StringTableViewModel : public TableViewModel
{
public:

	using DataModel = StringTableDataModel;

	StringTableViewModel();
	virtual ~StringTableViewModel();

	void setDataModel(Ptr<DataModel> dataModel);
	DataModel * getDataModel() const;

	virtual std::size_t getColumnCount() const override;
	virtual std::size_t getRowCount() const override;

	void setColumnName(std::size_t column, std::string name);
	virtual std::string getColumnName(std::size_t column) const;

	virtual float getRowYPosition(std::size_t row) const override;
	virtual float getRowHeight(std::size_t row) const override;

	virtual Ptr<Widget> generateRow(std::size_t row, bool selected) override;
	virtual void onHideRow(std::size_t row) override;

	virtual void onSelectRow(std::size_t row) override;
	virtual void onDeselectRow(std::size_t row) override;

protected:

	/**
	 * Base class for cell renderers.
	 *
	 * Inherit this class and override "generateCell" in the view model to customize the table renderer.
	 *
	 * Subclasses are expected to hold a constant pointer to one widget instance that will be used for cell rendering.
	 */
	class Cell
	{
	public:
		Cell() = default;
		virtual ~Cell() = default;

		/**
		 * Returns the widget used to render this cell. It will be automatically resized by the view model.
		 *
		 * The returned pointer must be identical across all calls to this function.
		 */
		virtual Ptr<Widget> getWidget() const = 0;

		/**
		 * Called whenever the cell's value changes in the data model. Implementations should update the widget with the
		 * new data.
		 */
		virtual void setData(std::string data) = 0;
	};

	/**
	 * Generates a widget to be used as a table cell for the specified column.
	 *
	 * Override this function to force a cell to be rendered differently.
	 *
	 * Produces a "TextCell" by default, which displays the table cell content using a text widget.
	 */
	virtual std::unique_ptr<Cell> generateCell(std::size_t column);

	/**
	 * Called whenever the table view model's parent widget changes.
	 *
	 * Overridden to reassign callbacks.
	 */
	virtual void onParentChanged(Widget * oldParent) override;

	/**
	 * Called whenever the size of a column is changed.
	 *
	 * Overridden to resize cells within row widgets.
	 */
	virtual void onUpdateColumnSize(std::size_t column) override;

private:

	class TextCell : public Cell
	{
	public:
		TextCell();
		~TextCell() = default;

		virtual Ptr<Widget> getWidget() const override;
		virtual void setData(std::string data) override;

	private:

		Ptr<Text> myText;
	};

	class Row : public Container
	{
	public:

		Row(StringTableViewModel * parentModel);

		void setColumnCount(std::size_t count);
		void setColumnText(std::size_t column, std::string text);
		void setColumnSize(std::size_t column, float size);
		void setSelected(bool selected);
		void setFaded(bool faded);

	private:

		void updateColumns();
		void updateConfig();
		void updateGradients();
		float getScaleFactor() const;

		struct Column
		{
			std::unique_ptr<Cell> cell;
			float size;
		};

		StringTableViewModel * myParentModel;
		std::vector<Column> myColumns;
		bool myIsFaded;
		Gradient mySelection;
		Gradient mySelectionOutline;
	};

	void handleParentStateEvent(StateEvent event);
	void updateConfig();

	void updateCellData(std::size_t row, std::size_t column);
	void updateRowData(std::size_t row);
	void updateTableData();
	void updateColumnCount();

	void updateRowFade(std::size_t row);
	void updateAllRowFades();

	ViewModelCache<Row> myCache;
	Ptr<DataModel> myDataModel;
	std::vector<std::string> myColumnNames;

	Callback<StateEvent> myParentCallback;
	Callback<DataModel::Event> myCellDataCallback;
	Callback<DataModel::Event> myTableDataCallback;
	Callback<DataModel::Event> myColumnCountCallback;
};

}

#endif
