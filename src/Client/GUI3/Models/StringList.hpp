#ifndef SRC_CLIENT_GUI3_MODELS_STRINGLIST_HPP_
#define SRC_CLIENT_GUI3_MODELS_STRINGLIST_HPP_

#include <Client/GUI3/Models/StringTableDataModel.hpp>
#include <cstddef>
#include <string>
#include <vector>

namespace gui3
{

class StringList : public StringTableDataModel
{
public:
	StringList();
	StringList(std::vector<std::string> strings);
	virtual ~StringList();

	void setStrings(std::vector<std::string> strings);
	const std::vector<std::string> & getStrings() const;

	void setString(std::size_t index, std::string string);
	std::string getString(std::size_t index) const;

	void resize(std::size_t size);
	std::size_t getSize() const;

	virtual std::string getCell(std::size_t row, std::size_t column) const override;
	virtual std::size_t getRowCount() const override;
	virtual std::size_t getColumnCount() const override;

private:

	std::vector<std::string> myStrings;
};

}

#endif
