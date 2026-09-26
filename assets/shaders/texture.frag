#version 330 core
in vec4 vColor;
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uTint = vec4(1, 1, 1, 1);

void main() {
    FragColor = texture(uTexture, vTexCoord) * vColor * uTint;
}
