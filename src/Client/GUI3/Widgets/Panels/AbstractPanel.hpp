#ifndef SRC_CLIENT_GUI3_WIDGETS_ABSTRACTPANEL_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_ABSTRACTPANEL_HPP_

#include <Client/GUI3/Container.hpp>
#include <Client/GUI3/Events/Callback.hpp>
#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Types.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <memory>
#include <vector>

namespace gui3
{

struct EmptyPanelData
{
};

/**
 * Simple container without automatic layout management.
 */
template<typename ExtraData = EmptyPanelData>
class AbstractPanel : public Container
{
public:

	AbstractPanel() :
		myWidgetScale(1, 1)
	{
	}

	virtual ~AbstractPanel()
	{
		while (!myOwnedWidgets.empty())
		{
			dropOwnership(myOwnedWidgets.end() - 1);
		}
	}

protected:

	/**
	 * Adds the widget to the container, leaving lifetime management up to the caller.
	 * 
	 * Call this function inside the "add" function of subclasses.
	 */
	bool own(Ptr<Widget> widget)
	{
		if (takeOwnership(widget))
		{
			return addWidget(*widget);
		}
		else
		{
			return false;
		}
	}

	/**
	 * Removes the widget from the container.
	 * 
	 * If the widget is owned by this container, it is automatically deallocated.
	 * 
	 * Call this function inside the "remove" function of subclasses.
	 */
	bool disown(Widget& widget)
	{
		if (removeWidget(widget))
		{
			return dropOwnership(widget);
		}
		else
		{
			return false;
		}
	}

	/**
	 * Returns a pointer to a custom data structure holding information for a specific widget in the panel.
	 */
	ExtraData * getWidgetData(Widget & widget) const
	{
		auto it = findWidget(widget);

		if (it == myOwnedWidgets.end() || it->widget.get() != &widget)
		{
			return nullptr;
		}

		return it->data.get();
	}

private:

	struct OwnedWidget
	{
		Ptr<Widget> widget;
		std::unique_ptr<ExtraData> data;
		CallbackHandle<StateEvent> callback;
	};

	bool takeOwnership(Ptr<Widget> widget)
	{
		if (widget == nullptr)
		{
			return false;
		}

		auto it = findWidget(*widget);

		Widget* widgetPointer = widget.get();

		if (it != myOwnedWidgets.end() && it->widget.get() == widgetPointer)
		{
			return false;
		}

		auto func = [this, widgetPointer](StateEvent event)
			{
				if (widgetPointer->getParent() != this)
				{
					dropOwnership(*widgetPointer);
				}
			};

		OwnedWidget info;
		info.widget = widget;
		info.data = makeUnique<ExtraData>();
		info.callback = widget->addStateCallback(std::move(func), StateEvent::ParentChanged, 0);

		myOwnedWidgets.insert(it, std::move(info));

		return true;
	}

	bool dropOwnership(Widget& widget)
	{
		auto it = findWidget(widget);

		if (it == myOwnedWidgets.end() || it->widget.get() != &widget)
		{
			return false;
		}

		dropOwnership(it);

		return true;
	}

	void dropOwnership(typename std::vector<OwnedWidget>::iterator it)
	{
		myDisownedWidgets.push_back(std::move(*it));
		myOwnedWidgets.erase(it);
		invokeLater([=]
		{
			myDisownedWidgets.clear();
		});
	}

	typename std::vector<OwnedWidget>::iterator findWidget(Widget & widget)
	{
		return std::lower_bound(myOwnedWidgets.begin(), myOwnedWidgets.end(), &widget,
			[](const OwnedWidget& left, Widget* right)
			{
				return left.widget.get() < right;
			});
	}

	typename std::vector<OwnedWidget>::const_iterator findWidget(Widget & widget) const
	{
		return std::lower_bound(myOwnedWidgets.begin(), myOwnedWidgets.end(), &widget,
			[](const OwnedWidget& left, Widget* right)
			{
				return left.widget.get() < right;
			});
	}

	std::vector<OwnedWidget> myOwnedWidgets;
	std::vector<OwnedWidget> myDisownedWidgets;

	sf::Vector2f myWidgetScale;
};

}

#endif
