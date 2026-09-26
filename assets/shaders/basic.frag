#version 330 core
in vec4 vColor;
out vec4 FragColor;

uniform vec4 uColor = vec4(1, 1, 1, 1);

void main() {
    FragColor = vColor * uColor;
}
