#include <Client/Graphics/GradientRect.hpp>
#include <Client/Graphics/UtilitiesSf.hpp>
#include <Client/GUI3/Application.hpp>
#include <Client/RankCheck/NautsNames.hpp>
#include <Client/RankCheck/PlayerCard.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
//#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/String.hpp>
//#include <SFML/System/Time.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/MiscMath.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <Shared/Utils/Utilities.hpp>
#include <Shared/Utils/VectorMul.hpp>
#include <algorithm>
//#include <cstddef>

float fadeIn(float secs, float fadeInStart, float fadeInEnd)
{
	return clamp<float>(0.f, (secs - fadeInStart) / (fadeInEnd - fadeInStart), 1.f);
}

float fadeOut(float secs, float fadeOutStart, float fadeOutEnd)
{
	return 1.f - fadeIn(secs, fadeOutStart, fadeOutEnd);
}

float fadeInOut(float secs, float fadeInStart, float fadeInEnd, float fadeOutStart, float fadeOutEnd)
{
	return fadeIn(secs, fadeInStart, fadeInEnd) * fadeOut(secs, fadeOutStart, fadeOutEnd);
}

PlayerCard::PlayerCard()
{
	windowWidth = 1280;
	maxCardCountPerTeam = 3;
	slot = 0;
	oldSlot = 0;
	curLineYPos = 0;
	minDisplayMode = 0;
	textResolution = 1.0;
	transitionInTime = 0.1;
	transitionOutTime = 0.1;
	subCardScaleFactor = 0.8;
	isSubCard = false;
	hasData = false;
	shown = false;
	killed = false;
}

PlayerCard::~PlayerCard()
{
}

void PlayerCard::show()
{
	if (!shown)
	{
		fixTimer.restart();
		shown = true;
		if (subCard)
		{
			subCard->show();
		}
	}
}

void PlayerCard::autoShow()
{
	//if (fixTimer.getElapsedTime() > sf::seconds(0.25))
	{
		show();
	}
}

void PlayerCard::setSubCard(std::shared_ptr<PlayerCard> subCard)
{
	this->subCard = subCard;

	subCard->setSlot(0);
	subCard->isSubCard = true;

	subCard->setPosition(0, 0);
}

void PlayerCard::setFont(const sf::Font& font)
{
	this->font = &font;
	for (auto & page : lines)
	{
		for (auto & line : page)
		{
			line.setFont(font);
		}
	}
	if (subCard)
	{
		subCard->setFont(font);
	}
}

