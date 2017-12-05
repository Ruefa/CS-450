#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include "glew.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"

//#include "room.cpp"
#include <time.h>
#include <stdlib.h>


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Alex Ruef

// NOTE: There are a lot of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.


// title of these windows:

const char *WINDOWTITLE = { "OpenGL / GLUT Sample -- Alex Ruef" };
const char *GLUITITLE   = { "User Interface Window" };


// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:

enum Projections
{
	ORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	RESET,
	QUIT
};


// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };


// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };


// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char * ColorNames[ ] =
{
	"Red",
	"Yellow",
	"Green",
	"Cyan",
	"Blue",
	"Magenta",
	"White",
	"Black"
};


// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};


// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to use the z-buffer
GLuint	BoxList;				// object display list
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void	Axes( float );
void	HsvRgb( float[3], float [3] );

float dx = BOXSIZE / 2.f;
float dy = BOXSIZE / 2.f;
float dz = BOXSIZE / 2.f;
float doorMult = 3.5;
float roomDistMult = 3.;
GLuint hallwayList;
GLuint doorDisplayList;

float lookAngle = 0.0;
float eyePos[3];
float lookPos[3];

int numRooms = 0;
struct room {
	float x, y, z;
	GLuint	wallList;
	struct room *door0 = NULL;
	struct room *door1 = NULL;
	struct room *door2 = NULL;
	struct room *door3 = NULL;

	struct doorList *door_trans0 = NULL;
	struct doorList *door_trans1 = NULL;
	struct doorList *door_trans2 = NULL;
	struct doorList *door_trans3 = NULL;
}headRoom;

struct pointList {
	float x, y, z;
	struct pointList *next = NULL;
	struct room *roomOnPoint = NULL;
}headPoint;

struct doorList {
	//float x, y, z;
	float yTrans = 0;
	bool ascending = false;
	struct doorList *next = NULL;
}headDoor;

void addPoint(struct room *newRoom) {
	struct pointList *newPoint = (struct pointList *)malloc(sizeof(pointList));

	newPoint->x = newRoom->x;
	newPoint->y = newRoom->y;
	newPoint->z = newRoom->z;
	newPoint->roomOnPoint = newRoom;

	newPoint->next = headPoint.next;
	headPoint.next = newPoint;
}

struct doorList * addDoor(struct doorList *newDoor) {
	//struct doorList *newDoor = (struct doorList *)malloc(sizeof(doorList));
	struct doorList *index;

	if (newDoor == NULL) {
		newDoor = (struct doorList *)malloc(sizeof(doorList));
		newDoor->ascending = false;
		newDoor->yTrans = 0.;
	}
	else {
		index = headDoor.next;
		while (index != NULL) {
			if (index == newDoor) {
				return newDoor;
			}
			index = index->next;
		}
	}

	newDoor->next = headDoor.next;
	headDoor.next = newDoor;

	return newDoor;
}

void removeDoor(struct doorList *toRemove) {
	struct doorList *index;
	struct doorList *prev;

	index = headDoor.next;
	prev = &headDoor;
	while (index != NULL) {
		printf("looping here");
		if (index == toRemove) {
			prev->next = index->next;
			return;
		}
		index = prev;
		index = index->next;
	}
}

struct room * exists(struct pointList *curPoint, float x, float y, float z) {
	if (curPoint->x == x && curPoint->y == y && curPoint->z == z) {
		return curPoint->roomOnPoint;
	}
	
	if (curPoint->next != NULL)
		return exists(curPoint->next, x, y, z);
	else
		return NULL;
}

struct room * findRoom(struct room *curRoom, struct room *prevRoom, float x, float z) {
	struct room *bufferRoom;

	if (x > curRoom->x - dx && x < curRoom->x + dx && z > curRoom->z - dz && z < curRoom->z + dz) {
		return curRoom;
	}

