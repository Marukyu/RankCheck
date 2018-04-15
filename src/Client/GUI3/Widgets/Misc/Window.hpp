#ifndef SRC_CLIENT_GUI3_WIDGETS_MISC_WINDOW_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_MISC_WINDOW_HPP_

#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Types.hpp>
#include <Client/GUI3/Widgets/Controls/Button.hpp>
#include <Client/GUI3/Widgets/Panels/Panel.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>

namespace gui3
{

class Window : public Container
{
public:

	Window();
	virtual ~Window();

	void add(Ptr<Widget> widget);
	void remove(Widget & widget);

	void setContentSize(float contentWidth, float contentHeight);
	void setContentSize(sf::Vector2f contentSize);
	sf::Vector2f getContentSize() const;

	void setMaximized(bool maximized);
	bool isMaximized() const;

	void setTitle(std::string title);
	std::string getTitle() const;

	void setResizable(bool resizable);
	bool isResizable() const;

	void setMovable(bool movable);
	bool isMovable() const;

	void setMaximizable(bool maximizable);
	bool isMaximizable() const;

	void setClosable(bool closable);
	bool isClosable() const;

	void setBorderless(bool borderless);
	bool isBorderless() const;

	void setProportional(bool proportional);
	bool isProportional() const;

	void setAspectRatio(float ratio);
	float getAspectRatio() const;

	void setMaximumSize(sf::Vector2f maxSize);
	sf::Vector2f getMaximumSize() const;

	void setMinimumSize(sf::Vector2f minSize);
	sf::Vector2f getMinimumSize() const;

private:

	class Titlebar : public Widget
	{
	public:

		Titlebar(Window * window);
		virtual ~Titlebar();

		void setTitle(std::string title);
		std::string getTitle() const;

	private:

		virtual void onRepaint(Canvas & canvas) override;

		void handleWindowStateEvent(StateEvent event);
		void handleMouseEvent(MouseEvent event);
		void handleParentMouseEvent(MouseEvent event);

		void updateWindowParent();

		Window * myWindow;

		Callback<MouseEvent> myParentMouseCallback;
		bool myIsDragging;
		sf::Vector2f myDragOffset;

		pieces::Text * myText;
	};

	class Background : public Widget
	{
	public:

		Background(Window * window);
		virtual ~Background();

	private:

		virtual void onRepaint(Canvas & canvas) override;
	};

	class ResizeOverlay : public Widget
	{
	public:

		ResizeOverlay(Window * window);
		virtual ~ResizeOverlay();

		bool isResizing() const;
		virtual bool testMouseOver(sf::Vector2f pos) const override;

	private:

		void updateResizeMode(sf::Vector2f mousePos);

		float getResizeMargin() const;
		sf::Vector2f getScaleFactor() const;
		sf::Vector2f getShiftFactor() const;

		void handleStateEvent(StateEvent event);
		void handleMouseEvent(MouseEvent event);

		enum class ResizeMode
		{
			Negative = -1,
			None = 0,
			Positive = 1,
		};

		Window * myWindow;

		bool myIsResizing;
		bool myRecursionLock;
		ResizeMode myResizeModeX;
		ResizeMode myResizeModeY;

		sf::Vector2f myDragOrigin;
		sf::Vector2f myInitialWindowPosition;
		sf::Vector2f myInitialWindowSize;

		Ptr<res::Image> myResizeCursor;
	};

	void handleStateEvent(StateEvent event);

	void updateContentSize();
	void updateButtonSizes();
	void updateTitleBarPlacement();
	void updateMaximizedSize();

	void restrictWindowPosition();

	void setTitleBarDown(bool down);
	bool isTitleBarDown() const;

	bool getTitleBarDownCondition() const;

	sf::Vector2f convertFrameSizeToContentSize(sf::Vector2f frameSize) const;
	sf::Vector2f convertContentSizeToFrameSize(sf::Vector2f contentSize) const;

	enum class ProportionMode
	{
		AxisX,
		AxisY,
		Diagonal
	};

	sf::Vector2f applyProportionsToContentSize(sf::Vector2f contentSize, ProportionMode mode) const;
	sf::Vector2f applySizeRestrictionsToContentSize(sf::Vector2f contentSize) const;

	sf::Vector2f getWindowOrigin() const;

	float getMargin() const;
	float getOverlap() const;
	float getVisibilityThreshold() const;
	float getTitleBarRightOffset() const;
	float getTitleBarHeight() const;
	float getTitleBarDownThreshold() const;
	float getResizeMargin() const;

	Titlebar myTitleBar;
	Background myBackground;
	Button myCloseButton;
	Button myMaximizeButton;
	Panel myContent;
	ResizeOverlay myResizeOverlay;

	float myAspectRatio;
	bool myIsProportional;

	sf::FloatRect myUnmaximizedRect;
	bool myIsMaximized;

	sf::Vector2f myMinimumSize;
	sf::Vector2f myMaximumSize;

	bool myIsTitleBarDown;
};

}

#endif
