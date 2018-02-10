#version 450 core
#extension GL_ARB_bindless_texture: require

layout(early_fragment_tests) in;

struct Example {
 uint a0[5], a1[3];
};

struct Example2 {
 uint a2, a3;
};

struct Example3 {
 uint ex[3];
};

struct Example5 {
 Example ex1;
 float test;
};

struct Example4 {
 float test;
 Example3 ex3;
 Example5 ex5;
 Example2 ex2;
};

layout(std430, binding = 0) buffer textureBuffer {
	vec3 discardColor;
	uint textures_c;
	Example3 ex3;
	Example2 ex4[4];
	Example4 ex5;
	uint test2[4][4];
	Example textures[30];
	Example2 ex2;
	sampler2D test[];
};

in vec2 uv;

out layout(location=0) vec4 color;

float rand(vec2 c){
	return fract(sin(dot(c.xy, vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p, float freq ){
	float unit = 1080. / freq;
	vec2 ij = floor(p/unit);
	vec2 xy = mod(p,unit)/unit;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
	xy = .5*(1.-cos(3.1415926535*xy));
	float a = rand((ij+vec2(0.,0.)));
	float b = rand((ij+vec2(1.,0.)));
	float c = rand((ij+vec2(0.,1.)));
	float d = rand((ij+vec2(1.,1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}

float pNoise(vec2 p, int res){
	float persistance = .5;
	float n = 0.;
	float normK = 0.;
	float f = 4.;
	float amp = 1.;
	int iCount = 0;
	for (int i = 0; i<50; i++){
		n+=amp*noise(p, f);
		f*=2.;
		normK+=amp;
		amp*=persistance;
		if (iCount == res) break;
		iCount++;
	}
	float nf = n/normK;
	return nf*nf*nf*nf;
}

vec2 dnoise(vec2 p, float length = 0.01, float d = 0.01){
	float s = pNoise(p, 8);
	float sX = pNoise(p + vec2(-d, 0), 8);
	float sY = pNoise(p + vec2(0, -d), 8);
	return normalize(vec2(s - sX, s - sY)) * length;
}

void main(){
	vec2 f = dnoise(uv * 1028, 0.1);
	vec2 uv2 = f + uv;
	color = vec4(uv2, 0, 1);
}