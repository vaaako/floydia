# Examples
> ![WARNING]
> Right now this is just some features that i will put here to make an example Markdown later

Multi window example
```cpp
Window win1(...);
Window win2(...);

void renderLoop(Window& win) {
	win.enable_ctx();
	while(win.is_open()) {
		win.poll_events();
		win.begin_draw(camera); // enable context
		// ...
		win.flush();
		win.swap_buffers(); // disable context
		win.clear();
	}
	win.disable_ctx();
}
```
