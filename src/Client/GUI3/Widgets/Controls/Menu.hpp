#ifndef SRC_CLIENT_GUI3_WIDGETS_CONTROLS_MENU_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_CONTROLS_MENU_HPP_

#include <Client/GUI3/Models/StringTableDataModel.hpp>
#include <Client/GUI3/Models/StringTableViewModel.hpp>
#include <Client/GUI3/Widgets/Controls/Table.hpp>
#include <cstddef>

namespace gui3
{
class Canvas;

/**
 * 
 */
class Menu : public Container
{
public:

	struct Entry
	{
		enum Type
		{
			Normal,
			Separator,
			Checkbox,
		};

		std::string text;
		bool checked;
		bool enabled;

		static Entry makeText(std::string text, bool enabled = true);
		static Entry makeSeparator();
		static Entry makeCheckbox(std::string text, bool checked = false, bool enabled = true);
	};

private:

	class MenuTableViewModel : public StringTableViewModel
	{
	public:

	private:

		class CheckboxCell : public Cell
		{
		public:
			CheckboxCell();
			~CheckboxCell() = default;

			virtual Ptr<Widget> getWidget() const override;
			virtual void setData(std::string data) override;

		private:

			Ptr<Text> myText;
		};

	};

	class MenuTableDataModel : public StringTableDataModel
	{
	public:

		MenuTableDataModel();
		virtual ~MenuTableDataModel();

		virtual std::string getCell(std::size_t row, std::size_t column) const override;
		virtual std::size_t getRowCount() const override;
		virtual std::size_t getColumnCount() const override;

		std::vector<Entry> & getEntries();
		const std::vector<Entry> & getEntries() const;

	private:

		std::vector<Entry> myEntries;
	};

public:

	class EntryList
	{
	public:
		void setEntry(std::size_t index, Entry entry);
		const Entry & getEntry(std::size_t index) const;

		std::size_t getEntryCount() const;
		void clear();

		void appendEntry(Entry entry);
		void insertEntry(std::size_t index, Entry entry);
		void removeEntry(std::size_t index);

	private:

		EntryList(MenuTableDataModel * model);

		MenuTableDataModel * myModel;
	};

	/**
	 * Default constructor.
	 */
	Menu();

	/**
	 * Destructor.
	 */
	virtual ~Menu();

private:

	virtual void onRepaint(Canvas & canvas) override;

	Table myTable;
};

}

#endif
