#include <lighting.glsl>

//Shader

layout(location = 0) in Vec3 pos;
layout(location = 1) in Vec2 uv;
layout(location = 2) in Vec3 normal;
layout(location = 3) in flat MaterialHandle material;

layout(location = 0) out Vec4 outColor;

layout(binding = 1) uniform PerExecution {

	Vec3 ambient;
	f32 time;

	Vec2u padding;
	f32 power;
	u32 view;
	
} exc;

layout(binding = 2) uniform Views_noalloc { ViewBuffer viewData; };

layout(binding = 3) uniform sampler samp;
layout(binding = 4) uniform texture2D tex[2];

layout(std430, binding = 5) buffer DirectionalLights {

	Light light[];

} dir;

layout(std430, binding = 6) buffer PointLights {

	Light light[];

} point;

layout(std430, binding = 7) buffer SpotLights {

	Light light[];

} spot;

layout(std430, binding = 8) buffer Materials {

	MaterialStruct mat[];

};

Vec4 sample2D(sampler s, TextureHandle handle, Vec2 uv){
	return texture(sampler2D(tex[handle], s), uv);
}

void main() {

	Camera cam = viewData.cameras[viewData.views[exc.view].camera];

    //Get camera position
	Vec3 cpos = normalize(cam.position - pos);

    //Calculate lighting result
	LightResult lr = { Vec3(0, 0, 0), 0, Vec3(0, 0, 0), 0 };
	
	for(int i = 0; i < dir.light.length(); i++){
		LightResult res = calculateDirectional(dir.light[i], pos, normal, cpos, exc.power);
		lr.diffuse += res.diffuse;
		lr.specular += res.specular;
	}
	
	for(int j = 0; j < point.light.length(); j++){
		LightResult res = calculatePoint(point.light[j], pos, normal, cpos, exc.power);
		lr.diffuse += res.diffuse;
		lr.specular += res.specular;
	}
	
	for(int k = 0; k < spot.light.length(); k++){
		LightResult res = calculateSpot(spot.light[k], pos, normal, cpos, exc.power);
		lr.diffuse += res.diffuse;
		lr.specular += res.specular;
	}
	
    //Get material
	MaterialStruct m = mat[material];
    
    //Get diffuse texture
	Vec3 dif = sample2D(samp, m.t_diffuse, uv).rgb;
    
    //Write lighting result to render target
    outColor = Vec4(calculateLighting(lr, dif, exc.ambient, m) * ((sin(exc.time) * 0.5 + 0.5) * 0.5 + 0.5), 1);

}