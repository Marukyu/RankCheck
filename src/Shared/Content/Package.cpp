#include <cassert>
#include <fstream>

#include "Shared/Content/Package.hpp"
#include "Shared/Utils/Hash.hpp"
#include "Shared/Utils/Zlib.hpp"
#include "Shared/Utils/Utilities.hpp"

const std::string Package::FileIDent = "WSP0";
const std::vector<char> Package::returnEmpty;
const unsigned int Package::identSize = FileIDent.size(), Package::tableBegin = 8, Package::tableSize = 256,
	Package::dataBegin = Package::tableBegin + tableSize * sizeof(sf::Uint32);

namespace priv
{
std::string fileNameToHashable(std::string filename)
{
	return removeFileExtension(filename);
}

bool hashCompare(std::string a, std::string b)
{
	// convert file names to hashable content names.
	a = fileNameToHashable(a);
	b = fileNameToHashable(b);

	// compare name hints first; then hashes in case of collision.
	return
		(Package::nameHint(a) < Package::nameHint(b)) ?
			true : (Package::nameHint(a) == Package::nameHint(b) && Package::nameHash(a) < Package::nameHash(b));
}
}

Package::Package()
{
	myStream.setIndexSize(2);
	close();
}
Package::~Package()
{

}

bool Package::compile(const std::string & path, const std::string & outputFile, StringStream & log, bool permissive)
{
	const std::string lgInfo = "[INFO] ", lgWarn = "[WARNING] ", lgErr = "[ERROR] ";

	log << lgInfo << "Compiling package from path " << path << '\n';

	std::vector<std::string> files;

	if (!listFiles(path, files, true, false))
		return false;

	if (!listDirectories(path, files, true, false))
		return false;

	std::sort(files.begin(), files.end(), priv::hashCompare);

	DataStream strm;
	strm.setIndexSize(2);

	// open target file.
	if (!strm.openOutFile(outputFile))
	{
		log << lgErr << "Failed to open file " << outputFile << "!" << '\n';
		return false;
	}

	// identifier.
	strm.addData(FileIDent.data(), identSize);

	// content count; update this later.
	sf::Uint32 contentCount = 0;
	strm << contentCount;

	// allocate space for the table.
	std::vector<sf::Uint32> hintTable;
	hintTable.resize(tableSize);
	std::fill(hintTable.begin(), hintTable.end(), 0);
	strm.addData(hintTable.data(), tableSize * sizeof(sf::Uint32));

	// begin adding content.
	for (std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		std::string curConName = *it;
		std::string curFileExt = getFileExtension(*it);

		if (curFileExt == ".old")
		{
			// ignore backup files.
			continue;
		}

		sf::Uint32 curConSize;
		// TODO!!!
		sf::Uint8 curConType = isDirectory(path + "/" + *it) ? CTDirectory : CTUnknown;
		bool curCompress = false;
		std::ifstream fileIn;

		// skip size detection for directories.
		if (curConType == CTDirectory)
		{
			// default size to 0 for virtual file entry without data.
			curConSize = 0;
		}
		else
		{
			// determine file size and readability.
			fileIn.open((path + "/" + *it).c_str(), std::ios::binary | std::ios::in);
			fileIn.seekg(0, std::ios::end);
			curConSize = fileIn.tellg();
			fileIn.seekg(0, std::ios::beg);

			if (fileIn.fail())
			{
				log << lgErr << "Failed to open file " << *it << '\n';

				if (permissive)
					continue;

				strm.close();
				remove(outputFile.c_str());
				return false;
			}

			// compress any unknown large file.
			if (curConType == CTUnknown && curConSize > 32768)
				curCompress = true;

			// compress game content and bitmap files.
			// checking extensions instead of detected type because ".wsc" files can resolve to many types,
			// and all image files resolve to "cExImage", but we only need to compress .bmp files.
			if (curFileExt == ".wsc" || curFileExt == ".bmp")
				curCompress = true;
		}

		// add hint table entry.
		sf::Uint8 curNameHint = nameHint(curConName);
		if (hintTable[curNameHint] == 0)
		{
			hintTable[curNameHint] = strm.tell();
		}

		// directories are virtual files and thus contain no data.
		if (false /*curConType == Content::cDirectory*/)
		{
			// write just the header instead.
			strm << curConName << curConType << curCompress << curConSize;
		}
		else
		{
			// read content data.
			std::vector<char> curConBuffer;
			curConBuffer.resize(curConSize);

			fileIn.read(&curConBuffer[0], curConSize);
			fileIn.close();

			// compress (strongly) if necessary.
			if (curCompress && !zu::compress(curConBuffer, 8))
				curCompress = false;	// compression fails? store uncompressed.

			// write content header (with size after possible compression).
			strm << curConName << curConType << curCompress << (sf::Uint32) curConBuffer.size();

			// write content data.
			strm.addData(curConBuffer.data(), curConBuffer.size());

			// document file addition.
			log << lgInfo << "Added " << curConName << " (" << getByteSizeString(curConSize) << " to "
				<< getByteSizeString(curConBuffer.size()) << ")" << '\n';
		}

		// increment content count.
		contentCount++;
	}

	// update content size.
	strm.seek(identSize);
	strm << contentCount;

	strm.seek(tableBegin);
	for (unsigned int i = 0; i < tableSize; ++i)
		strm << hintTable[i];

	strm.close();

	log << lgInfo << "Done! Wrote package file to " << outputFile << '\n';

	return true;
}

