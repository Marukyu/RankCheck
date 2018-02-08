#ifndef SRC_CLIENT_GUI3_WIDGETS_PANELS_SETTINGSPANEL_HPP_
#define SRC_CLIENT_GUI3_WIDGETS_PANELS_SETTINGSPANEL_HPP_

#include <Client/GUI3/Events/Callback.hpp>
#include <Client/GUI3/Events/MouseEvent.hpp>
#include <Client/GUI3/Types.hpp>
#include <Client/GUI3/Widgets/Controls/Slider.hpp>
#include <Client/GUI3/Widgets/Panels/AbstractPanel.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Utils/Event/CallbackManager.hpp>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace gui3
{

class SettingsPanel : public AbstractPanel<>
{
public:
	SettingsPanel();
	virtual ~SettingsPanel();

	void addButton(std::function<void()> callback, std::string label);
	void addCheckbox(cfg::Bool configKey, std::string label);
	void addSlider(cfg::Float configKey, std::function<std::string(float)> labelProvider, float min, float max);

	float getPreferredHeight() const;

	void update();

private:

	void addBackground();
	void addWidgetToNextSlot(Ptr<Widget> widget);
	CallbackHandle<> addUpdateCallback(EventFunc<> func, int order = 0);

	std::size_t currentSlot = 0;

	CallbackManager<> updateCallbacks;

	// Store scoped callback handles to bind lifetime to settings panel, preventing leaks/crashes.
	std::vector<Callback<>> voidCallbacks;
	std::vector<Callback<MouseEvent>> mouseCallbacks;
	std::vector<Callback<Slider::Event>> sliderCallbacks;
};

}

#endif
