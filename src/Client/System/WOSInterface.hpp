#ifndef WOS_INTERFACE_HPP
#define WOS_INTERFACE_HPP

#include <Client/GUI3/Interface.hpp>
#include <Client/System/WOSRenderer.hpp>

class WOSApplication;

class WOSInterface : public gui3::Interface
{
public:

	WOSInterface(WOSApplication * parentApplication);
	virtual ~WOSInterface();

private:

	WOSRenderer myRenderer;
};

#endif