void PlayerCard::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (!shown)
	{
		return;
	}

	states.transform *= getTransform();

	const auto & modeData = getCurrentDisplayMode();

	float scaleFactor = screenSize.y / (cardSize.y * maxCardCountPerTeam);

	if (screenSize.x < 2 * cardSize.x * scaleFactor)
	{
		scaleFactor = screenSize.x / (cardSize.x * 2);
	}

	auto lerpSlot = [=](float slot) -> sf::Vector2f
	{
		auto getSlotUnitPos = [=](int slotNum) -> sf::Vector2f
		{
			int m = slotNum < 0 ? -1 : 1;
			return slotNum == 0 ? sf::Vector2f(.5, 0) : sf::Vector2f((m + 1) / 2, slotNum * m - 1);
		};

		int slotNum = std::floor(slot);
		float lerp = slot - slotNum;
		float transitionLerp = 1.f;
		float dispTime = displaySettings.empty() ? 36000.f : timeout + transitionOutTime;
		float curTimeIn = fixTimer.getElapsedTime().asSeconds();
		float curTimeOut = getSecs();

		if (curTimeIn < transitionInTime)
		{
			transitionLerp *= curTimeIn / transitionInTime;
		}

		if (curTimeOut > dispTime - transitionOutTime)
		{
			transitionLerp *= (dispTime - curTimeOut) / transitionOutTime;
		}

		sf::Vector2f unitPosBasic = (1 - lerp) * getSlotUnitPos(slotNum) + lerp * getSlotUnitPos(slotNum + 1);
		sf::Vector2f unitPosTransition = unitPosBasic;
		if (slotNum < 0)
		{
			unitPosTransition.x -= 1;
		}
		else if (slotNum > 0)
		{
			unitPosTransition.x += 1;
		}
		else
		{
			unitPosTransition.y -= 1;
		}
		sf::Vector2f unitPos = interpolateCosine(unitPosTransition, unitPosBasic, transitionLerp);
		return sf::Vector2f(unitPos.x * (screenSize.x / scaleFactor - cardSize.x), unitPos.y * cardSize.y);
	};

	states.transform.scale(scaleFactor, scaleFactor);
	states.transform.translate(lerpSlot(getInterpolatedSlot()));

	sf::FloatRect rect(0, 0, cardSize.x, cardSize.y);

	auto makeRect = [=](sf::FloatRect r, sf::Color fill, sf::Color outline, float outlineSize)
	{
		GradientRect shape;
		shape.setPosition(r.left, r.top);
		shape.setSize(sf::Vector2f(r.width, r.height));
		shape.setDirection(GradientRect::Vertical);
		if (fill == sf::Color::Transparent)
		{
			shape.setColors(fill, fill);
		}
		else
		{
			shape.setColorA(interpolateLinear(fill, gradientColorTop, gradientIntensityTop));
			shape.setColorB(interpolateLinear(fill, gradientColorBottom, gradientIntensityBottom));
		}
		shape.setOutlineColor(outline);
		shape.setOutlineThickness(outlineSize);
		return shape;
	};

	sf::Color bgColor;
	sf::Color outColor;
	sf::Color borColor = outlineColorBorder;
	float bf = 1.f / getScale().x;

	auto lerpTeamColor = [](sf::Color red, sf::Color blue, PlayerData::Team team) -> sf::Color
	{
		float lerp = (team == PlayerData::Red ? 0 : (team == PlayerData::Blue ? 1 : 0.5));
		return lerp * blue + (1 - lerp) * red;
	};

	if (data.isLocal)
	{
		bgColor = lerpTeamColor(fillColorSelfRed, fillColorSelfBlue, data.team);
		outColor = lerpTeamColor(outlineColorSelfRed, outlineColorSelfBlue, data.team);
	}
	else
	{
		bgColor = lerpTeamColor(fillColorRed, fillColorBlue, data.team);
		outColor = lerpTeamColor(outlineColorRed, outlineColorBlue, data.team);
	}

	GradientRect background = makeRect(expandRect(rect, -6.f * bf), bgColor, sf::Color::Transparent, 0.f * bf);
	GradientRect border1 = makeRect(expandRect(rect, -6.f * bf), sf::Color::Transparent, borColor, 6.f * bf);
	GradientRect border2 = makeRect(expandRect(rect, -4.f * bf), sf::Color::Transparent, outColor, 2.f * bf);

	target.draw(background, states);

	auto drawModeParts = [&](int num)
	{
		const auto & modeID = (num == 1 ? modeData.mode1 : modeData.mode2);
		float lerp = (num == 1 ? 1.f - modeData.interpolation : modeData.interpolation);

		if (modeID >= 0 && modeID < (int)lines.size())
		{
			for (auto & line : lines[modeID])
			{
				sf::Color color = line.getFillColor();
				color.a = lerp * 255;
				line.setFillColor(color);
				target.draw(line, states);
			}
			for (auto & icon : icons[modeID])
			{
				sf::Color bcolor = icon.border.getOutlineColor();
				bcolor.a = lerp * 255;
				icon.border.setOutlineColor(bcolor);
				target.draw(icon.border, states);

				sf::Color color = icon.sprite.getColor();
				color.a = lerp * 255;
				icon.sprite.setColor(color);
				target.draw(icon.sprite, states);
			}
		}
	};

	drawModeParts(1);
	//drawModeParts(2);

	target.draw(border1, states);
	target.draw(border2, states);
}

