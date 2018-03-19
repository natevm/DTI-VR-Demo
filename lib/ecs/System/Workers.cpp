#pragma once
#include "Workers.hpp"

namespace System::Workers {
	std::atomic<bool> quit = false;

	int UpdateRate = 90;
	int FrameRate = 90;

	std::thread *UpdateThread;
	std::thread *EventThread;
	std::thread *RenderThread;
	std::thread *RaycastThread;

	Callbacks callbacks;

	void SetupCallbacks(Callbacks _callbacks) {
		callbacks = _callbacks;
	}

	void Start() {
		quit = false;

		/* Call the callbacks */
		if (callbacks.updateCallback && !callbacks.currentThreadCallback == Update)
			UpdateThread = new std::thread(callbacks.updateCallback);

		if (callbacks.pollEventsCallback && !callbacks.currentThreadCallback == PollEvents)
			EventThread = new std::thread(callbacks.pollEventsCallback);

		if (callbacks.raycastCallback && !callbacks.currentThreadCallback == Raycast)
			RaycastThread = new std::thread(callbacks.raycastCallback);

		if (callbacks.renderCallback && !callbacks.currentThreadCallback == Render)
			RenderThread = new std::thread(callbacks.renderCallback);

		/* Call the appropriate callback on this thread */
		switch (callbacks.currentThreadCallback) {
		case Update: callbacks.updateCallback(); break;
		case Render: callbacks.renderCallback(); break;
		case PollEvents: callbacks.pollEventsCallback(); break;
		case Raycast: callbacks.raycastCallback(); break;
		}
	}

	void Stop() {
		quit = true;

		if (UpdateThread) UpdateThread->join();
		if (EventThread) EventThread->join();
		if (RaycastThread) RaycastThread->join();
	}
}