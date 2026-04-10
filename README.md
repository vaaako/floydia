# Requirements
- libxcursor
- libxrandr
- libxi
- mesa (libgl1-mesa-dev on debian)

# improvements
- UniformBuffer and SSBO use mapping
- The raycast to change object property
- Map storage
- Entity Scene system
	+ Objects in scene may have triggers inject to it
	+ Make class "Script" to be a base to other scripts
	+ `on_trigger(Entity other)` and `update(dt)` methods
```cpp
class DoorScript : public Script {
	public:
		bool opened = false;

		void on_trigger(Entity other) override {
			if(other.has<PlayerComponent>()) {
				opened = true;
			}
		}

		void update(float dt) override {
			// open door
		}
};
```

# advanced
- frustum culling
- depth pre-pass
- LOD
- occlusion culling

- Cada arquivo deve ter uma função
- Cada classe deve ter uma função
- Não misturar OpenGL com API da library
