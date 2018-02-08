/*
 * Canvas.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: marukyu
 */

#ifndef SRC_CLIENT_GUI3_UTILS_CANVAS_HPP_
#define SRC_CLIENT_GUI3_UTILS_CANVAS_HPP_

#include <Client/GUI3/Renderer.hpp>
#include <Client/GUI3/Utils/VertexBuffer.hpp>
#include <cstddef>

namespace gui3
{

class Canvas
{
public:

	/**
	 * Constructs a canvas from a vertex buffer, a renderer and a theme.
	 */
	Canvas(VertexBuffer & buffer, const Renderer * renderer);

	/**
	 * Draws a primitive to the canvas using the current renderer.
	 */
	template<typename Primitive>
	void draw(Primitive primitive)
	{
		myRenderer->addPrimitive(*this, primitive);
	}

	/**
	 * Draws vertices on the canvas, increasing the current offset by the number of vertices drawn, and inserting new
	 * vertices at the end if the current canvas size is insufficient.
	 */
	template<typename Iterator>
	void draw(Iterator vertexStart, Iterator vertexEnd)
	{
		int addedVertexCount = std::distance(vertexStart, vertexEnd);

		if (addedVertexCount <= 0)
		{
			return;
		}

		if (myVertices.getVertexCount() - myVertexCount < (std::size_t) addedVertexCount)
		{
			myVertices.resizeSection(myVertexCount, myVertices.getVertexCount() - myVertexCount, addedVertexCount);
		}

		myVertices.replaceSection(myVertexCount, vertexStart, vertexEnd);

		myVertexCount += addedVertexCount;
	}

	/**
	 * Skips a number of existing vertices within the canvas buffer.
	 */
	void skip(std::size_t vertexCount);

	/**
	 * Returns the number of vertices in the canvas buffer.
	 */
	std::size_t getVertexCount() const;

	/**
	 * Returns the vertex buffer this canvas uses internally.
	 */
	VertexBuffer & getVertexBuffer() const;

private:

	VertexBuffer & myVertices;
	std::size_t myVertexCount;

	const Renderer * myRenderer;

	friend class Widget;
};

}

#endif
