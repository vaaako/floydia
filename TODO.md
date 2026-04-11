- SSBO Map
	+ Ring Buffer
- Shader Pipeline

- Make `Assets` store members into a unordered map instead

```c++
struct Window {
	WindowObject window; // creates GL context
	Core core; // initialized after, declaration order guarantees it
};
```
