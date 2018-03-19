// ┌──────────────────────────────────────────────────────────────────┐
// │ Developer : n8vm                                                 |
// │  Workers: Handles common callbacks for rendering, updating,      |
// |    polling events, raycasting, and potentially more.             |
// └──────────────────────────────────────────────────────────────────┘

#include <thread>
#include <atomic>

namespace System::Workers {
	enum CallbackTypes {
		Update, PollEvents, Render, Raycast
	};

	struct Callbacks {
		CallbackTypes currentThreadCallback = Render;
		std::function<void()> updateCallback;
		std::function<void()> pollEventsCallback;
		std::function<void()> renderCallback;
		std::function<void()> raycastCallback;
	};

	/* Threads */
	extern std::thread *UpdateThread;
	extern std::thread *EventThread;
	extern std::thread *RenderThread;
	extern std::thread *RaycastThread;
	
	extern int UpdateRate;
	extern int FrameRate;

	/* Starts and stops worker threads. */
	extern void Start();
	extern void Stop();
	extern void SetupCallbacks(Callbacks callbacks);
}