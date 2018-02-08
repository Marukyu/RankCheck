#ifndef SRC_CLIENT_GUI3_UTILS_NULLRESOURCEMANAGER_HPP_
#define SRC_CLIENT_GUI3_UTILS_NULLRESOURCEMANAGER_HPP_

#include <Client/GUI3/ResourceManager.hpp>

namespace gui3
{

namespace res
{

/**
 * Dummy resource manager that returns null pointers for all resources.
 */
class NullResourceManager : public ResourceManager
{
public:
	NullResourceManager();
	virtual ~NullResourceManager();
};

}

}

#endif
