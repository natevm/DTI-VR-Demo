#extension GL_ARB_separate_shader_objects : enable
layout(binding = 0) uniform sampler3D volumeTexture;

layout(location = 0) in vec3 inTexCoord;
layout(location = 0) out vec4 outColor;
//uniform int samples;

//const float maxDist = sqrt(2.0);
//float step_size = maxDist/float(samples);
//float s0s = (step_size*float(samples))/maxDist;
float rand(vec2 co) {
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
	vec3 texCoord = inTexCoord;
	// vec4 uvw = vec4((inTexCoord + 1.0) * .5, 1.0);
	// uvw = inverse(ubo.model) * uvw;

	// uvw /= ubo.size;
	// uvw += ubo.offset;
	if (any(greaterThan(texCoord, vec3(1.0, 1.0, 1.0))) || any(lessThan(texCoord, vec3(0.0, 0.0, 0.0))))
		discard;

	//float average = (value.r + value.g + value.b) * .333;
	//outColor = value;
	//outColor.a = average;

	// vec3 pad = (inTexCoord / ubo.size) - inTexCoord;

	float value = texture(volumeTexture, vec3(texCoord.x, texCoord.y, texCoord.z)).r;
	outColor = vec4(value, value, value, value) * 4.0;
}