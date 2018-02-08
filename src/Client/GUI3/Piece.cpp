#include <Client/GUI3/Application.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Piece.hpp>
#include <Client/GUI3/Utils/Canvas.hpp>
#include <Client/GUI3/Utils/NullResourceManager.hpp>
#include <Client/GUI3/Utils/VertexBuffer.hpp>
#include <Client/GUI3/Widget.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <algorithm>
#include <vector>

namespace gui3
{

Piece::Piece() :
	myWidget(nullptr),
	myIsComplex(false),
	myHasResizeFunction(false)
{
}

Piece::~Piece()
{
}

bool Piece::isComplex() const
{
	return myIsComplex;
}

void Piece::paint(Canvas& canvas)
{
}

void Piece::render(sf::RenderTarget& target, sf::RenderStates states)
{
	if (myVertexBuffer == nullptr)
	{
		myVertexBuffer = makeUnique<VertexBuffer>();
	}

	Canvas canvas(*myVertexBuffer, getWidget()->getRenderer());
	paint(canvas);
	myVertexBuffer->resizeSection(0, myVertexBuffer->getVertexCount(), canvas.getVertexCount());
	target.draw(myVertexBuffer->getVertices().data(), myVertexBuffer->getVertices().size(), sf::Triangles, states);
}

void Piece::setPosition(sf::Vector2f position)
{
	setPosition(position.x, position.y);
}

void Piece::setPosition(float x, float y)
{
	myRect.left = x;
	myRect.top = y;
	onMove();
}

sf::Vector2f Piece::getPosition() const
{
	return
	{	myRect.left, myRect.top};
}

void Piece::setSize(sf::Vector2f size)
{
	setSize(size.x, size.y);
}

void Piece::setSize(float width, float height)
{
	myRect.width = width;
	myRect.height = height;
	onResize();
}

sf::Vector2f Piece::getSize() const
{
	return
	{	myRect.width, myRect.height};
}

void Piece::setRect(sf::FloatRect rect)
{
	setRect(rect.left, rect.top, rect.width, rect.height);
}

void Piece::setRect(float left, float top, float width, float height)
{
	setPosition(left, top);
	setSize(width, height);
}

sf::FloatRect Piece::getRect() const
{
	return myRect;
}

void Piece::setAutoResizeFunction()
{
	setResizeFunction(generateResizeFunction());
}

void Piece::setResizeFunction(ResizeFunc function)
{
	myResizeFunction = std::move(function);
	myHasResizeFunction = true;
	updateSize();
}

void Piece::clearResizeFunction()
{
	myHasResizeFunction = false;
}

void Piece::updateSize()
{
	if (myHasResizeFunction && getWidget() != nullptr)
	{
		setRect(myResizeFunction(getWidget()->getSize()));
	}
}

Piece::ResizeFunc Piece::generateResizeFunction()
{
	return generateResizeFunction(0, 0, 0, 0);
}

Piece::ResizeFunc Piece::generateResizeFunction(float horizontalMargin, float verticalMargin)
{
	return generateResizeFunction(horizontalMargin, verticalMargin, horizontalMargin, verticalMargin);
}

Piece::ResizeFunc Piece::generateResizeFunction(float leftMargin, float topMargin, float rightMargin,
	float bottomMargin)
{
	return [=](sf::Vector2f size)
	{
		return sf::FloatRect(leftMargin, topMargin, size.x - rightMargin - leftMargin, size.y - bottomMargin - topMargin);
	};
}

void Piece::onInit()
{
}

void Piece::onMove()
{
	repaint();
}

void Piece::onResize()
{
	repaint();
}

void Piece::setComplex(bool complex)
{
	if (myIsComplex != complex)
	{
		myIsComplex = complex;

		if (getWidget() != nullptr)
		{
			getWidget()->updateComplexity();
			repaint();
		}
	}
}

void Piece::repaint()
{
	if (getWidget() != nullptr)
	{
		getWidget()->repaint();
	}
}

cfg::Config & Piece::config() const
{
	if (getWidget() == nullptr || getWidget()->getParentApplication() == nullptr)
	{
		static cfg::Config nullConfig;
		return nullConfig;
	}
	return getWidget()->getParentApplication()->getConfig();
}

res::ResourceManager & Piece::resources() const
{
	if (getWidget() == nullptr || getWidget()->getParentApplication() == nullptr)
	{
		static res::NullResourceManager nullResourceManager;
		return nullResourceManager;
	}
	return getWidget()->getParentApplication()->getResourceManager();
}

Widget* Piece::getWidget() const
{
	return myWidget;
}

void Piece::doAutoResize()
{
	updateSize();
}

void Piece::setWidget(Widget& widget)
{
	myWidget = &widget;

	getWidget()->addStateCallback([this](StateEvent e)
	{
		doAutoResize();
	}, StateEvent::Resized);
	updateSize();

	onInit();

	getWidget()->updateComplexity();
}

}
