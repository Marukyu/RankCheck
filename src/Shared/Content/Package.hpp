#ifndef PACKAGE_HPP
#define PACKAGE_HPP

#include "Shared/Utils/DataStream.hpp"
#include "Shared/Utils/StringStream.hpp"

class Package
{

public:

	 enum ContentType
	 {
		 CTError     = -1,
		 CTGeneric   = 0,
		 CTUnknown   = 1,
		 CTDirectory = 2,
		 CTWorld     = 3,
		 CTText	     = 4,
		 CTImage	 = 5,
		 CTSound	 = 6,
	 };

	Package();
	~Package();

	static bool compile(const std::string & path, const std::string & outputFile, StringStream & log, bool permissive =
		false);

	bool openFile(const std::string & filename);

	void close();

	// selects the next content in the package. returns false if end was reached.
	bool nextContent();

	// selects the first content in the package. returns false if the package is empty or closed.
	bool firstContent();

	// selects a content by its id. specify a file extension to check for a specific type, or none
	// to check for any type.
	bool select(const std::string & id);

	// clears the content data buffer.
	void deselect();

	// returns true if the package contains a specific content, false otherwise. same extension
	// rules apply as with select().
	bool hasContent(const std::string & id);

	// returns information about the currently selected content.
	std::string getContentId();
	const std::vector<char> & getContentData();
	ContentType getContentType();
	unsigned int getContentSize();

	std::size_t getContentCount();

	static sf::Uint32 nameHash(const std::string & id);
	static sf::Uint8 nameHint(const std::string & id);

private:

	static const std::string FileIDent;
	static const std::vector<char> returnEmpty;
	static const unsigned int identSize, tableBegin, tableSize, dataBegin;

	sf::Uint32 findContent(const std::string & id);
	bool select(sf::Uint32 pos);
	bool readData();
	bool checkHeader();

	DataStream myStream;

	sf::Uint32 myCurrentPosition;
	std::string myCurrentContentId;
	sf::Uint8 myCurrentContentType;
	std::vector<char> myCurrentContentData;
	bool myIsDataRead;

	sf::Uint32 myContentCount;
	std::vector<sf::Uint32> myHintTable;
};

#endif

