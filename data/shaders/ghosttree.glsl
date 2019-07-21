#ifdef GL_ES
precision highp float;
#endif

const float tau = 6.28318530718;
uniform sampler2D al_tex;
uniform sampler2D displacement;
varying vec2 varying_texcoord;
varying vec4 varying_color;
uniform vec2 tex_whole_pixel_size;
uniform vec4 tex_boundaries;
uniform bool invert;
uniform bool inplace;
uniform bool active;
uniform float time;

float insideBox(vec2 v, vec2 bottomLeft, vec2 topRight) {
	vec2 s = step(bottomLeft, v) - step(topRight, v);
	return s.x * s.y;
}

vec4 clampTexture2D(sampler2D tex, vec2 uv) {
	return insideBox(uv, tex_boundaries.xy, tex_boundaries.zw) * texture2D(tex, uv);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Developed by Masaki Kawase, Bunkasha Games
// Used in DOUBLE-S.T.E.A.L. (aka Wreckless)
// From his GDC2003 Presentation: Frame Buffer Postprocessing Effects in  DOUBLE-S.T.E.A.L (Wreckless)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: maybe use textureOffset

vec4 KawaseBlurFilter( sampler2D tex, vec2 texCoord, vec2 pixelSize, float iteration ) {
	vec2 texCoordSample;
	vec2 halfPixelSize = pixelSize / 2.0;
	vec2 dUV = pixelSize.xy * vec2(iteration, iteration) + halfPixelSize.xy;

	vec4 cOut;

	// Sample top left pixel
	texCoordSample.x = texCoord.x - dUV.x;
	texCoordSample.y = texCoord.y + dUV.y;

	cOut = clampTexture2D(tex, texCoordSample);

	// Sample top right pixel
	texCoordSample.x = texCoord.x + dUV.x;
	texCoordSample.y = texCoord.y + dUV.y;

	cOut += clampTexture2D(tex, texCoordSample);

	// Sample bottom right pixel
	texCoordSample.x = texCoord.x + dUV.x;
	texCoordSample.y = texCoord.y - dUV.y;
	cOut += clampTexture2D(tex, texCoordSample);

	// Sample bottom left pixel
	texCoordSample.x = texCoord.x - dUV.x;
	texCoordSample.y = texCoord.y - dUV.y;

	cOut += clampTexture2D(tex, texCoordSample);

	// Average
	cOut *= 0.25;

	return cOut;
}

// https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-26-50/siggraph2015_2D00_mmg_2D00_marius_2D00_notes.pdf
// also, remember that both algos should actually be done in multiple passes...

vec4 downsample(sampler2D tex, vec2 uv, vec2 halfpixel) {
	vec4 sum = clampTexture2D(tex, uv) * 4.0;
	sum += clampTexture2D(tex, uv - halfpixel.xy);
	sum += clampTexture2D(tex, uv + halfpixel.xy);
	sum += clampTexture2D(tex, uv + vec2(halfpixel.x, -halfpixel.y));
	sum += clampTexture2D(tex, uv - vec2(halfpixel.x, -halfpixel.y));
	return sum / 8.0;
}
vec4 upsample(sampler2D tex, vec2 uv, vec2 halfpixel) {
	vec4 sum = clampTexture2D(tex, uv + vec2(-halfpixel.x * 2.0, 0.0));
	sum += clampTexture2D(tex, uv + vec2(-halfpixel.x, halfpixel.y)) * 2.0;
	sum += clampTexture2D(tex, uv + vec2(0.0, halfpixel.y * 2.0));
	sum += clampTexture2D(tex, uv + vec2(halfpixel.x, halfpixel.y)) * 2.0;
	sum += clampTexture2D(tex, uv + vec2(halfpixel.x * 2.0, 0.0));
	sum += clampTexture2D(tex, uv + vec2(halfpixel.x, -halfpixel.y)) * 2.0;
	sum += clampTexture2D(tex, uv + vec2(0.0, -halfpixel.y * 2.0));
	sum += clampTexture2D(tex, uv + vec2(-halfpixel.x, -halfpixel.y)) * 2.0;
	return sum / 12.0;
}


void main() {
	float t = time / 16.0;

	vec2 pos = varying_texcoord;
	//if (!inplace) {
		float y =
			0.7*sin(mod((varying_texcoord.y + t) * 4.0, tau)) * 0.038 +
			0.3*sin(mod((varying_texcoord.y + t) * 8.0, tau)) * 0.010 +
			0.05*sin(mod((varying_texcoord.y + t) * 40.0, tau)) * 0.05;

		float x =
			0.5*sin(mod((varying_texcoord.y + t) * 5.0, tau)) * 0.1 +
			0.2*sin(mod((varying_texcoord.x + t) * 10.0, tau)) * 0.05 +
			0.2*sin(mod((varying_texcoord.x + t) * 30.0, tau)) * 0.02;

		pos = clamp(1.0 * (varying_texcoord + vec2(y, x) / 8.0), 0.0, 1.0);
	//}

	vec4 dis = clampTexture2D(displacement, pos);

	pos += dis.rg / 256.0;

	vec4 color = clampTexture2D(al_tex, pos) * (1.0 - dis.r * 0.2);

/*	vec2 pixelsize = vec2(1.0) / tex_whole_pixel_size;
	vec4 blur = KawaseBlurFilter(al_tex, pos, pixelsize, 2.0);
	blur += KawaseBlurFilter(al_tex, pos, pixelsize, 4.0);
	blur += KawaseBlurFilter(al_tex, pos, pixelsize, 8.0);
	color += blur;

	color /= 3.0;

	color = vec4(vec3(color.a) - color.rgb, color.a);
*/
color.b += dis.b * 0.125;
color.g += dis.g * 0.025;
	gl_FragColor = color * varying_color;;
}
