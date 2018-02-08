#ifndef SRC_CLIENT_GUI3_WIDGETS_PANELS_FILLPANEL_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_PANELS_FILLPANEL_HPP_

#include <Client/GUI3/Widgets/Panels/AbstractPanel.hpp>

namespace gui3
{

class FillPanel : public AbstractPanel<>
{
public:

	FillPanel();
	FillPanel(Ptr<Widget> widget);
	virtual ~FillPanel();

	void add(Ptr<Widget> widget);
	void remove(Widget & widget);

private:

	void updateSize();
	void updateWidgetSize(Widget & widget);

	Callback<StateEvent> myFillCallback;
};

}

#endif
