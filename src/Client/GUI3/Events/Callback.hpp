#ifndef SRC_CLIENT_GUI3_EVENTS_CALLBACK_HPP_
#define SRC_CLIENT_GUI3_EVENTS_CALLBACK_HPP_

#include <Shared/Utils/Event/CallbackManager.hpp>

template<typename ... Args>
using Callback = typename CallbackManager<Args...>::ScopedHandle;
template<typename ... Args>
using CallbackHandle = typename CallbackManager<Args...>::Handle;
template<typename ... Args>
using EventFunc = typename CallbackManager<Args...>::CallbackFunc;

#endif