	if (curRoom->door0 != NULL && curRoom->door0 != prevRoom) {
		bufferRoom = findRoom(curRoom->door0, curRoom, x, z);

		if (bufferRoom != NULL) {
			return bufferRoom;
		}
	}
	if (curRoom->door1 != NULL && curRoom->door1 != prevRoom) {
		bufferRoom = findRoom(curRoom->door1, curRoom, x, z);

		if (bufferRoom != NULL) {
			return bufferRoom;
		}
	}
	if (curRoom->door2 != NULL && curRoom->door2 != prevRoom) {
		bufferRoom = findRoom(curRoom->door2, curRoom, x, z);

		if (bufferRoom != NULL) {
			return bufferRoom;
		}
	}
	if (curRoom->door3 != NULL && curRoom->door3 != prevRoom) {
		bufferRoom = findRoom(curRoom->door3, curRoom, x, z);

		if (bufferRoom != NULL) {
			return bufferRoom;
		}
	}

	return NULL;
}

struct doorList * findDoor() {
	struct room *curRoom = findRoom(&headRoom, NULL, eyePos[0], eyePos[2]);

	if (curRoom != NULL) {
		if (eyePos[0] < curRoom->x + dx / 2 && eyePos[0] > curRoom->x - dx / 2 && eyePos[2] < curRoom->z + dz && eyePos[2] > curRoom->z + dz - dz / 2) {
			curRoom->door_trans0 = addDoor(curRoom->door_trans0);
			curRoom->door0->door_trans1 = addDoor(curRoom->door0->door_trans1);
		}
		else if (eyePos[0] < curRoom->x + dx / 2 && eyePos[0] > curRoom->x - dx / 2 && eyePos[2] > curRoom->z - dz && eyePos[2] < curRoom->z - dz + dz / 2) {
			printf("near a door1\n");
			curRoom->door_trans1 = addDoor(curRoom->door_trans1);
			curRoom->door1->door_trans0 = addDoor(curRoom->door1->door_trans0);
		}
		else if (eyePos[0] < curRoom->x + dx && eyePos[0] > curRoom->x + dx - dx / 2 && eyePos[2] < curRoom->z + dz / 2 && eyePos[2] > curRoom->z - dz / 2) {
			printf("near a door2\n");
			curRoom->door_trans2 = addDoor(curRoom->door_trans2);
			curRoom->door2->door_trans3 = addDoor(curRoom->door2->door_trans3);
		}
		else if (eyePos[0] > curRoom->x - dx && eyePos[0] < curRoom->x - dx + dx / 2 && eyePos[2] < curRoom->z + dz / 2 && eyePos[2] > curRoom->z - dz / 2) {
			printf("near a door3\n");
			curRoom->door_trans3 = addDoor(curRoom->door_trans3);
			curRoom->door3->door_trans2 = addDoor(curRoom->door3->door_trans2);
		}
	}

	return NULL;
}