void PlayerCard::setSlot(int slot, bool transition)
{
	if (transition)
	{
		slotChangeTimer.restart();
		this->oldSlot = this->slot;
		this->slot = slot;
	}
	else
	{
		this->oldSlot = slot;
		this->slot = slot;
	}
}

int PlayerCard::getSlot() const
{
	return slot;
}

float PlayerCard::getInterpolatedSlot() const
{
	float lerp = std::min(1.f, slotChangeTimer.getElapsedTime().asSeconds() / transitionInTime);
	float lerpedSlot = interpolateCosine<float>(oldSlot, slot, lerp);
	return lerpedSlot;
}

void PlayerCard::setPlayerData(PlayerData data)
{
	this->data = data;
	this->hasData = true;
	updatePlayer();
}

PlayerData PlayerCard::getPlayerData() const
{
	return data;
}

#define WOS_LOCAL_CFG_ENTRY(Type, Name) static cfg::Type c_##Name("rankcheck.playerPopups." #Name); \
                                        this->Name = config.get(c_##Name);

void PlayerCard::initWithConfig(const cfg::Config& config)
{
	WOS_LOCAL_CFG_ENTRY(Vector2f, cardSize);
	WOS_LOCAL_CFG_ENTRY(Int, maxCardCountPerTeam);
	WOS_LOCAL_CFG_ENTRY(Float, transitionInTime);
	WOS_LOCAL_CFG_ENTRY(Float, textResolution);
	WOS_LOCAL_CFG_ENTRY(Float, transitionOutTime);
	WOS_LOCAL_CFG_ENTRY(Float, timeout);
	WOS_LOCAL_CFG_ENTRY(Color, fillColorRed);
	WOS_LOCAL_CFG_ENTRY(Color, fillColorBlue);
	WOS_LOCAL_CFG_ENTRY(Color, fillColorSelfRed);
	WOS_LOCAL_CFG_ENTRY(Color, fillColorSelfBlue);
	WOS_LOCAL_CFG_ENTRY(Color, outlineColorRed);
	WOS_LOCAL_CFG_ENTRY(Color, outlineColorBlue);
	WOS_LOCAL_CFG_ENTRY(Color, outlineColorSelfRed);
	WOS_LOCAL_CFG_ENTRY(Color, outlineColorSelfBlue);
	WOS_LOCAL_CFG_ENTRY(Color, outlineColorBorder);
	WOS_LOCAL_CFG_ENTRY(Float, gradientIntensityTop);
	WOS_LOCAL_CFG_ENTRY(Float, gradientIntensityBottom);
	WOS_LOCAL_CFG_ENTRY(Color, gradientColorTop);
	WOS_LOCAL_CFG_ENTRY(Color, gradientColorBottom);

	if (timeout < 1.f)
	{
		timeout = 36000;
	}

	static cfg::Int displayModeCount("rankcheck.playerPopups.displayModes.length");

	displaySettings.clear();

	for (int i = 0; i < config.get(displayModeCount); ++i)
	{
		DisplaySetting displayMode;
		std::string displayModeKey = "rankcheck.playerPopups.displayModes[" + cNtoS(i) + "]";

		displayMode.displayTime = 0;
		displayMode.height = 0;

		for (int j = 0; j < config.get(cfg::Int(displayModeKey + ".lines.length")); ++j)
		{
			LineSetting line;
			std::string lineKey = displayModeKey + ".lines[" + cNtoS(j) + "]";

			line.text = config.get(cfg::String(lineKey + ".text"));
			line.size = config.get(cfg::Int(lineKey + ".size"));
			line.color = config.get(cfg::Color(lineKey + ".color"));
			line.offset = config.get(cfg::Vector2f(lineKey + ".offset"));
			line.condition = config.get(cfg::String(lineKey + ".condition"));
			line.alignRight = config.get(cfg::Bool(lineKey + ".alignRight"));

			displayMode.lines.push_back(line);
		}

		for (int j = 0; j < config.get(cfg::Int(displayModeKey + ".icons.length")); ++j)
		{
			IconSetting icon;
			std::string iconKey = displayModeKey + ".icons[" + cNtoS(j) + "]";

			std::string iconType = toLowercase(config.get(cfg::String(iconKey + ".type")));

			if      (iconType == "league")   icon.type = IconSetting::League;
			else if (iconType == "naut")     icon.type = IconSetting::Naut;
			else if (iconType == "mainnaut") icon.type = IconSetting::MainNaut;
			else if (iconType == "country")  icon.type = IconSetting::Country;
			else                             icon.type = IconSetting::Unknown;

			icon.position = config.get(cfg::Vector2f(iconKey + ".position"));
			icon.size = config.get(cfg::Vector2f(iconKey + ".size"));
			icon.borderThickness = config.get(cfg::Float(iconKey + ".borderThickness"));
			icon.condition = config.get(cfg::String(iconKey + ".condition"));

			displayMode.icons.push_back(icon);
		}

		displaySettings.push_back(displayMode);
	}

	if (subCard)
	{
		subCard->initWithConfig(config);
	}

	updatePlayer();
}

void PlayerCard::setApplication(gui3::Application * app)
{
	this->app = app;

	if (subCard)
	{
		subCard->setApplication(app);
	}
}

void PlayerCard::resetTimer()
{
	if (killed)
	{
		return;
	}

	if (subCard)
	{
		subCard->resetTimer();
	}

	minDisplayMode = getCurrentDisplayMode().mode2;
	deltaTime -= sf::seconds(getSecs());
}

sf::Vector2f PlayerCard::getSize() const
{
	return cardSize;
}

float PlayerCard::getSecs() const
{
	return (timer.getElapsedTime() + deltaTime).asSeconds();
}

bool PlayerCard::done() const
{
	if (subCard && !subCard->done())
	{
		return false;
	}

	float maxTime = transitionOutTime + timeout;
	return getSecs() >= maxTime;
}

inline sf::FloatRect scaleToRect(sf::Vector2f inner, sf::FloatRect outer)
{
	sf::Vector2f iSize(outer.width, outer.height);
	sf::Vector2f wSize(inner);

	if (iSize.x == 0.f || iSize.y == 0.f || wSize.x == 0.f || wSize.y == 0.f)
		return sf::FloatRect();

	float iRatio = iSize.x / iSize.y;
	float wRatio = wSize.x / wSize.y;

	if (iRatio > wRatio)
	{
		// Inner is wider than outer.
		wSize = sf::Vector2f(iSize.y * wRatio, iSize.y);
	}
	else
	{
		// Inner is higher than outer.
		wSize = sf::Vector2f(iSize.x, iSize.x / wRatio);
	}

	sf::Vector2f wPos((iSize - wSize) / 2.f);
	return sf::FloatRect(outer.left + wPos.x, outer.top + wPos.y, wSize.x, wSize.y);
}

void PlayerCard::updatePlayer()
{
	if (!hasData)
	{
		return;
	}

	lines.clear();
	icons.clear();
	for (DisplaySetting & mode : displaySettings)
	{
		addMode();
		for (const LineSetting & line : mode.lines)
		{
			if (data.evaluate(line.condition))
			{
				addGap(line.offset.y);
				addLine(data.format(line.text), line.size, line.offset.x, line.color, line.alignRight);
			}
		}

		mode.height = curLineYPos + 25;

		std::vector<Icon> iconList;
		for (const IconSetting & iconSetting : mode.icons)
		{
			if (!data.evaluate(iconSetting.condition))
			{
				continue;
			}

			Icon icon;
			std::string iconName = "Missing";
			switch (iconSetting.type)
			{
			case IconSetting::League:
				iconName = "UI_League" + cNtoS(data.league);
				break;

			case IconSetting::Naut:
				iconName = "Classicon_" + NautsNames::getInstance().getNautClassName(data.currentNaut);
				break;

			case IconSetting::MainNaut:
				iconName = "Classicon_" + NautsNames::getInstance().getNautClassName(data.mainNaut);
				break;

			case IconSetting::Country:
				iconName = "flags/" + (data.countryCode.empty() ? "XX" : data.countryCode);
				break;

			default:
				break;
			}

			icon.image = app ? app->getResourceManager().acquireImage("rankcheck/" + iconName + ".png") : nullptr;
			if (icon.image != nullptr)
			{
				icon.sprite.setTexture(*app->getTexture(icon.image->getTexturePage()));
				icon.sprite.setTextureRect(sf::IntRect(icon.image->getTextureRect()));
				sf::Vector2f imgSize(icon.image->getTextureRect().width, icon.image->getTextureRect().height);
				auto rect = scaleToRect(imgSize, sf::FloatRect(iconSetting.position, iconSetting.size));
				icon.sprite.setPosition(sf::Vector2f(10.f + rect.left, 10.f + rect.top));
				icon.sprite.setScale(sf::Vector2f(rect.width, rect.height) / imgSize);

				if (iconSetting.borderThickness >= 0.001f)
				{
					sf::Color color = (data.team == PlayerData::Red ? fillColorRed : fillColorBlue);
					sf::Color xcolor = sf::Color::Black;
					color.r = interpolateLinear(color.r, xcolor.r, 0.5);
					color.g = interpolateLinear(color.g, xcolor.g, 0.5);
					color.b = interpolateLinear(color.b, xcolor.b, 0.5);
					icon.border.setOutlineColor(color);
					icon.border.setFillColor(sf::Color::Transparent);
					icon.border.setOutlineThickness(iconSetting.borderThickness);
					icon.border.setPosition(icon.sprite.getPosition());
					icon.border.setSize(sf::Vector2f(rect.width, rect.height));
				}
			}

			iconList.push_back(icon);
		}
		icons.push_back(iconList);
	}
}

PlayerCard::CurDispMode PlayerCard::getCurrentDisplayMode() const
{
	return {0, 0, 0.f};
}

void PlayerCard::addMode()
{
	lines.emplace_back();
	curLineYPos = 0;
}

void PlayerCard::addLine(const std::string & str, unsigned int size, float x, sf::Color color, bool alignRight)
{
	float fullWidth = getSize().x;

	if (font && !lines.empty())
	{
		sf::Text line;
		line.setPosition(alignRight ? (fullWidth - 10) - x : x + 10, curLineYPos + 10);
		line.setCharacterSize(size * textResolution);
		line.setFont(*font);
		line.setString(sf::String::fromUtf8(str.begin(), str.end()));
		line.setFillColor(color);
		if (alignRight)
		{
			line.setOrigin(line.getLocalBounds().width, 0.f);
		}
		float widthLimit = ((fullWidth - 10) - line.getPosition().x * (alignRight ? -1.f : 1.f)) * textResolution;
		float hScale = 1.f;
		if (line.getLocalBounds().width > widthLimit)
		{
			hScale = widthLimit / line.getLocalBounds().width;
		}
		line.setScale(hScale / textResolution, 1.f / textResolution);
		lines.back().push_back(line);
		addGap(size);
	}
}

void PlayerCard::setScreenSize(sf::Vector2f screenSize)
{
	this->screenSize = screenSize;
}

void PlayerCard::kill()
{
	timer.restart();
	float dispTime = displaySettings.empty() ? 36000.f : timeout + transitionOutTime;
	deltaTime = sf::seconds(dispTime - transitionOutTime);
	killed = true;
}

void PlayerCard::addGap(float gap)
{
	curLineYPos += gap;
}

bool PlayerCard::isAnimating() const
{
	if (fixTimer.getElapsedTime().asSeconds() < transitionInTime)
	{
		return true;
	}

	float dispTime = displaySettings.empty() ? 36000.f : timeout + transitionOutTime;
	if (getSecs() > dispTime - transitionOutTime)
	{
		return true;
	}

	if (slot != oldSlot && slotChangeTimer.getElapsedTime().asSeconds() < transitionInTime)
	{
		return true;
	}

	return false;
}
