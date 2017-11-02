#version 330 compatibility

uniform float	uTime;		// "Time", from Animate( )
uniform bool	ufragShader;
in vec2  	vST;		// texture coords

void
main( )
{
	vec3 myColor = vec3( 0., 1., 1. );
	if( ufragShader )
	{
		myColor = vec3( 1., 0., 0. );
	}
	gl_FragColor = vec4( myColor,  1. );
}
