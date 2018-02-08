/*
 * Renderer.hpp
 *
 *  Created on: Jul 10, 2015
 *      Author: marukyu
 */

#ifndef SRC_CLIENT_GUI3_RENDERER_HPP_
#define SRC_CLIENT_GUI3_RENDERER_HPP_

#include <SFML/System/Vector2.hpp>

namespace gui3
{

class Canvas;

namespace primitives
{
class Gradient;
class Outline;
class TintedBox;
class Box;
}

class Renderer
{
public:

	Renderer();
	virtual ~Renderer();

	virtual sf::Vector2f getWhitePixel() const;

	virtual void addPrimitive(Canvas & canvas, primitives::Gradient gradient) const;
	virtual void addPrimitive(Canvas & canvas, primitives::Outline outline) const;
	virtual void addPrimitive(Canvas & canvas, primitives::Box box) const;
	virtual void addPrimitive(Canvas & canvas, primitives::TintedBox tintedBox) const;
};

}

#endif
