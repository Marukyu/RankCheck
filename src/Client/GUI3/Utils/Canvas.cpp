/*
 * Canvas.cpp
 *
 *  Created on: Mar 7, 2015
 *      Author: marukyu
 */

#include <Client/GUI3/Utils/Canvas.hpp>
#include <Shared/Utils/Error.hpp>

namespace gui3
{

Canvas::Canvas(VertexBuffer & buffer, const Renderer * renderer) :
	myVertices(buffer),
	myVertexCount(0),
	myRenderer(renderer)
{
	if (renderer == nullptr)
	{
		throw Error("Failed to create canvas: renderer is null");
	}
}

void Canvas::skip(std::size_t vertexCount)
{
	myVertexCount += vertexCount;

	if (myVertices.getVertexCount() < myVertexCount)
	{
		myVertices.resizeSection(0, myVertices.getVertexCount(), myVertexCount);
	}
}

std::size_t Canvas::getVertexCount() const
{
	return myVertexCount;
}

VertexBuffer & Canvas::getVertexBuffer() const
{
	return myVertices;
}

}
