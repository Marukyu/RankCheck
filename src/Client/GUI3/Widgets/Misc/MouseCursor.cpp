#include <Client/GUI3/Application.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/ResourceManager.hpp>
#include <Client/GUI3/Types.hpp>
#include <Client/GUI3/Utils/Canvas.hpp>
#include <Client/GUI3/Widgets/Misc/MouseCursor.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

namespace gui3
{


MouseCursor::MouseCursor()
{
	setZPosition(1000);

	addStateCallback([=](StateEvent event)
	{
		handleStateEvent(event);
	}, gui3::StateEvent::Any);

	addTickCallback([=]()
	{
		determineCursorImage();
	});
}

MouseCursor::~MouseCursor()
{
}

void MouseCursor::determineCursorImage()
{
	Ptr<res::Image> image;
	Widget * widget = getParent();

	while (widget != nullptr)
	{
		Ptr<res::Image> widgetImage = widget->getMouseCursorOverride();
		if (widgetImage != nullptr)
		{
			image = widgetImage;
		}

		Container * container = dynamic_cast<Container *>(widget);
		if (container != nullptr)
		{
			widget = container->getMouseOverWidget();
		}
		else
		{
			break;
		}
	}

	setImage(image);
}

void MouseCursor::setImage(Ptr<res::Image> image)
{
	if (myImage != image)
	{
		myImage = image;
		updateImage();
	}
}

Ptr<res::Image> MouseCursor::getImage() const
{
	return myImage;
}

sf::FloatRect MouseCursor::getCursorRect() const
{
	if (myImage == nullptr)
	{
		return sf::FloatRect();
	}
	else
	{
		sf::FloatRect texRect = myImage->getTextureRect();
		return sf::FloatRect(myMousePosition.x, myMousePosition.y, texRect.width, texRect.height);
	}
}

void MouseCursor::onRepaint(Canvas& canvas)
{
	if (myImage)
	{
		sf::FloatRect vrect = getCursorRect();
		sf::FloatRect trect = myImage->getTextureRect();
		sf::Color white = sf::Color::White;

		std::vector<sf::Vertex> vertices;
		vertices.emplace_back(sf::Vector2f(vrect.left, vrect.top), white, sf::Vector2f(trect.left, trect.top));
		vertices.emplace_back(sf::Vector2f(vrect.left + vrect.width, vrect.top), white,
			sf::Vector2f(trect.left + trect.width, trect.top));
		vertices.emplace_back(sf::Vector2f(vrect.left + vrect.width, vrect.top + vrect.height), white,
			sf::Vector2f(trect.left + trect.width, trect.top + trect.height));
		vertices.push_back(vertices[0]);
		vertices.push_back(vertices[2]);
		vertices.emplace_back(sf::Vector2f(vrect.left, vrect.top + vrect.height), white,
			sf::Vector2f(trect.left, trect.top + trect.height));
		canvas.draw(vertices.begin(), vertices.end());
	}
}

void MouseCursor::onRender(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (myImage)
	{
		sf::Sprite spr;
		spr.setTexture(*getParentApplication()->getTexture(myImage->getTexturePage()));
		spr.setTextureRect(sf::IntRect(myImage->getTextureRect()));
		sf::FloatRect rect = getCursorRect();
		spr.setPosition(rect.left, rect.top);
		spr.setScale(rect.width / myImage->getTextureRect().width, rect.height / myImage->getTextureRect().height);
		target.draw(spr, states);
	}
}

void MouseCursor::handleStateEvent(StateEvent event)
{
	switch (event.type)
	{
	case StateEvent::ResourcesChanged:
		updateImage();
		break;
	case StateEvent::ParentChanged:
		if (getParent() == nullptr)
		{
			myParentMouseCallback.remove();
		}
		else
		{
			myParentMouseCallback = getParent()->addMouseCallback([=](MouseEvent event)
			{
				handleParentMouseMove(event);
			}, MouseEvent::Move);
		}
		break;
	default:
		break;
	}
}

void MouseCursor::handleParentMouseMove(MouseEvent event)
{
	if (myMousePosition != event.position)
	{
		myMousePosition = event.position;
		repaint();
	}
}

void MouseCursor::updateImage()
{
	setComplexOverride(myImage != nullptr && !myImage->isOnMainTexture());
	repaint();
}

}
