#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 outTexCoord;

layout(binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    mat4 viewinv;
    mat4 projinv;
    float near;
    float far;
} cbo;

layout(binding = 1) uniform UniformBufferObject {
    mat4 model;
    mat4 scaleMatrix;
    float pointSize;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;
	  float gl_PointSize;
};

void main() {
    vec3 position = inPosition;
    
    
    gl_Position = cbo.proj * cbo.view * ubo.model * vec4(position, 1.0);

    vec4 scaledPos = ubo.scaleMatrix * vec4(position, 1.0);

    gl_PointSize = ubo.pointSize;
    outTexCoord = (scaledPos.xyz + 1.0) * .5;
}
