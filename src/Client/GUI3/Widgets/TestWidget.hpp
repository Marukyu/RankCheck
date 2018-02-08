#ifndef SRC_CLIENT_GUI3_WIDGETS_TESTWIDGET_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_TESTWIDGET_HPP_

#include <Client/GUI3/Widget.hpp>

namespace gui3
{

namespace pieces
{
class Button;
class Text;
}

class Canvas;
class MouseEvent;

class TestWidget : public Widget
{
public:
	TestWidget();
	virtual ~TestWidget();

private:

	void onRepaint(Canvas & canvas) override;

	pieces::Button * myButton;
	pieces::Text * myText;
};

}

#endif
