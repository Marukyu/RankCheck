#include <Client/GUI3/Events/StateEvent.hpp>
#include <Client/GUI3/Interface.hpp>
#include <Client/GUI3/Widgets/Controls/Button.hpp>
#include <Client/GUI3/Widgets/Controls/Checkbox.hpp>
#include <Client/GUI3/Widgets/Controls/Table.hpp>
#include <Client/GUI3/Widgets/Controls/Text.hpp>
#include <Client/GUI3/Widgets/Panels/FillPanel.hpp>
#include <Client/GUI3/Widgets/Panels/Panel.hpp>
#include <Client/GUI3/Widgets/Panels/SettingsPanel.hpp>
#include <Shared/Config/Config.hpp>

namespace gui3
{

static cfg::Float itemHeight("gui.widgets.settingsPanel.itemHeight");
static cfg::Float margin("gui.widgets.settingsPanel.margin");

SettingsPanel::SettingsPanel()
{
	addBackground();
	addStateCallback([=](StateEvent event)
	{
		update();
	}, StateEvent::ParentApplicationChanged);
}

SettingsPanel::~SettingsPanel()
{
}

void SettingsPanel::addButton(std::function<void()> callback, std::string label)
{
	auto button = make<Button>(label);
	voidCallbacks.push_back(button->addActionCallback(callback));
	addWidgetToNextSlot(button);
}

void SettingsPanel::addCheckbox(cfg::Bool configKey, std::string label)
{
	auto checkbox = make<Checkbox>(label);

	auto updateValue = [=]()
	{
		checkbox->setChecked(config().get(configKey));
	};

	mouseCallbacks.push_back(checkbox->addMouseCallback([=](MouseEvent)
	{
		config().set(configKey, checkbox->isChecked());
		if (getParentInterface())
		{
			getParentInterface()->getRootContainer().fireStateEvent(StateEvent::ConfigChanged);
		}
	}, MouseEvent::Click));

	addUpdateCallback([=]()
	{
		updateValue();
	});

	updateValue();

	addWidgetToNextSlot(checkbox);
}

void SettingsPanel::addSlider(cfg::Float configKey, std::function<std::string(float)> labelProvider, float min,
	float max)
{
	auto label = make<Text>();
	auto slider = make<Slider>();

	auto updateText = [=]()
	{
		label->setText(labelProvider(config().get(configKey)));
	};

	auto updateValue = [=]()
	{
		slider->setValue(config().get(configKey));
		updateText();
	};

	slider->setBounds(min, max);
	slider->setScrollSpeed(0);

	sliderCallbacks.push_back(slider->addEventCallback([=](Slider::Event)
	{
		config().set(configKey, slider->getValue());
		updateText();
	}, Slider::Event::UserChanged));

	slider->addMouseCallback([=](MouseEvent)
	{
		if (getParentInterface())
		{
			getParentInterface()->getRootContainer().fireStateEvent(StateEvent::ConfigChanged);
		}
	}, MouseEvent::ButtonUp);

	addUpdateCallback([=]()
	{
		updateValue();
	});

	updateValue();

	addWidgetToNextSlot(label);
	addWidgetToNextSlot(slider);
}

float SettingsPanel::getPreferredHeight() const
{
	return config().get(itemHeight) * currentSlot + config().get(margin) * (currentSlot + 1);
}

void SettingsPanel::update()
{
	updateCallbacks.fireCallback(1);
}

void SettingsPanel::addBackground()
{
	auto background = make<Table>();
	background->setScrollBarVisible(false);

	auto fillPanel = make<FillPanel>(background);
	fillPanel->setZPosition(-1);

	own(fillPanel);
}

void SettingsPanel::addWidgetToNextSlot(Ptr<Widget> widget)
{
	std::size_t slot = currentSlot++;

	own(widget);

	auto updateRect = [=]()
	{
		widget->setPosition(config().get(margin), config().get(itemHeight) * slot + config().get(margin) * (slot + 1));
		widget->setSize(getSize().x - config().get(margin) * 2, config().get(itemHeight));
	};

	addStateCallback([=](StateEvent)
	{
		updateRect();
	}, StateEvent::Resized | StateEvent::ConfigChanged);

	updateRect();
}

CallbackHandle<> SettingsPanel::addUpdateCallback(EventFunc<> func, int order)
{
	return updateCallbacks.addCallback(func, 1, order);
}

}
