#include <Client/GUI3/Widgets/Panels/Panel.hpp>

namespace gui3
{

Panel::Panel()
{
}

Panel::~Panel()
{
}

void Panel::add(Ptr<Widget> widget)
{
	own(widget);
}

void Panel::remove(Widget& widget)
{
	disown(widget);
}

}
