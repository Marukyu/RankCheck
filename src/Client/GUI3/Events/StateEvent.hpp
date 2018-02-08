/*
 * StateEvent.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: marukyu
 */

#ifndef SRC_CLIENT_GUI3_EVENTS_STATEEVENT_HPP_
#define SRC_CLIENT_GUI3_EVENTS_STATEEVENT_HPP_

namespace gui3
{

class StateEvent
{
public:

	enum Type
	{
		/**
		 * Empty event mask.
		 */
		None = 0,

		/**
		 * Called whenever the widget's position changes.
		 */
		Moved = 1 << 0,

		/**
		 * Called whenever the widget's size changes.
		 */
		Resized = 1 << 1,

		/**
		 * Called whenever the widget's custom or internal transformation is changed.
		 */
		CustomTransformChanged = 1 << 2,
		InternalTransformChanged = 1 << 3,

		/**
		 * Called whenever the widget or its parent gets focused or unfocused after previously being in a different
		 * state.
		 */
		FocusGained = 1 << 4,
		FocusLost = 1 << 5,

		/**
		 * Called whenever the widget is shown or hidden after previously being in the opposite state.
		 */
		VisibilityChanged = 1 << 6,

		/**
		 * Called whenever the widget is enabled or disabled after previously being in the opposite state.
		 */
		UsabilityChanged = 1 << 7,

		/**
		 * Called whenever a different GUI renderer is selected for the widget (or its parent, if the widget inherits
		 * its renderer).
		 */
		RendererChanged = 1 << 8,

		/**
		 * Called whenever the widget's parent is changed, including being set to or from null.
		 */
		ParentChanged = 1 << 9,

		/**
		 * Called whenever the widget's parent container's bounding box or transformation matrix changes.
		 */
		ParentBoundsChanged = 1 << 10,

		/**
		 * Called whenever the widget's parent application's resources are invalidated.
		 */
		ResourcesChanged = 1 << 11,

		/**
		 * Called whenever the widget's parent application is changed.
		 */
		ParentApplicationChanged = 1 << 12,

		/**
		 * Called whenever the widget's parent's configuration is reloaded.
		 */
		ConfigChanged = 1 << 13,

		/**
		 * Full event mask (for event forwarding).
		 */
		Any = 0x7fffffff
	};

	StateEvent(Type type) :
		type(type)
	{
	}

	const Type type;
};

}

#endif
