#ifndef SRC_CLIENT_RANKCHECK_RATINGHISTORYENTRY_HPP_
#define SRC_CLIENT_RANKCHECK_RATINGHISTORYENTRY_HPP_

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

namespace sf
{
class Font;
}

namespace sf
{
class RenderTarget;
}

namespace cfg
{
class Config;
}

class RatingHistoryEntry : public sf::Drawable, public sf::Transformable
{
public:
	RatingHistoryEntry();
	virtual ~RatingHistoryEntry();

	void initWithConfig(const cfg::Config & config);

	void setScreenSize(sf::Vector2f screenSize);
	void setFont(const sf::Font & font);

	bool isAnimating() const;

	void setValue(int value);
	int getValue() const;

	void setTotal(bool total);
	bool isTotal() const;

	void setSlot(int slot);
	void setSlotInstant(int slot);
	int getSlot() const;

private:

	struct Style
	{
		sf::Color textColor;
		sf::Color fillColor;
		sf::Color outlineColor;
		sf::Color borderColor;
	};

	void draw(sf::RenderTarget & target, sf::RenderStates states) const override;
	void update();

	Style style;
	sf::Vector2f screenSize;
	sf::Vector2f diffSize;
	sf::Vector2f scoreSize;
	int slot = 0;
	int prevSlot = 0;
	int value = 0;
	bool total = false;
	sf::Text text;
	sf::Clock animClock;
	float animTime = 0.5f;
};

#endif