void InitDungeon(struct room *curRoom) {
	int randDoor;
	bool created = false;
	bool *existing = (bool *)malloc(sizeof(bool)*3);

	//initialize head nodes for tree and list structures
	if (numRooms == 0) {
		curRoom->x = 0.;
		curRoom->y = 0.;
		curRoom->z = 0.;

		headPoint.x = 0.;
		headPoint.y = 0.;
		headPoint.z = 0.;
		headPoint.roomOnPoint = &headRoom;
	}
	else {
		addPoint(curRoom);
	}

	numRooms++;

	// Coding myself out of a corner
	if (curRoom->door0 != NULL) {
		existing[0] = true;
	}
	else if (exists(&headPoint, curRoom->x, curRoom->y, curRoom->z + roomDistMult*dz) != NULL) {
		existing[0] = true;
	}
	else {
		existing[0] = false;
	}
	if (curRoom->door1 != NULL) {
		existing[1] = true;
	}
	else if (exists(&headPoint, curRoom->x, curRoom->y, curRoom->z - roomDistMult*dz) != NULL) {
		existing[1] = true;
	}
	else {
		existing[1] = false;
	}
	if (curRoom->door2 != NULL) {
		existing[2] = true;
	}
	else if (exists(&headPoint, curRoom->x + roomDistMult*dx, curRoom->y, curRoom->z) != NULL) {
		existing[2] = true;
	}
	else {
		existing[2] = false;
	}
	if (curRoom->door3 != NULL) {
		existing[3] = true;
	}
	else if (exists(&headPoint, curRoom->x - roomDistMult*dx, curRoom->y, curRoom->z) != NULL) {
		existing[3] = true;
	}
	else {
		existing[3] = false;
	}

	if (numRooms < 100) {
		while (!created) {
			randDoor = rand() % 4;

			switch (randDoor) {
			case 0:
				if (!existing[0]) {
					curRoom->door0 = (struct room *)malloc(sizeof(struct room));
					curRoom->door0->door0 = NULL;
					curRoom->door0->door1 = curRoom;
					curRoom->door0->door2 = NULL;
					curRoom->door0->door3 = NULL;
					curRoom->door0->x = curRoom->x;
					curRoom->door0->y = curRoom->y;
					curRoom->door0->z = curRoom->z + roomDistMult*dz;

					curRoom->door0->door_trans0 = NULL;
					curRoom->door0->door_trans1 = NULL;
					curRoom->door0->door_trans2 = NULL;
					curRoom->door0->door_trans3 = NULL;
					InitDungeon(curRoom->door0);
					created = true;
				}
				break;

			case 1:
				if (!existing[1]) {
					curRoom->door1 = (struct room *)malloc(sizeof(struct room));
					curRoom->door1->door0 = curRoom;
					curRoom->door1->door1 = NULL;
					curRoom->door1->door2 = NULL;
					curRoom->door1->door3 = NULL;
					curRoom->door1->x = curRoom->x;
					curRoom->door1->y = curRoom->y;
					curRoom->door1->z = curRoom->z - roomDistMult*dz;
					curRoom->door1->door_trans0 = NULL;
					curRoom->door1->door_trans1 = NULL;
					curRoom->door1->door_trans2 = NULL;
					curRoom->door1->door_trans3 = NULL;
					InitDungeon(curRoom->door1);
					created = true;
				}
				break;

			case 2:
				if (!existing[2]) {
					curRoom->door2 = (struct room *)malloc(sizeof(struct room));
					curRoom->door2->door0 = NULL;
					curRoom->door2->door1 = NULL;
					curRoom->door2->door2 = NULL;
					curRoom->door2->door3 = curRoom;
					curRoom->door2->x = curRoom->x + roomDistMult*dx;
					curRoom->door2->y = curRoom->y;
					curRoom->door2->z = curRoom->z;
					curRoom->door2->door_trans0 = NULL;
					curRoom->door2->door_trans1 = NULL;
					curRoom->door2->door_trans2 = NULL;
					curRoom->door2->door_trans3 = NULL;
					InitDungeon(curRoom->door2);
					created = true;
				}
				break;

			case 3:
				if (!existing[3]) {
					curRoom->door3 = (struct room *)malloc(sizeof(struct room));
					curRoom->door3->door0 = NULL;
					curRoom->door3->door1 = NULL;
					curRoom->door3->door2 = curRoom;
					curRoom->door3->door3 = NULL;
					curRoom->door3->x = curRoom->x - roomDistMult*dx;
					curRoom->door3->y = curRoom->y;
					curRoom->door3->z = curRoom->z;
					curRoom->door3->door_trans0 = NULL;
					curRoom->door3->door_trans1 = NULL;
					curRoom->door3->door_trans2 = NULL;
					curRoom->door3->door_trans3 = NULL;
					InitDungeon(curRoom->door3);
					created = true;
				}
				break;

			default:
				printf("Incorrect room number in door generation");
				break;
			}

			if ((existing[0] && existing[1] && existing[2] && existing[3]) || numRooms >= 100) {
				created = true;
			}
			else if (rand() % 4 == 0) {
				created = false;
			}
		}
	}
}

