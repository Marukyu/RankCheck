#ifndef SRC_CLIENT_RANKCHECK_PLAYERCARD_HPP_
#define SRC_CLIENT_RANKCHECK_PLAYERCARD_HPP_

#include <Client/GUI3/ResourceManager.hpp>
#include <Client/GUI3/Types.hpp>
#include <Client/RankCheck/PlayerData.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <string>
#include <vector>

namespace sf
{
class Font;
}

namespace gui3
{
class Application;
}

namespace cfg
{
class Config;
}

namespace sf
{
class RenderTarget;
}

class PlayerCard : public sf::Drawable, public sf::Transformable
{
public:

	enum Team
	{
		Red,
		Blue
	};

	enum Type
	{
		Player,
		Sponsor
	};

	PlayerCard();
	virtual ~PlayerCard();

	void show();
	void autoShow();
	void setSubCard(std::shared_ptr<PlayerCard> alt);
	void setFont(const sf::Font & font);
	void setSlot(int slot, bool transition = false);
	int getSlot() const;
	float getInterpolatedSlot() const;
	void setPlayerData(PlayerData data);
	PlayerData getPlayerData() const;
	void initWithConfig(const cfg::Config & config);
	void setApplication(gui3::Application * app);
	void resetTimer();
	float getSecs() const;
	sf::Vector2f getSize() const;
	void setScreenSize(sf::Vector2f screenSize);

	void kill();
	bool done() const;
	bool isAnimating() const;

private:

	void updatePlayer();

	struct LineSetting
	{
		std::string text;
		unsigned int size;
		sf::Color color;
		sf::Vector2f offset;
		std::string condition;
		bool alignRight;
	};

	struct IconSetting
	{
		enum Type
		{
			Unknown,
			Naut,
			MainNaut,
			League,
			Country,
		};
		Type type;
		sf::Vector2f position;
		sf::Vector2f size;
		float borderThickness;
		std::string condition;
	};

	struct DisplaySetting
	{
		float displayTime;
		std::vector<LineSetting> lines;
		std::vector<IconSetting> icons;
		float height;
	};

	struct CurDispMode
	{
		int mode1;
		int mode2;
		float interpolation;
	};

	struct Icon
	{
		sf::Sprite sprite;
		sf::RectangleShape border;
		gui3::Ptr<gui3::res::Image> image;
	};

	void draw(sf::RenderTarget & target, sf::RenderStates states) const override;


	CurDispMode getCurrentDisplayMode() const;

	void addMode();
	void addLine(const std::string & str, unsigned int size, float x, sf::Color color, bool alignRight);
	void addGap(float gap);

	bool isSubCard;
	bool hasData;
	bool shown;
	PlayerData data;
	sf::Clock timer;
	sf::Clock fixTimer;
	sf::Clock slotChangeTimer;
	sf::Time deltaTime;
	int minDisplayMode;

	gui3::Application * app;
	const sf::Font * font;
	mutable std::vector<std::vector<sf::Text>> lines;
	mutable std::vector<std::vector<Icon>> icons;

	float curLineYPos;

	int slot;
	int oldSlot;
	std::vector<DisplaySetting> displaySettings;
	float transitionInTime;
	float transitionOutTime;

	float textResolution;
	float subCardScaleFactor;
	sf::Vector2f screenSize;

	sf::Vector2f cardSize;
	float windowWidth;
	int maxCardCountPerTeam;
	sf::Color fillColorRed;
	sf::Color fillColorBlue;
	sf::Color fillColorSelfRed;
	sf::Color fillColorSelfBlue;
	sf::Color outlineColorRed;
	sf::Color outlineColorBlue;
	sf::Color outlineColorSelfRed;
	sf::Color outlineColorSelfBlue;
	sf::Color outlineColorBorder;
	float timeout;
	bool killed;

	float gradientIntensityTop;
	float gradientIntensityBottom;
	sf::Color gradientColorTop;
	sf::Color gradientColorBottom;

	std::shared_ptr<PlayerCard> subCard;
};

#endif
