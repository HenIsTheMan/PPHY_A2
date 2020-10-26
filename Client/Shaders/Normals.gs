#version 330 core
layout (triangles) in; //Takes set of vertices that form a primitive as input (so its input data from the vertex shader is always represented as arrays of vertex data even though we only have a single vertex right now??)
layout (line_strip, max_vertices = 6) out; //Shapes are dynamically generated on the GPU with geometry shaders (better than defining shapes within vertex buffers) so good for simple repeating forms like cubes in a voxel world or grass in a field

in myInterface{
	vec4 colour;
	vec3 normal;
} gsIn[];

out myInterface{
	vec4 colour;
	vec3 normal;
} gsOut;

const float len = 1.2f;

void GenLine(int index){
    for(float i = 0.f; i <= len; i += len){
        gl_Position = gl_in[index].gl_Position + i * vec4(gsIn[index].normal, 0.f);
        gsOut.colour = gsIn[index].colour;
        gsOut.normal = gsIn[index].normal;
        EmitVertex();
    }
    EndPrimitive();
}

void main(){
    for(int i = 0; i < 3; ++i){ 
        GenLine(i);
    }
}