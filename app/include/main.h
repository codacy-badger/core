#pragma once

#include <graphics/graphicsinterface.h>
#include <types/vector.h>
#include <types/matrix.h>
#include <platforms/generic.h>
#include <graphics/texture.h>
#include <utils/json.h>

struct Application {
	static void instantiate(oi::wc::WindowHandleExt *param = nullptr);
};


struct NoiseLayer {

	oi::Vec3 offset;
	u32 octaves = 1;
	f32 persistence = 1, roughness = 1, scale = 1, frequency = 1, minValue = 0;
	bool enabled = true, seeded = true, maskLand = false;

	//Returns 0-scale
	f32 sample(oi::Vec3 pos, f32 current);

	void serialize(oi::JSONNode &json, bool save);

};

struct Planet {

	std::vector<NoiseLayer> noiseLayer = { {} };

	f32 minHeight = -0.6f, scale = 1, coastSize = 0.1f;
	bool randomize = true;
	oi::String name = "earth";

	oi::Vec3 offset;

	//Returns 0-1 for a point on the planet
	f32 sample(oi::Vec3 pos);

	void seed();

	void serialize(oi::JSONNode &node, bool save);

};

class MainInterface : public oi::gc::GraphicsInterface {

public:

	~MainInterface();
	void initScene() override;
	void renderScene() override;
	void onInput(oi::wc::InputDevice *device, oi::wc::Binding b, bool down) override;
	
	void load(oi::String path) override {}
	void save(oi::String path) override {}

	void update(f32 dt) override;
	void initSceneSurface() override;

	void refreshPlanet(Planet planet);
	void readPlanet(Planet &planet, oi::String name, bool fromResource = false);
	void writePlanet(Planet planet);

private:
	
	oi::gc::Shader *shader, *shader0;
	oi::gc::Pipeline *pipeline, *pipeline0;
	oi::gc::PipelineState *pipelineState;
	oi::gc::Sampler *sampler;
	oi::gc::Camera *camera;
	oi::gc::MeshBuffer *meshBuffer, *meshBuffer0;
	oi::gc::Mesh *mesh, *mesh0, *mesh1, *mesh2, *mesh3 = nullptr;
	oi::gc::DrawList *drawList, *drawList0;
	oi::gc::RenderTarget *renderTarget;

	oi::gc::Texture *osomi, *water, *rock;
	u32 hwater, hrock;

	float exposure = .15f, gamma = .85f;
	oi::Vec2 prevMouse;

	struct PerObject {

		oi::Matrix m;
		oi::Matrix mvp;

		u32 diffuse;
		u32 specular;
		u32 ambient;
		u32 padding;

	};

	static constexpr u32 totalObjects = 4U;

	PerObject objects[totalObjects];

	oi::Vec3 planetRotation;

	Planet earth;

};