void roomLists(struct room *curRoom, struct room *prevRoom) {
	curRoom->wallList = glGenLists(1);
	glNewList(curRoom->wallList, GL_COMPILE);

	glBegin(GL_QUADS);

	glColor3f(0., 0.5, 0.);

	//wall0
	glNormal3f(0., 0., 1.);
	if (curRoom->door0 == NULL) {
		glVertex3f(-dx, -dy, dz);
		glVertex3f(dx, -dy, dz);
		glVertex3f(dx, dy, dz);
		glVertex3f(-dx, dy, dz);
	}
	else {
		//top portion
		glVertex3f(-dx, dy, dz);
		glVertex3f(-dx, 0, dz);
		glVertex3f(dx, 0, dz);
		glVertex3f(dx, dy, dz);

		//door side 1
		glVertex3f(-dx, 0, dz);
		glVertex3f(-dx, -dy, dz);
		glVertex3f(-dx / doorMult, -dy, dz);
		glVertex3f(-dx / doorMult, 0, dz);

		//door side 2
		glVertex3f(dx, 0, dz);
		glVertex3f(dx, -dy, dz);
		glVertex3f(dx / doorMult, -dy, dz);
		glVertex3f(dx / doorMult, 0, dz);
	}


	//wall1
	glNormal3f(0., 0., -1.);
	if (curRoom->door1 == NULL) {
		glTexCoord2f(0., 0.);
		glVertex3f(-dx, -dy, -dz);
		glTexCoord2f(0., 1.);
		glVertex3f(-dx, dy, -dz);
		glTexCoord2f(1., 1.);
		glVertex3f(dx, dy, -dz);
		glTexCoord2f(1., 0.);
		glVertex3f(dx, -dy, -dz);
	}
	else {
		//top portion
		glVertex3f(-dx, dy, -dz);
		glVertex3f(-dx, 0, -dz);
		glVertex3f(dx, 0, -dz);
		glVertex3f(dx, dy, -dz);

		//door side 1
		glVertex3f(-dx, 0, -dz);
		glVertex3f(-dx, -dy, -dz);
		glVertex3f(-dx / doorMult, -dy, -dz);
		glVertex3f(-dx / doorMult, 0, -dz);

		//door side 2
		glVertex3f(dx, 0, -dz);
		glVertex3f(dx, -dy, -dz);
		glVertex3f(dx / doorMult, -dy, -dz);
		glVertex3f(dx / doorMult, 0, -dz);
	}


	//wall2
	glNormal3f(1., 0., 0.);
	if (curRoom->door2 == NULL) {
		glVertex3f(dx, -dy, dz);
		glVertex3f(dx, -dy, -dz);
		glVertex3f(dx, dy, -dz);
		glVertex3f(dx, dy, dz);
	}
	else {
		glVertex3f(dx, 0, dz);
		glVertex3f(dx, 0, -dz);
		glVertex3f(dx, dy, -dz);
		glVertex3f(dx, dy, dz);

		glVertex3f(dx, 0, -dz);
		glVertex3f(dx, -dy, -dz);
		glVertex3f(dx, -dy, -dz / doorMult);
		glVertex3f(dx, 0, -dz / doorMult);

		glVertex3f(dx, 0, dz);
		glVertex3f(dx, -dy, dz);
		glVertex3f(dx, -dy, dz / doorMult);
		glVertex3f(dx, 0, dz / doorMult);
	}

	//wall3
	glNormal3f(-1., 0., 0.);
	if (curRoom->door3 == NULL) {
		glVertex3f(-dx, -dy, dz);
		glVertex3f(-dx, dy, dz);
		glVertex3f(-dx, dy, -dz);
		glVertex3f(-dx, -dy, -dz);
	}
	else {
		glVertex3f(-dx, 0, dz);
		glVertex3f(-dx, 0, -dz);
		glVertex3f(-dx, dy, -dz);
		glVertex3f(-dx, dy, dz);

		glVertex3f(-dx, 0, -dz);
		glVertex3f(-dx, -dy, -dz);
		glVertex3f(-dx, -dy, -dz / doorMult);
		glVertex3f(-dx, 0, -dz / doorMult);

		glVertex3f(-dx, 0, dz);
		glVertex3f(-dx, -dy, dz);
		glVertex3f(-dx, -dy, dz / doorMult);
		glVertex3f(-dx, 0, dz / doorMult);
	}

	//ceiling
	glColor3f(0.87, 0.72, 0.53);
	glNormal3f(0., 1., 0.);
	glVertex3f(-dx, dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(-dx, dy, -dz);

	//floor
	glColor3f(.5, 0.5, 0.5);
	glNormal3f(0., -1., 0.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(-dx, -dy, -dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, -dy, dz);

	glEnd();
	glEndList();

	if (curRoom->door0 != NULL && curRoom->door0 != prevRoom) {
		roomLists(curRoom->door0, curRoom);
	}
	if (curRoom->door1 != NULL && curRoom->door1 != prevRoom) {
		roomLists(curRoom->door1, curRoom);
	}
	if (curRoom->door2 != NULL && curRoom->door2 != prevRoom) {
		roomLists(curRoom->door2, curRoom);
	}
	if (curRoom->door3 != NULL && curRoom->door3 != prevRoom) {
		roomLists(curRoom->door3, curRoom);
	}
}

void displayRooms(struct room *curRoom, struct room *prevRoom) {

	glPushMatrix();
	glTranslatef(curRoom->x, curRoom->y, curRoom->z);
	glCallList(curRoom->wallList);
	glPopMatrix();

	//draw second door from previous room
	if (curRoom->door0 != NULL) {
		glPushMatrix();
		if (curRoom->door_trans0 != NULL) {
			glTranslatef(0., curRoom->door_trans0->yTrans, 0.000001);
		}
		glTranslatef(curRoom->x, curRoom->y, curRoom->z);
		glCallList(doorDisplayList);
		glPopMatrix();
	}
	if (curRoom->door1 != NULL) {
		glPushMatrix();
		if (curRoom->door_trans1 != NULL) {
			glTranslatef(0., curRoom->door_trans1->yTrans, -0.000001);
		}
		glTranslatef(0, 0, -2 * dz);
		glTranslatef(curRoom->x, curRoom->y, curRoom->z);
		glCallList(doorDisplayList);
		glPopMatrix();
	}
	if (curRoom->door2 != NULL) {
		glPushMatrix();
		if (curRoom->door_trans2 != NULL) {
			glTranslatef(0.000001, curRoom->door_trans2->yTrans, 0.);
		}
		glTranslatef(curRoom->x, curRoom->y, curRoom->z);
		glRotatef(90, 0., 1., 0.);
		glCallList(doorDisplayList);
		glPopMatrix();
	}
	if (curRoom->door3 != NULL) {
		glPushMatrix();
		if (curRoom->door_trans3 != NULL) {
			glTranslatef(-0.000001, curRoom->door_trans3->yTrans, 0.);
		}
		glTranslatef(curRoom->x, curRoom->y, curRoom->z);
		glRotatef(-90, 0., 1., 0.);
		glCallList(doorDisplayList);
		glPopMatrix();
	}

	//hallways
	if (curRoom->door0 != NULL && curRoom->door0 != prevRoom) {
		glPushMatrix();
		glTranslatef(curRoom->x, curRoom->y, curRoom->z);
		glCallList(hallwayList);
		glPopMatrix();

		displayRooms(curRoom->door0, curRoom);
	}
	if (curRoom->door1 != NULL && curRoom->door1 != prevRoom) {
		glPushMatrix();
		glTranslatef(0, 0, -3 * dz);
		glTranslatef(curRoom->x, curRoom->y, curRoom->z);
		glCallList(hallwayList);
		glPopMatrix();

		displayRooms(curRoom->door1, curRoom);
	}
	if (curRoom->door2 != NULL && curRoom->door2 != prevRoom) {
		glPushMatrix();
		glTranslatef(curRoom->x, curRoom->y, curRoom->z);
		glRotatef(90, 0., 1., 0.);
		glCallList(hallwayList);
		glPopMatrix();

		displayRooms(curRoom->door2, curRoom);
	}
	if (curRoom->door3 != NULL && curRoom->door3 != prevRoom) {
		glPushMatrix();
		glTranslatef(curRoom->x, curRoom->y, curRoom->z);
		glRotatef(-90, 0., 1., 0.);
		glCallList(hallwayList);
		glPopMatrix();

		displayRooms(curRoom->door3, curRoom);
	}
}

// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );


	// setup all the graphics stuff:

	InitGraphics( );

	// init dungeon

	srand(time(NULL));

	eyePos[0] = 0.;
	eyePos[1] = -.3;
	eyePos[2] = 0.;

	lookPos[0] = 0.;
	lookPos[1] = eyePos[1];
	lookPos[2] = 1.;

	InitDungeon(&headRoom);

	// create the display structures that will not change:

	InitLists( );


	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );


	// setup all the user interface stuff:

	InitMenus( );


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );


	// this is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
	struct doorList *index;

	index = headDoor.next;
	while (index != NULL) {
		if(!index->ascending)
			index->yTrans += .05;
		else 
			index->yTrans -= .05;
		if (index->yTrans >= 1.) {
			index->ascending = true;
			removeDoor(index);
			index = &headDoor;
		}
		else if (index->yTrans <= 0.) {
			index->yTrans = 0.;
			index->ascending = false;
			removeDoor(index);
			index = &headDoor;
		}
		index = index->next;
	}

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}


	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );


	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	if( DepthBufferOn != 0 )
		glEnable( GL_DEPTH_TEST );
	else
		glDisable( GL_DEPTH_TEST );


	// specify shading to be flat:

	glShadeModel( GL_FLAT );


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );


	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );


	// set the eye position, look-at position, and up-vector:

	gluLookAt( eyePos[0], eyePos[1], eyePos[2],    eyePos[0] + lookPos[0], lookPos[1], eyePos[2] + lookPos[2],     0., 1., 0. );


	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );


	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );


	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}


	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[WhichColor][0] );
		glCallList( AxesList );
	}


	// since we are using glScalef( ), be sure normals get unitized:

	glEnable( GL_NORMALIZE );


	// draw the current object:

	displayRooms(&headRoom, NULL);

	//glCallList(hallwayList);

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	WhichColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	WhichProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Colors",        colormenu);
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );
	glutIdleFunc( Animate );

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	glutSetWindow( MainWindow );

	// create the object:

	roomLists(&headRoom, NULL);

	hallwayList = glGenLists(1);
	glNewList(hallwayList, GL_COMPILE);

	glBegin(GL_QUADS);

		glColor3f(0., 0., 0.5);
		glVertex3f(-dx / doorMult, 0, dz);
		glVertex3f(-dx / doorMult, -dy, dz);
		glVertex3f(-dx / doorMult, -dy, dz*2);
		glVertex3f(-dx / doorMult, 0, dz*2);

		glVertex3f(dx / doorMult, 0, dz);
		glVertex3f(dx / doorMult, -dy, dz);
		glVertex3f(dx / doorMult, -dy, dz*2);
		glVertex3f(dx / doorMult, 0, dz*2);

		glColor3f(0.87, 0.72, 0.53);
		glVertex3f(-dx / doorMult, 0, dz);
		glVertex3f(dx / doorMult, 0, dz);
		glVertex3f(dx / doorMult, 0, dz*2);
		glVertex3f(-dx / doorMult, 0, dz*2);

		glColor3f(0.5, 0.5, 0.5);
		glVertex3f(-dx / doorMult, -dy, dz);
		glVertex3f(dx / doorMult, -dy, dz);
		glVertex3f(dx / doorMult, -dy, dz*2);
		glVertex3f(-dx / doorMult, -dy, dz*2);

	glEnd();
	glEndList();

	doorDisplayList = glGenLists(1);
	glNewList(doorDisplayList, GL_COMPILE);

	glBegin(GL_QUADS);

	glColor3f(.16, 0.1, 0.05);
		glVertex3f(-dx / doorMult, 0, dz);
		glVertex3f(-dx / doorMult, -dy, dz);
		glVertex3f(dx / doorMult, -dy, dz);
		glVertex3f(dx / doorMult, 0, dz);

	glEnd();
	glEndList();

	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	float units = 0.1;
	float angleUnits = 0.05;

	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			WhichProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		case 'w':
			eyePos[0] += lookPos[0] * units;
			eyePos[2] += lookPos[2] * units;
			break;

		case 's':
			eyePos[0] -= lookPos[0] * units;
			eyePos[2] -= lookPos[2] * units;
			break;

		case 'd':
			lookAngle += angleUnits;
			lookPos[0] = sin(lookAngle);
			lookPos[2] = -cos(lookAngle);
			break;

		case 'a':
			lookAngle -= angleUnits;
			lookPos[0] = sin(lookAngle);
			lookPos[2] = -cos(lookAngle);
			break;

		case 'e':
			findDoor();
			break;

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}


	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}


	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r, g, b;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}
