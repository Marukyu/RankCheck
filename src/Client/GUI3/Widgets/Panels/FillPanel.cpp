#include <Client/GUI3/Widgets/Panels/FillPanel.hpp>

namespace gui3
{

FillPanel::FillPanel()
{
	addStateCallback([this](StateEvent event)
	{
		myFillCallback = addStateCallback([this](StateEvent event)
			{
				updateSize();
			}, StateEvent::ParentBoundsChanged);
		updateSize();
	}, StateEvent::ParentChanged);
}

FillPanel::FillPanel(Ptr<Widget> widget) :
	FillPanel()
{
	add(widget);
}

FillPanel::~FillPanel()
{
}

void FillPanel::add(Ptr<Widget> widget)
{
	own(widget);
}

void FillPanel::remove(Widget & widget)
{
	disown(widget);
}

void FillPanel::updateSize()
{
	if (getParent())
	{
		setRect(getParent()->getContainerBoundingBox());
	}

	for (Widget * widget : getContainedWidgets())
	{
		updateWidgetSize(*widget);
	}
}

void FillPanel::updateWidgetSize(Widget & widget)
{
	widget.setRect(getContainerBoundingBox());
}

}

