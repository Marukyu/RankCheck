/*
 * VertexBuffer.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: marukyu
 */

#ifndef SRC_CLIENT_GUI3_UTILS_VERTEXBUFFER_HPP_
#define SRC_CLIENT_GUI3_UTILS_VERTEXBUFFER_HPP_

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <Shared/Utils/Error.hpp>
#include <cstddef>
#include <vector>

namespace sf
{
class Transform;
}

namespace gui3
{

class VertexBuffer
{
public:

	template<typename Iterator>
	void replaceSection(std::size_t sectionStart, Iterator vertexStart, Iterator vertexEnd)
	{
		// If vertex buffer is not fully used yet: copy new vertices over old vertices.
		while (vertexStart != vertexEnd)
		{
			if (sectionStart >= myVertices.size())
			{
				throw Error("Invalid vertex section write (end of buffer passed)");
			}

			myVertices[sectionStart] = *vertexStart;
			++sectionStart;
			++vertexStart;
		}
	}

	/**
	 * Fills the section with the specified vertex value.
	 */
	void fillSection(std::size_t sectionStart, std::size_t sectionSize, const sf::Vertex & vertex);

	/**
	 * Replaces the entire vertex buffer with the specified vector.
	 */
	void replace(std::vector<sf::Vertex> && vertices);

	/**
	 * Inserts/erases vertices to change the size of the section at sectionStart from currentSize to newSize.
	 */
	void resizeSection(std::size_t sectionStart, std::size_t currentSize, std::size_t newSize);

	/**
	 * Rotates a section of vertices within the vertex buffer.
	 */
	void rotateSection(std::size_t sectionStart, std::size_t newStart, std::size_t sectionEnd);

	/**
	 * Returns a subset of the buffer's vertices.
	 */
	std::vector<sf::Vertex> getSection(std::size_t sectionStart, std::size_t sectionSize) const;

	/**
	 * Returns all of the buffer's vertices.
	 */
	const std::vector<sf::Vertex> & getVertices() const;

	/**
	 * Returns all of the buffer's vertices.
	 */
	std::size_t getVertexCount() const;

	/**
	 * Clips the vertices in the specified range to a rectangle.
	 * 
	 * Returns the new size of the section after clipping.
	 */
	std::size_t clipSection(std::size_t sectionStart, std::size_t sectionSize, const sf::FloatRect & rect);

	/**
	 * Transforms the vertices in the specified range by a transformation matrix.
	 */
	void transformSection(std::size_t sectionStart, std::size_t sectionSize, const sf::Transform & trans);

private:

	std::vector<sf::Vertex> myVertices;

};

}

#endif
