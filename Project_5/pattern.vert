#version 330 compatibility

uniform float	uTime;		// "Time", from Animate( )
uniform bool	uvertShader;
out vec2  	vST;		// texture coords

const float PI = 	3.14159265;
const float AMP = 	0.2;		// amplitude
const float W = 	2.;		// frequency

void
main( )
{ 
	vST = gl_MultiTexCoord0.st;
	vec3 vert = gl_Vertex.xyz;
	if(uvertShader){
		/*if(vert.x > .95){
			vert.x += uTime;
			vert.y += uTime;
			vert.z += cos(uTime*2*PI);
		}*/
		//if(vert.x == uTime)
		vert.z += cos(uTime*2*PI)*.25*cos(vert.x*PI*4);
	}
	gl_Position = gl_ModelViewProjectionMatrix * vec4( vert, 1. );
}
