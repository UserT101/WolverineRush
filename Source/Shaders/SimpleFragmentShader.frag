#version 330 core

uniform vec4 color;
uniform sampler2D textureSampler;
uniform bool useTexture;

in vec2 texCoord;

// Output data
out vec4 fragCol;

void main()
{
	// Output color from main
    fragCol = useTexture ? texture(textureSampler, texCoord) * color : color;
}