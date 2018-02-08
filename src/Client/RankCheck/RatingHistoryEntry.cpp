#include <Client/Graphics/GradientRect.hpp>
#include <Client/RankCheck/RatingHistoryEntry.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Time.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/MiscMath.hpp>
#include <Shared/Utils/Utilities.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

RatingHistoryEntry::RatingHistoryEntry()
{
}

RatingHistoryEntry::~RatingHistoryEntry()
{
}

void RatingHistoryEntry::initWithConfig(const cfg::Config& config)
{
	static cfg::Vector2f cfgDiffSize("rankcheck.ratingHistory.size");
	static cfg::Vector2f cfgScoreSize("rankcheck.ratingHistory.scoreSize");
	static cfg::Float cfgTextSize("rankcheck.ratingHistory.textSize");
	static cfg::Float cfgTextResolution("rankcheck.ratingHistory.textResolution");
	static cfg::Float cfgAnimTime("rankcheck.ratingHistory.animationTime");

	diffSize = config.get(cfgDiffSize);
	scoreSize = config.get(cfgScoreSize);
	animTime = config.get(cfgAnimTime);

	std::string styleName;

	if (isTotal())
	{
		styleName = "total";
	}
	else if (getValue() > 0)
	{
		styleName = "positive";
	}
	else if (getValue() < 0)
	{
		styleName = "negative";
	}
	else
	{
		styleName = "neutral";
	}

	style.textColor = config.get(cfg::Color("rankcheck.ratingHistory.colors." + styleName + ".textColor"));
	style.fillColor = config.get(cfg::Color("rankcheck.ratingHistory.colors." + styleName + ".fillColor"));
	style.outlineColor = config.get(cfg::Color("rankcheck.ratingHistory.colors." + styleName + ".outlineColor"));
	style.borderColor = config.get(cfg::Color("rankcheck.ratingHistory.colors." + styleName + ".borderColor"));

	auto textRes = config.get(cfgTextResolution);
	auto textSize = config.get(cfgTextSize);

	text.setFillColor(style.textColor);
	text.setCharacterSize(textSize * textRes);
	text.setScale(1 / textRes, 1 / textRes);
	update();
}

void RatingHistoryEntry::setScreenSize(sf::Vector2f screenSize)
{
	this->screenSize = screenSize;
}

bool RatingHistoryEntry::isAnimating() const
{
	return animClock.getElapsedTime().asSeconds() < animTime;
}

void RatingHistoryEntry::setTotal(bool total)
{
	this->total = total;
}

bool RatingHistoryEntry::isTotal() const
{
	return total;
}

void RatingHistoryEntry::setSlot(int slot)
{
	this->prevSlot = this->slot;
	this->slot = slot;
	animClock.restart();
}

int RatingHistoryEntry::getSlot() const
{
	return slot;
}

void RatingHistoryEntry::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	float scaleFactor = screenSize.y / diffSize.y;

	float lerpSlot = animTime == 0 ? slot : interpolateCosine<float>(prevSlot, slot,
		std::min(1.f, animClock.getElapsedTime().asSeconds() / animTime));

	sf::FloatRect rect(
		(-scoreSize.x - lerpSlot * diffSize.x) * scaleFactor,
		-(total ? scoreSize.y : diffSize.y) * scaleFactor,
		(total ? scoreSize.x : diffSize.x) * scaleFactor,
		(total ? scoreSize.y : diffSize.y) * scaleFactor
	);

	rect = sf::FloatRect(sf::IntRect(rect));

	auto makeRect = [=](sf::FloatRect r, sf::Color fill, sf::Color outline, float outlineSize)
	{
		GradientRect shape;
		shape.setPosition(r.left, r.top);
		shape.setSize(sf::Vector2f(r.width, r.height));
		shape.setDirection(GradientRect::Vertical);
		shape.setColors(fill, fill);
		shape.setOutlineColor(outline);
		shape.setOutlineThickness(outlineSize);
		return shape;
	};

	int bf = 1;

	GradientRect background = makeRect(expandRect(rect, -3.f * bf), style.fillColor, sf::Color::Transparent, 0.f * bf);
	GradientRect border1 = makeRect(expandRect(rect, -3.f * bf), sf::Color::Transparent, style.borderColor, 3.f * bf);
	GradientRect border2 = makeRect(expandRect(rect, -2.f * bf), sf::Color::Transparent, style.outlineColor, 1.f * bf);

	target.draw(background, states);
	target.draw(border1, states);
	target.draw(border2, states);

	sf::RenderStates textStates = states;
	textStates.transform.translate(
		rect.left + (rect.width - text.getGlobalBounds().width * scaleFactor) / 2.f,
		rect.top + (rect.height - text.getGlobalBounds().height * scaleFactor * 1.5) / 2.f);
	textStates.transform.scale(scaleFactor, scaleFactor);
	target.draw(text, textStates);
}

void RatingHistoryEntry::setValue(int value)
{
	this->value = value;
	update();
}

int RatingHistoryEntry::getValue() const
{
	return value;
}

void RatingHistoryEntry::setFont(const sf::Font& font)
{
	text.setFont(font);
}

void RatingHistoryEntry::setSlotInstant(int slot)
{
	setSlot(slot);
	setSlot(slot);
}

void RatingHistoryEntry::update()
{
	std::ostringstream stream;
	stream << std::setprecision(std::abs(value) >= 1000 ? 0 : 1) << std::fixed << std::abs(value / 10.0);

	std::string label = stream.str();

	if (!isTotal())
	{
		if (getValue() > 0)
		{
			label = "+" + label;
		}
		else if (getValue() < 0)
		{
			label = "-" + label;
		}
		else
		{
			label = "0";
		}
	}
	text.setString(label);
}
