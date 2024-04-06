#version 460 core

layout(location = 0) in vec3 aPos;

out vec2 uv_coord;
out int objectID;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

layout(std430, binding = 0) buffer layoutObjectID
{
    int dataID[];
};

vec2 texCoords[4] = vec2[4](
        vec2(0.0f,1.0f),
        vec2(1.0f,1.0f),
        vec2(0.0f,0.0f),
        vec2(1.0f,0.0f)
);

void main(){
        mat4 mvp = Projection * View * Model;
        gl_Position = mvp * vec4(aPos,1);
        uv_coord = texCoords[gl_VertexID%4];
        objectID = dataID[gl_VertexID/24];
}