bool Package::openFile(const std::string & filename)
{
	close();

	if (!myStream.openInFile(filename))
		return false;

	if (checkHeader())
		return true;
	else
	{
		close();
		return false;
	}
}

void Package::close()
{
	myContentCount = 0;
	myCurrentPosition = 0;
	myCurrentContentId.clear();
	myCurrentContentData.clear();
	myStream.close();
}

bool Package::nextContent()
{
	sf::Uint32 contentSize;

	myStream.seek(myCurrentPosition);
	myStream.skip<std::string>();   // id.
	myStream.skip<sf::Uint8>();	 // type.
	myStream.skip<bool>();		  // compression.
	myStream >> contentSize;

	myStream.seekOff(contentSize);

	myCurrentPosition = myStream.tell();

	return select(myCurrentPosition);
}
bool Package::firstContent()
{
	myCurrentPosition = dataBegin;

	return select(myCurrentPosition);
}

bool Package::select(const std::string & id)
{
	return select(findContent(id));
}
bool Package::select(sf::Uint32 pos)
{
	myCurrentContentId.clear();
	myCurrentContentData.clear();
	myIsDataRead = false;

	myCurrentPosition = pos;

	if (!pos)
		return false;

	bool contentCompression;
	sf::Uint32 contentSize;

	myStream.seek(pos);
	myStream >> myCurrentContentId;
	myStream >> myCurrentContentType;
	myStream >> contentCompression;
	myStream >> contentSize;

	if (!myStream.isValid())
	{
		myCurrentContentId.clear();
		return false;
	}

	return true;
}
bool Package::readData()
{
	if (myIsDataRead)
		return true;

	bool contentCompression;
	sf::Uint32 contentSize;

	myStream.seek(myCurrentPosition);
	myStream.skip<std::string>();	   // name.
	myStream >> myCurrentContentType;
	myStream >> contentCompression;
	myStream >> contentSize;

	myIsDataRead = true;

	if (!myStream.isValid())
	{
		return false;
	}

	myCurrentContentData.resize(contentSize);
	myStream.extractData(&myCurrentContentData[0], contentSize);

	if (contentCompression && !zu::decompress(myCurrentContentData))
	{
		myCurrentContentData.clear();
		return false;
	}

	return true;
}

void Package::deselect()
{
	myCurrentContentId.clear();
	myCurrentContentData.clear();
}

bool Package::hasContent(const std::string & id)
{
	return findContent(id) != 0;
}

std::string Package::getContentId()
{
	if (!myStream.isOpen() || myCurrentContentId.empty())
		return "";

	return myCurrentContentId;
}

const std::vector<char> & Package::getContentData()
{
	if (!myStream.isOpen() || myCurrentContentId.empty())
		return returnEmpty;

	readData();

	return myCurrentContentData;
}

Package::ContentType Package::getContentType()
{
	if (!myStream.isOpen() || myCurrentContentId.empty())
		return CTUnknown;

	return static_cast<ContentType>(myCurrentContentType);
}

unsigned int Package::getContentSize()
{
	if (!myStream.isOpen() || myCurrentContentId.empty())
		return 0;

	readData();

	return myCurrentContentData.size();
}

std::size_t Package::getContentCount()
{
	return myContentCount;
}

sf::Uint32 Package::nameHash(const std::string & id)
{
	std::string hashableId = priv::fileNameToHashable(id);
	return dataHash32(hashableId.data(), hashableId.size());
}
sf::Uint8 Package::nameHint(const std::string & id)
{
	return nameHash(id) % 256;
}

sf::Uint32 Package::findContent(const std::string & id)
{
	if (!myStream.isOpen() || id.empty())
		return 0;

	bool hasExtension = !getFileExtension(id).empty();
	sf::Uint8 hint = nameHint(id);
	sf::Uint32 hintPos = myHintTable[hint];

	if (hintPos == 0)  // no content with this hash hint exists.
		return 0;

	// jump to hinted position.
	myStream.seek(hintPos);

	sf::Uint32 currentPosition;
	std::string currentId;
	sf::Uint8 currentType;
	bool curCompressed;
	sf::Uint32 currentSize;

	while (true)
	{
		currentPosition = myStream.tell();

		// read id from current position.
		myStream >> currentId;

		// ignore content type...
		myStream >> currentType;

		// ignore content compression...
		myStream >> curCompressed;

		// read content size from current position.
		myStream >> currentSize;

		// if input string is extensionless, compare found entry and without extensions.
		if (hasExtension ? (currentId == id) : (priv::fileNameToHashable(currentId) == priv::fileNameToHashable(id)))
		{
			// found you!
			return currentPosition;
		}
		else if (nameHint(currentId) != hint)
		{
			// hash no longer matches, stop searching.
			return 0;
		}
		else
		{
			// advance by content size.
			myStream.seekOff(currentSize);
		}

		// end of stream? do not bother wrapping around, hash will not match anyway.
		if (myStream.endReached() || !myStream.isValid())
		{
			return 0;
		}
	};

	return 0;
}

bool Package::checkHeader()
{
	std::string header(identSize, 0);
	myStream.seek(0);
	myStream.extractData(&header[0], identSize);

	if (header != FileIDent)
		return false;

	myStream >> myContentCount;

	if (!myStream.isValid())
		return false;

	myHintTable.resize(tableSize);

	for (unsigned int i = 0; i < tableSize; ++i)
		myStream >> myHintTable[i];

	return true;
}
