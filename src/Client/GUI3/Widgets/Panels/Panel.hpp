#ifndef SRC_CLIENT_GUI3_WIDGETS_PANELS_PANEL_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_PANELS_PANEL_HPP_

#include <Client/GUI3/Types.hpp>
#include <Client/GUI3/Widgets/Panels/AbstractPanel.hpp>

namespace gui3
{

class Panel : public AbstractPanel<>
{
public:

	Panel();
	virtual ~Panel();

	void add(Ptr<Widget> widget);
	void remove(Widget & widget);
};

}

#endif
