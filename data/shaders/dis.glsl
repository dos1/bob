#ifdef GL_ES
precision highp float;
#endif

const float tau = 6.28318530718;
uniform sampler2D al_tex;
uniform sampler2D displacement;
varying vec2 varying_texcoord;
varying vec4 varying_color;


void main() {

	vec2 pos = varying_texcoord;
	vec4 dis = texture2D(displacement, pos);
	vec4 color = texture2D(al_tex, pos);

color -= vec4(dis.g * 0.3);
	gl_FragColor = color * varying_color;
}
