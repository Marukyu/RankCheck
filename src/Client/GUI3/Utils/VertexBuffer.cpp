/*
 * VertexBuffer.cpp
 *
 *  Created on: Mar 7, 2015
 *      Author: marukyu
 */

#include <Client/Graphics/UtilitiesSf.hpp>
#include <Client/GUI3/Utils/VertexBuffer.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <Shared/Utils/Error.hpp>
#include <algorithm>
#include <iterator>

namespace gui3
{

void VertexBuffer::fillSection(std::size_t sectionStart, std::size_t sectionSize, const sf::Vertex& vertex)
{
	sf::Vertex * currentVertex = myVertices.data() + sectionStart;

	while (sectionSize-- != 0)
	{
		*currentVertex = vertex;
	}
}

void VertexBuffer::replace(std::vector<sf::Vertex>&& vertices)
{
	myVertices = std::move(vertices);
}

void VertexBuffer::resizeSection(std::size_t sectionStart, std::size_t currentSize, std::size_t newSize)
{
	if (currentSize == newSize)
	{
		// Nothing to do.
		return;
	}

	if (sectionStart + currentSize > myVertices.size())
	{
		// Invalid: current vertex count extends past end of buffer.
		throw Error("Invalid section resize (section start past end of buffer)");
	}

	if (newSize > currentSize)
	{
		// Section size increased (insert vertices at end of section).
		myVertices.insert(myVertices.begin() + sectionStart + currentSize, newSize - currentSize, sf::Vertex());
	}
	else
	{
		// Section size reduced (remove last vertices in section).
		myVertices.erase(myVertices.begin() + sectionStart + newSize, myVertices.begin() + sectionStart + currentSize);
	}
}

void VertexBuffer::rotateSection(std::size_t sectionStart, std::size_t newStart, std::size_t sectionEnd)
{
	if (sectionEnd < sectionStart || newStart < sectionStart || newStart > sectionEnd || sectionEnd > myVertices.size())
	{
		throw Error("Invalid section rotation");
	}

	std::rotate(myVertices.begin() + sectionStart, myVertices.begin() + newStart, myVertices.begin() + sectionEnd);
}

std::vector<sf::Vertex> VertexBuffer::getSection(std::size_t sectionStart, std::size_t sectionSize) const
{
	if (sectionStart + sectionSize > myVertices.size())
	{
		// Invalid: section size past end of buffer.
		throw Error("Invalid section request (section end past end of buffer)");
	}

	return std::vector<sf::Vertex>(myVertices.begin() + sectionStart, myVertices.begin() + sectionStart + sectionSize);
}

const std::vector<sf::Vertex>& VertexBuffer::getVertices() const
{
	return myVertices;
}

std::size_t VertexBuffer::getVertexCount() const
{
	return myVertices.size();
}

std::size_t VertexBuffer::clipSection(std::size_t sectionStart, std::size_t sectionSize, const sf::FloatRect & rect)
{
	if (sectionStart + sectionSize > myVertices.size())
	{
		// Invalid: section size past end of buffer.
		throw Error("Invalid clipping request (section end past end of buffer)");
	}

	std::size_t oldSize = myVertices.size();
	clipVertices(myVertices, sectionStart, sectionStart + sectionSize, rect);
	return sectionSize + myVertices.size() - oldSize;
}

void VertexBuffer::transformSection(std::size_t sectionStart, std::size_t sectionSize, const sf::Transform & trans)
{
	if (sectionStart + sectionSize > myVertices.size())
	{
		// Invalid: section size past end of buffer.
		throw Error("Invalid transformation request (section end past end of buffer)");
	}

	for (std::size_t vertexID = sectionStart; vertexID < sectionStart + sectionSize; ++vertexID)
	{
		myVertices[vertexID].position = trans.transformPoint(myVertices[vertexID].position);
	}
}

}
