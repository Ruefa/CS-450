#version 330 compatibility

uniform float	uTime;		// "Time", from Animate( )
uniform bool	ufragShader;
in vec2  	vST;		// texture coords

const float PI = 3.14159265;

void
main( )
{
	vec3 myColor = vec3( 0., 1., 1. );
	if( ufragShader )
	{
		//if(vST.x > .5*uTime && vST.y > .5*uTime)
		if(vST.x*vST.x + vST.y*vST.y > .25 && vST.x*vST.x + vST.y*vST.y < .5*cos(uTime*PI*2)*2)
			myColor = vec3( 0., 1., 0. );
	}
	gl_FragColor = vec4( myColor,  1. );
}
