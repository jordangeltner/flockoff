//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
// (JT: I prefer to keep line lengths <=80 chars for easy printing/display.
//
// EECS 351-2 'Starter Code' for Project A: Simple + State 2D Bouncing Ball
//
//             VERSION D -- First use of State Space...
//                          --All improvements of Version C, plus:
//                          --Made parallel bouncing ball using CPartSys class
//                              and its assoc'd classes CPart, CForcer, CWall.
//                              (new ball located 10cm lower and rightwards).
//                          --In CPartSys class, implemented basic versions of:
//                              initExample(), applyAllForces(), dotMaker(),
//                              render(),solver(),stateVecSwap(),stateVecZero(),
//                              StateVecAplusBtimesC(), etc.
//                          --Implemented basic CPart class, but with separate
//                              named member vars for mass,pos,vel, ftot, etc
//                          --Implemented basic CForcer class, with one object
//                              for the 'gravity' force.
//                          --Implemented basic CWall class, with one object for
//                              each of the 4 walls.
//

#include "glutBall.h"
using namespace std;        // access common C++ along with OpenGL's C functions

//=====================
// GLOBAL VARIABLES: bad idea, but forced on us by GLUT's callback fcns.
//  Do your best to keep the number of global vars low.
//  Good Idea:  JUST ONE global variable, an instance of a C++ class whose
//              member vars hold all the values you need.
//=====================
CTheApp myApp;              // our one-and-only global variable; holds all the
                            // values we need to get into/out of callback fcns.

// OLD 'Bouncing Ball':
// (duplicate in a CPartSys-based particle system, then delete this old one)
//-------------------------------------
GLdouble mass;              // mass of the ball: --> CPart object member
GLdouble gravConst;         // gravitational constant: --> CForcer object 'grav'
GLdouble timeStep;          // integration timestep -->CPartSys object member
GLdouble myDamp;            // velocity-damping factor (pg 57, textbook);
                            // --> CPartSys member
GLdouble myRest;            // Coeff. of Restitution for wall-bouncing
                            //  --> CWall object member
GLdouble ball_xpos,ball_zpos;       //Ball  position, velocity, acceleration,
GLdouble ball_xvel,ball_zvel;       // --> CPart member.
GLdouble ball_xacc,ball_zacc;       // --> CPart member.

GLdouble lookAtX, lookAtY, lookAtZ = 0;
GLdouble cameraX = 0;
GLdouble cameraZ = 10;
GLdouble cameraY = -28;

GLdouble gravity = 9.8;
int flocking = 0;
int debug = 0;
int solver = 0;
bool spring = false;
bool spring2 = false;

//NEW: particle system----------------
CPartSys bball;
//========================
// END Global vars



int main( int argc, char *argv[] )
//==============================================================================
{
    //--Init Bouncing Ball state:----------------
    // NEW: state-based particle system
     if (debug == 1)
        cout << "entering initExample1\n";
    bball.initExample1();       // init the 'parallel' particle system...

	// OLD Bouncing ball (not based on state-space classes)
    // Use MKS units (Meter,Kilogram,Seconds) for all physical measurements.
    mass = 1.0;                 // ball mass in kilgrams (kg)
    gravConst = 9.8;            // gravity accelerates ANY mass at 9.8m/s^2
                                // (by applying force of mass*gravConst)
    timeStep = 1.0/60.0;        // assumes 60Hz refresh/redisplay rate
    myDamp = 0.995;             // velocity drag/damping (smaller-->more stable)
                                // (==1.0 for ideal, frictionless system)
    myRest = 0.9;              // Coeff. of restitution for wall-bounces.
    ball_xpos = ball_zpos = 0.0;
    ball_xvel = ball_zvel = 2.6;
    ball_xacc = 0.0; ball_zacc = -gravConst;
    // Acceleration due to gravity (at earth's surface) DOES NOT depend on mass:
    // it's always 9.8 meters/sec^2.  Gravity applies force of gravConst*mass.
    //------------------------------------------

     if (debug == 1)
        cout << "entering open gl\n";
    myApp.oglStartup(argc, argv);
                            // Do ALL graphics startup: openGL/GLUT/GLM, etc;
                            // then enter GLUT's event-loop and stay there.
                            // !ALWAYS PUT THIS CALL AT THE VERY END OF MAIN()
                            // because call to oglStartup() will never return!
 }

void onReshape( int width, int height )
//------------------------------------------------------------------------------
// Called by GLUT on initial creation of the window, using the width,height
// arguments given in glutInitWindowSize(); see main() fcn. ALSO called by GLUT
// whenever you re-size the display window to a different width, height.
{
    myApp.window_width    = width;    // save the current display size
    myApp.window_height   = height;   // in these global vars.

    glViewport(0,0,width,height);   // fill the display window with CVV contents
    glMatrixMode(GL_PROJECTION);    // Select the camera matrix, and
    glLoadIdentity();               // Clear out any old contents.
    gluPerspective( 35.0,           // 25.0 degree vertical Field of view,
                   (GLdouble)width/ (GLdouble)height,   // aspect ratio matches
                                                        // display window,
                    0.1,            // zNear = 10cm from camera origin
                    100.0);         // zFar = 100 meters from camera origin.
    // (With the help of the 'gluLookAt()' call in onDisplay() callback fcn,
    //  we'll define 'world' coord. system in meters, with x,y horizontal, and
    //  'up' in the +z direction.)
	glutPostRedisplay();			// request redraw--we changed window size!
}

void onDisplay( void )
//------------------------------------------------------------------------------
// Registered in GLUT as the display callback function. GLUT calls this function
// whenever the OS needs us to re-draw the screen
// (or whenever this program calls glutPostRedisplay()).
// OPENGL HINTS:
//  glTranslated(0.1,0.3,0.5);	// translate along x,y,z axes.
//	glRotated(3 , 1.1,-2.1,1.3);// rotate by 3 degrees around a 3D axis vector;
//	glScaled(0.2, 0.3, 0.4);	// shrink along x,y,z axes
{

    // Clear the screen (to change screen-clearing color, call glClearColor() )
    glClearColor( 0.0, 0.0, 0, 0.0 );	// screen-clear color: dark blue.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                                // clear the color AND depth buffers. Now draw:

	glMatrixMode(GL_MODELVIEW);     // select the modeling matrix stack
	glLoadIdentity();               // clear it.  Drawing axes at 'eye'.

                                    // Now create 'world' coord. system axes
                                    // in front of the camera:

	gluLookAt(cameraX,cameraY,cameraZ,       // Camera loc. in 'world' coords (in meters)
              lookAtX,lookAtY,lookAtZ,       // Aim camera to 'look at' world origin;
              0.0,  0.0, 1.0);      // with 'up' in the +z direction.

    //==================WORLD COORD SYSTEM
	drawAxes();
	glRotated(90,1,0,0);		            // draw simple +x,+y,+z axes as R,G,B.
	groundPlane(100,1);
    // NEW: Draw our entire particle system------------------------------------
    bball.dotMaker(bball.pS0dot, bball.pS0, bball.pF0, bball.pC0);
                                        // find current state deriv: pS0dot
    bball.render(bball.pS0, bball.pF0, bball.pC0);
                                        // draw current particle system
    bball.solver();                   // find next state of particle system
    bball.doConstraints(bball.pS1, bball.pS0, bball.pC0);
                                        // adjust to comply with constraints,
    if (debug == 1)
        cout << "swapping vectors\n";
    bball.stateVecSwap( &(bball.pS0), &(bball.pS1) );
    if (debug == 1)
        cout << "vectors swapped\n";
                                        // swap s0 and s1 contents.
    //===============DRAWING DONE.
    glutSwapBuffers();                // show result on-screen.
}

void onKeyboard(unsigned char key, int xw, int yw)
//------------------------------------------------------------------------------
// GLUT CALLBACK: Don't call this function in your program--GLUT does it every
// time user presses an alphanumeric key on the keyboard.
//	xw,yw == mouse position in window-system pixels (origin at UPPER left)

{
int i;
GLdouble tmp;
	switch(key)
	{
		case 'A':       // User pressed the 'A' key...
		case 'a':
            cout << "that *IS* the 'A' key!" << endl;
            break;
        case 'R':
        case 'r':
            myApp.xMrot = 0.0;
            myApp.yMrot = 0.0;
            ball_xpos = ball_zpos = 0.0;
            ball_xvel = ball_zvel = 2.6;
            // NEW!!
            // restart state-space particle sys.
            if(bball.solverType == NU_SOLV_AD_HOC)
                bball.initExample1();
            else bball.initExample2();  // (keep same solver type)
            cout << "reset Bouncing ball." << endl;
            break;
        case 'd':       // REDUCE damping amount: make 'myDamp' closer to 1.0
            myDamp += (1.0 - myDamp)*0.2;       // 20% closer to 1.0
            if(myDamp > 1.0) myDamp = 1.0;      // stay in 0.5 to 1.0 range.
            cout << "myDamp = " << myDamp << endl;
            // ad-hoc state-space version:-----
            bball.myDamp += (1.0 - bball.myDamp)*0.2;   // 20% closer to 1.0
            if(bball.myDamp > 1.0) bball.myDamp = 1.0;  // stay within 0.5 - 1.0
            cout << "bball.myDamp = " << bball.myDamp << endl;
            break;
        case 'D':       // INCREASE damping amount: make myDamp further below 1
            myDamp -= (1.0 - myDamp + 0.001)*0.2; // 20% further from 1.0,
                                        // (add 0.001 for when myDamp==1.0)
            if(myDamp < 0.5) myDamp = 0.5;      // stay in 0.5 to 1.0 range.
            cout << "myDamp= " << myDamp << endl;
            // ad-hoc state-space version:-----
            bball.myDamp -= (1.0 - bball.myDamp + 0.001)*0.2;
                                        // 20% further from 1.0,
                                        // (add 0.001 for when myDamp==1.0)
            if(bball.myDamp < 0.5) bball.myDamp = 0.5;  // stay within 0.5 - 1.0
            cout << "bball.myDamp= " << bball.myDamp << endl;
            break;
        case 'k':
            if (spring)
            {
                spring = false;
                cout << "spring off\n";
            }
            else
            {
                spring = true;
                cout << "spring on\n";
            }
            break;
        case 'K':
            if (spring2)
            {
                spring2 = false;
                cout << "big spring off\n";
            }
            else
            {
                spring2 = true;
                cout << "big spring on\n";
                bball.initExample2();
            }
            break;
        case 'f':
            if (flocking == 0)
            {
                cout << "flocking on\n";
                flocking = 1;
                bball.initExample2();
            }
            else
            {
                cout << "flocking off\n";
                flocking = 0;
                bball.initExample2();
            }
            break;
        case 'h':
            cout << "Help Menu Reached!!!!!!\n";
            cout << "Arrows to move camera forwards.\npageUP and pageDown to ";
            cout << "move up and down.\nright alt and right control to pan left ";
            cout << "and right.\nwith home and end to pan down and up.\n";
            cout << "s key changes the solver.\nr resets the particles to a new ";
            cout << "random position.\nw and d can be used to change the damping.\n";
            cout << "f turns flocking on and off.\n";
            cout << "g turns gravity on and off.\n";
            cout << "k turns spring force on and off.\n";
            cout << "K turns on a big connected spring that could be a soft object.\n";
            break;
        case 'W':       // INCREASE wall-bouncyness (coeff. of restitution)
            myRest += (1.0 - myRest)*0.2;       // 20% closer to 1.0
            if(myRest > 1.0) myRest = 1.0;      // stay in 0.5 to 1.0 range.
            cout << "myRest= " << myRest << endl;
            // ad-hoc state-space version:-----
            tmp = bball.pC0[0].Kbouncy;         //Get current restoration coeff.
            tmp += (1.0 - tmp)*0.2;             // go 20% closer to 1.0
            if(tmp > 1.0) tmp = 1.0;            // but stay within 0.5 - 1.0.
            cout << "bball.pC0[0].Kbouncy= " << tmp << endl;
            for(i=0; i< bball.wallCount; i++)   // Set all wall constraints.
                bball.pC0[i].Kbouncy = tmp;
            break;
        case 'w':       // REDUCE wall-bouncyness (coeff. of restitution)
            myRest -= (1.0 - myRest + 0.001)*0.2; // 20% further from 1.0,
                                        // (add epsilon for when myRest==1.0)
            if(myRest < 0.5) myRest = 0.5;      // stay in 0.5 to 1.0 range.
            cout << "myRest= " << myRest << endl;
            // ad-hoc state-space version:-----
            tmp = bball.pC0[0].Kbouncy;        // Get current restoration coeff.
            tmp -= (1.0 - tmp + 0.001)*0.2;    // go 20% further from 1.0
                                               // (add epsilon for when tmp==1)
            if(tmp < 0.5) tmp = 0.5;           // stay in 0.5 to 1.0 range.
            cout << "bball.pC0[0].Kbouncy = " << tmp << endl;
            for(i=0; i<bball.wallCount; i++)    // set all wall constraints.
                bball.pC0[i].Kbouncy = tmp;
            break;
        case 'S':
        case 's':
            if (solver == 0) //ad hoc
            {
                cout << "changing to Euler's\n";
                bball.solverType = NU_SOLV_EULER;
                solver++;
            }
            else if (solver == 1) //euler's
            {
                cout << "changing to midpoint\n";
                bball.solverType = NU_SOLV_MIDPOINT;
                solver++;
            }
            else if (solver == 2) //midpoint
            {
                cout << "changing to verlet lol\n";
                bball.solverType = NU_SOLV_VERLET;
                solver++;
            }
            else if (solver == 3)//verlet:
            {
                cout << "changing to backward midpoint\n";
                bball.solverType = NU_SOLV_BACK_MIDPT;
                solver = 4;
            }
            else //backward midpoint
            {
                cout << "changing to ad hoc\n";
                bball.solverType = NU_SOLV_AD_HOC;
                solver = 0;
            }

            /*bball.solverType = (bball.solverType+1)%2;  // switch between two;
            if(bball.solverType == NU_SOLV_EULER)
                cout << "Euler Solver." << endl;
            else
                cout << "Ad-Hoc Solver." << endl;*/
            break;
        case 'g':
        case 'G':
            if (gravity != 0)
            {
                gravity = 0;
                cout << "Gravity is off.\n";
            }
            else
            {
                gravity = 9.8;
                cout << "Gravity is on.\n";
            }

            break;
        case 'P':       // run/stop animation timer with 'p' key
        case 'p':
            runAnimTimer(2);
            break;
        case ' ':       // Single-step animation with 'space' key.
            runAnimTimer(3);
            break;
		case 27:		// User pressed the 'Esc'  key...
		case 'Q':		// User pressed the 'Q' key...
		case 'q':
			exit(0);    // Done! quit the program.
			break;
        default:
            cout << "I don't respond to the '" << key <<"' key." << endl;
            break;
	}
}

void onKeySpecial(int key, int xw, int yw)
//------------------------------------------------------------------------------
// Called by GLUT when users press 'arrow' keys, 'Fcn' keys, or any other
// non-alphanumeric key.  Search glut.h to find other #define literals for
// special keys, such as GLUT_KEY_F1, GLUT_KEY_UP, etc.
//	xw,yw == mouse position in window-system pixels (origin at UPPER left!)/
{
	switch(key)
	{
		case GLUT_KEY_LEFT:		// left arrow key
            cout << "left-arrow key.\n";
            cameraX -= 1;
            lookAtX -= 1;
			break;
		case GLUT_KEY_RIGHT:	// right arrow key
            cout << "right-arrow key.\n";
            cameraX += 1;
            lookAtX += 1;
			break;
		case GLUT_KEY_DOWN:		// dn arrow key
            cout << "dn-arrow key.\n";
            cameraY -= 1;
            lookAtY -= 1;
			break;
		case GLUT_KEY_UP:		// up arrow key
            cout << "up-arrow key.\n";
            cameraY += 1;
            lookAtY += 1;
			break;
        case GLUT_KEY_PAGE_UP:
            cout << "page up key.\n";
            cameraZ += 1;
            lookAtZ += 1;
            break;
        case GLUT_KEY_PAGE_DOWN:
            cout << "page down key.\n";
            cameraZ -= 1;
            lookAtZ -= 1;
            break;
        case GLUT_KEY_HOME:
            lookAtX -= 1;
            break;
        case GLUT_KEY_END:
            lookAtX += 1;
            break;
        case GLUT_KEY_CTRL_R:
            lookAtY += 1;
            break;
        case GLUT_KEY_ALT_R:
            lookAtY -= 1;
            break;
		// SEARCH glut.h for more arrow key #define statements.
		default:
			cout << "Special key; integer code value"<< (int)key << "\n";
			break;
	}
}

void onMouseClik(int buttonID, int upDown, int xw, int yw)
//-----------------------------------------------------------------------------
// Called by GLUT on mouse button click or un-click
// When user clicks a mouse button,  buttonID== 0 for left mouse button,
//                              		    (== 1 for middle mouse button?)
//	                                 		 == 2 for right mouse button;
//	                        upDown  == 0 if mouse button was pressed down,
//	                                == 1 if mouse button released.
//	xw,yw == mouse position in window-system pixels (origin at UPPER left!)
{
int xpos,ypos;  // mouse position in coords with origin at lower left.
GLdouble cvvX,cvvY;     // approx mouse position measured in the CVV:

    xpos = xw;
    ypos = myApp.window_height - yw;
                                    //(window system puts origin at UPPER left)

    cout << "click; buttonID=" << buttonID <<", upDown=" << upDown;
    cout << ", at xpos,ypos=(" << xpos <<"," << ypos << ")\n";

    // save most-recent click in these two global vars
    myApp.xMclik = xpos;
    myApp.yMclik = ypos;
    //But these are measured in pixels;
    // How can we measure mouse position in CVV?
    // Depends glViewport() command above!
    // let's just approximate:  (to make it exact, see: onReshape())
    //          0.0 <= (GLdouble)xpos / getDisplayWidth()  <= 1.0
    //          0.0 <= (GLdouble)ypos / getDisplayHeight() <= 1.0
    //  so to get +/- 1 range of the CVV, multiply these [0,1] range
    //  values by 2.0 and subtract 1:
    // Use that approximate click location draw some text:
    cvvX = -1.0 + 2.0*((GLdouble)myApp.xMclik / myApp.window_width);
    cvvY = -1.0 + 2.0*((GLdouble)myApp.yMclik / myApp.window_height);
    glColor3d(0.8, 1.0, 0.8);	//Bright green text:
    drawText2D(rom24, cvvX, cvvY, "!MOUSE CLICK!");
    glutSwapBuffers();
    //===============DRAWING DONE.
}

void onMouseMove(int xw, int yw)
//-----------------------------------------------------------------------------
// Called by GLUT when user moves mouse while pressing any mouse button.
//	xw,yw == mouse position in window-system pixels (origin at UPPER left)
{
int xpos,ypos;  // mouse position in coords with origin at lower left.

    xpos = xw;
    ypos = myApp.window_height - yw; //(window system puts origin at UPPER left)

    myApp.yMrot += 0.3*(GLdouble)(myApp.xMclik - xpos);
                                // change rot. angle by drag distance
                                // (drag in +X direction rotates on +Y axis)
    myApp.xMrot -= 0.3*(GLdouble)(myApp.yMclik - ypos);
                                // (drag in +Y direction rotates on -X axis)
    cout << "xpos= " << xpos << " xMclik= "<< myApp.xMclik << endl;

    myApp.xMclik = xpos;                   // update current mouse position.
    myApp.yMclik = ypos;
    cout << ".";
    cout << "xMrot,yMrot= " << myApp.xMrot << ", " << myApp.yMrot << endl;
    //===============DRAWING DONE.

}

void runAnimTimer(int isOn)
//-----------------------------------------------------------------------------
// Call runAnimTimer(0) to STOP animation;
//      runAnimTimer(1) to START or CONTINUE it.
//      runAnimTimer(2) to TOGGLE it (off->on; on->off).
//      runAnimTimer(3) to STEP it forward by 1 frame.
{
    switch(isOn)
    {
        case 0: // --STOP animation--don't register a GLUT timer with onTimer().
            myApp.anim_doMore = 0;      // Update animation flag.
            break;
        case 1: //YES. ANIMATE!   register a GLUT timer callback:
            myApp.anim_doMore = 1;      // Update animation flag.
            glutTimerFunc(NU_PAUSE, onTimer, 1);
        // This registers 'onTimer()' as a timer callback function.  This causes
        // GLUT to pause NU_PAUSE milliseconds (longer if busy drawing), then
        // to call onTimer(1), and then to 'un-register' the callback.  GLUT
        // won't call onTimer() again unless we register it again.
        //
        // The call to onTimer() is tricky.  First check anim_doMore to be SURE
        // no other 'stop animation' requests arrived (e.g. from keyboard input,
        // mouse input, visibility change, etc.) while GLUT's timer ran.
        // If global variable anim_doMore still has value 1, then onTimer()
        // calls runAnimTimer(1) to continue the animation.
            break;
        case 2: // TOGGLE animation on/off
            if(myApp.anim_doMore==0)    // if we're NOT animating,
            {
                myApp.anim_doMore = 1;  // Animate.
                glutTimerFunc(NU_PAUSE, onTimer, 1);
            }
            else myApp.anim_doMore = 0; // if we ARE animating, stop.
            break;
        case 3: // STEP animation forward by 1 frame:
            myApp.anim_doMore = 0;                  // stop animating, BUT
            glutTimerFunc(NU_PAUSE, onTimer, 1);    // wait for 1 more frame and
            glutPostRedisplay();                    // show result on-screen.
            break;
        default:
            myApp.anim_doMore = 0;      // stop animating, and
            cout << "ERROR! I don't understand runAnimTimer(" << isOn;
            cout << "). Stopped Animation." << endl;
            break;
    }
}

void onTimer (int value)
//------------------------------------------------------------------------------
// Don't call this function yourself; use runAnimTimer() instead!
// runAnimTimer(1) causes GLUT to wait NU_PAUSE milliseconds (longer if busy)
// and then call this function.
// TRICKY: We allow users to choose very long NU_PAUSE times to see what happens
// in animation.  During GLUT's wait time, user events (keyboard, mouse, or
// window cover/uncover) may make runAnimTimer(0) calls intended to STOP
// animation.  GLUT can't cancel a previously-issued GLUT timer callback, so
// in the TimerCallback() function we check global variable nu_Anim_isON.
{
                                // GLUT finished waiting.
    /*
    if(value != myApp.anim_doMore)  // does user still want more animation?
        cout << "\n(Anim cancelled during GLUT wait)\n";
    */
    if(myApp.anim_doMore==1)    // Go on with animation?
    {                           // YES.
        glutPostRedisplay();    // tell GLUT to re-draw the screen, and
        runAnimTimer(1);        // continue with the next time interval.
    }
                                // NO. Do nothing. No further animation.
}

void onHidden (int isVisible)
//------------------------------------------------------------------------------
// Called by GLUT when our on-screen window gets covered or un-covered. When
// called, it turns off animation and re-display when our display window is
// covered or stowed (not visible). SEE ALSO: runAnimTimer() and onTimer()
{
    if(isVisible == GLUT_VISIBLE &&     // IF window is newly visible, AND
       myApp.anim_doMore==1)            // user wants animation, then
        runAnimTimer(1);                // YES! Restart Animation!
    else runAnimTimer(0);                           // No. Stop animating.
}

void drawAxes(void)
//------------------------------------------------------------------------------
// Draw small +x,+y,+z axes in red, green, blue:
{
	glColor3d(1.0, 0.0, 0.0);	// red
	glBegin(GL_LINES);
		glVertex3d(0.0,0.0,0.0);	// +x-axis
		glVertex3d(0.4,0.0,0.0);
	glEnd();
	glColor3d(0.0, 1.0, 0.0);	// green
	glBegin(GL_LINES);
		glVertex3d(0.0,0.0,0.0);	// +y-axis
		glVertex3d(0.0,0.4,0.0);
	glEnd();
	glColor3d(0.0, 0.5, 1.0);	// bright blue
	glBegin(GL_LINES);
		glVertex3d(0.0,0.0,0.0);	// +z-axis
		glVertex3d(0.0,0.0,0.4);
	glEnd();
}

void drawText3D(void *pFont, double x0,double y0,double z0, const char* pString)
//-----------------------------------------------------------------------------
//  Using the specified font, write the text in 'pString' to the display window
// at 3D location x0,y0,z0 in the CURRENT 3D drawing axes using current color.
// CAREFUL! This is trickier to use than drawText2D()!
// CAREFUL! Some or all of your text might be hiding inside a 3D object! e.g. if
// you put text at the origin of the current coord system (x0,y0,z) = (0,0,0)
// enable openGL's depth-testing, AND draw a teapot at that origin, then some or
// all of your text may hide inside!
//      Usage example:
//              glColor3d(1.0,1.0,0.0);     // bright yellow color
//              drawText3D(helv18, 0.3,0.4 ,"Hello!");  // print text in 3D;
//
// Available fonts:
// helv10 (HELVETICA size 10)   rom10 (TIMES_ROMAN size 10)
// helv12 (HELVETICA size 12)   rom24 (TIMES_ROMAN size 24)
// helv18 (HELVETICA size 18)
{
int i, imax;					// character counters

	glRasterPos3d(x0,y0,z0);	// set text's lower-left corner position
                                // in current drawing axes (set by contents
                                // of GL_MODELVIEW, GL_PROJECTION matrices)
	imax = 1023;				// limit the number of chars printed.
	for(i=0; pString[i] != '\0' && i<imax; i++)	// for each char,
	{
		glutBitmapCharacter(pFont, pString[i]);
	}
}

void drawText2D(void *pFont, double x0, double y0, const char* pString)
//------------------------------------------------------------------------------
// same as drawText3D(), but puts text on top of the displayed image.
// Draws text within the CVV, so be sure x0,y0 fit within +/-1.0.
//      Usage example:
//              glColor3d(1.0,1.0,0.0);     // bright yellow color
//              drawText2D(helv18, 0.3,0.4 ,"the 'M' key changes materials!");
//
// Available fonts:
// helv10 (HELVETICA size 10)   rom10 (TIMES_ROMAN size 10)
// helv12 (HELVETICA size 12)   rom24 (TIMES_ROMAN size 24)
// helv18 (HELVETICA size 18)
{
GLboolean isLit;
GLint curMatMode;

    glGetIntegerv(GL_MATRIX_MODE, &curMatMode);
                                    // save the currently-selected matrix-mode.
    glMatrixMode(GL_PROJECTION);    // Then temporarily turn off openGL camera,
    glPushMatrix();                 // save it.
        glLoadIdentity();           // put in a 'do-nothing' matrix
        glMatrixMode(GL_MODELVIEW); // temporarily turn off all model xforms,
            glPushMatrix();         // save them
                glLoadIdentity();       // put in a 'do-nothing' matrix,
                glGetBooleanv(GL_LIGHTING,&isLit); // save the lighting state;
                if(isLit == GL_TRUE)
                {
                    glDisable(GL_LIGHTING);  // temp. disable;
                    glColor3d(1.0,1.0,1.0);     // Set text color to 'white'
                    drawText3D(pFont, x0, y0, 0.0, pString); // print it!
                    glEnable(GL_LIGHTING);  // re-enable.
                }
                else
                {   // lighting is OK; don't bother it, just print text.
                    drawText3D(pFont, x0, y0, 0.0, pString); // print it!
                }
            glPopMatrix();              // restore previous model xforms,
        glMatrixMode(GL_PROJECTION);    // go back to camera matrix mode,
    glPopMatrix();                      // restore previous openGL camera.
    glMatrixMode(curMatMode);           // restore previous matrix mode. Done!
}


//==========================
//
// CPart Member Functions
//
//==========================
CPart::CPart(void)
{

}

CPart::~CPart(void)
{

}


//==========================
//
// CForcer Member Functions
//
//==========================
CForcer::CForcer(void)
{

}
CForcer::~CForcer(void)
{
}

void CForcer::drawMe(void)
//------------------------------------------------------------------------------
{
    if (debug == 1)
        cout << "drawing forces\n";

    switch(forceType)
    {
        case F_SPRING:
            break;
        default:
        // Keep this force-making object invisible on-screen.
            break;
    }
    if (debug == 1)
        cout << "forces drawn\n";
}

//==========================
//
// CWall Member Functions
//
//==========================
CWall::CWall(void)
{

}

CWall::~CWall(void)
{

}

void CWall::drawMe(void)
//------------------------------------------------------------------------------
{
    glColor3f(1.0, 0.5, 0.2);       // bright red/orange color
    switch(wallType)
    {
        case WTYPE_XWALL_HI:        // (2D) walls perpendicular to X axis.
            glBegin(GL_LINES);
                glVertex3d(xmax, 0.0, -1.5);
                glVertex3d(xmax, 0.0,  1.5);
            glEnd();
            break;
        case WTYPE_XWALL_LO:
            glBegin(GL_LINES);
                glVertex3d(xmin, 0.0, -1.5);
                glVertex3d(xmin, 0.0,  1.5);
            glEnd();
            break;
        case WTYPE_ZWALL_HI:    // (2D) walls perpendicular to Z axis.
            glBegin(GL_LINES);
                glVertex3d(-1.5, 0.0, zmax);
                glVertex3d( 1.5, 0.0, zmax);
            glEnd();
            break;
        case WTYPE_ZWALL_LO:
            glBegin(GL_LINES);
                glVertex3d(-1.5, 0.0, zmin);
                glVertex3d( 1.5, 0.0, zmin);
            glEnd();
            break;
        default:
            break;
    }

}

//==========================
//
// CPartSys Member Functions
//
//==========================
CPartSys::CPartSys(void)
{
}

CPartSys::~CPartSys(void)
{
}

void CPartSys::initExample1(void)
//------------------------------------------------------------------------------
// Init a simple bouncy-ball, use the cheezy built-in ballA,ballB CPart objects,
// implementing an offset EXACT COPY of the ad-hoc bouncy-ball of version C.
{
    cout < "press h for the help menu.\n";
    int i;
    timeStep = 1.0/60.0;        // assumes 60Hz refresh/redisplay rate
    myDamp = 0.995;             // set particle's velocity drag/damping
                                // (smaller-->more stable:   see textbook, pg58)
                                //  ==1.0 for ideal, frictionless system.

    solverType = NU_SOLV_AD_HOC;
                                // recreates the 'ad-hoc' method we used for
                                // our pre-state-space 'bouncy-ball' example.

    if( (flocking == 1) || (spring2 == true))
        partCount = 100;
    else
        partCount = 1000;
    CPart * tmp,*tmp2,*tmp3,*tmp4,*tmp5, *tmp1, *tmp6, *tmp7; //pointers for dynamically created particles
    tmp = new CPart[partCount];
    tmp1 = new CPart[partCount];
    tmp2 = new CPart[partCount];
    tmp3 = new CPart[partCount];
    tmp4 = new CPart[partCount];
    tmp5 = new CPart[partCount];
    tmp6 = new CPart[partCount];
    tmp7 = new CPart[partCount];

    pS0 = tmp;   // current ball-system state
    pS1 = tmp2;
    pSM = tmp1;
    pSerr = tmp7;

    for (i = 0; i < partCount; i++)
    {
        pS0[i].xpos = (rand() % partCount - partCount/2)/8;
        pS0[i].ypos = (rand() % partCount - partCount/2)/8;
        pS0[i].zpos = (rand() % partCount)/25;
        pS0[i].xvel = 0;
        pS0[i].yvel = 0;
        pS0[i].zvel = 0;
        pS0[i].mass = 1;
        pS0[i].xftot = 0;
        pS0[i].yftot = 0;
        pS0[i].zftot = 0;
    }


    pS0dot = tmp3;
    pS1dot = tmp4;
                    // and another for its second time-derivative
    pS0dot2 = tmp5;
    pSMdot = tmp6;


    // init the force-making object:--------------------------------------------

    forcerCount = 4;  // number of forces.
    CForcer * tmpForce;
    tmpForce = new CForcer[forcerCount];
    pF0 = tmpForce;
                            // list of CForcer objects.

   /* pF0->forceType = F_GRAV_E;  // type of force: earth gravity
    pF0->gravConst =  gravity;  // acceleration due to gravity, in meters/sec^2.
                            // multiply this by mass (Kg) to get force.
    pF0->downDir[0] = 0.0;  // World-coords vector for direction of gravity.
    pF0->downDir[1] = -1.0;
    pF0->downDir[2] = 0.0;*/


    pF0[0].forceType = F_GRAV_E;
    pF0[0].gravConst = gravity;
    pF0[0].downDir[0] = 0;
    pF0[0].downDir[1] = -1;
    pF0[0].downDir[2] = 0;

    pF0[1].forceType = F_WIND;
    pF0[1].gravConst = gravity;
    pF0[1].downDir[0] = 0;
    pF0[1].downDir[1] = 0;
    pF0[1].downDir[2] = 0;

    pF0[2].forceType = F_BUBBLE;
    pF0[2].gravConst = gravity;
    pF0[2].downDir[0] = 0;
    pF0[2].downDir[1] = 0;
    pF0[2].downDir[2] = 0;

    pF0[3].forceType = F_SPRING;
    pF0[3].gravConst = gravity;
    pF0[3].downDir[0] = 0;
    pF0[3].downDir[1] = 0;
    pF0[3].downDir[2] = 0;
///spring stuff?

    // init the constraint-making objects:--------------------------------------
    wallCount = 6;          // set the number of constraints (4 walls)
    CWall * tmpWall;
    tmpWall = new CWall[wallCount];
    pC0 = tmpWall;          // address of fixed array of 4 axis-aligned walls.
    for(i=0; i< wallCount; i++)
        pC0[i].Kbouncy = 0.9;   // Coeff. of restitution for all 4 walls.
    // init each of these separate constraint-describing objects.
/*
    // Valid CWall::wallType values:
        //-------------------------------
        #define WTYPE_DEAD       0  // DEAD CONSTRAINT!!!  Abandoned, not in use, no
                                    // meaningful values, available for re-use.
        #define WTYPE_GROUND     1  // z=0 gnd-plane; Kbouncy=0; keep particle z>=0.
        #define WTYPE_XWALL_LO   2  // planar X wall; keeps particles >= xmin
        #define WTYPE_XWALL_HI   3  // planar X wall; keeps particles <= xmax
        #define WTYPE_YWALL_LO   4  // planar Y wall; keeps particles >= ymin
        #define WTYPE_YWALL_HI   5  // planar Y wall; keeps particles <= ymax
        #define WTYPE_ZWALL_LO   6  // planar Z wall; keeps particles >= zmin
        #define WTYPE_ZWALL_HI   7  // planar Z wall; keeps particles <= zmax
*/
    pC0[0].wallType = WTYPE_XWALL_LO;   // keeps particles >= xmin
    pC0[0].xmin = -100;
    // Right Wall:
    pC0[1].wallType = WTYPE_XWALL_HI;   // keeps particles <= xmax
    pC0[1].xmax =  100;
    // Bottom Wall:
    pC0[2].wallType = WTYPE_ZWALL_LO;   // keeps particles >= zmin
    pC0[2].zmin = -100;
    // Top Wall
    pC0[3].wallType = WTYPE_ZWALL_HI;   // keeps particles <= zmax
    pC0[3].zmax =  100;

    pC0[4].wallType = WTYPE_YWALL_LO;   // keeps particles >= zmin
    pC0[4].ymin = 0;

    pC0[5].wallType = WTYPE_YWALL_HI;   // keeps particles <= zmax
    pC0[5].ymax =  100;
}

void CPartSys::initExample2(void)
//------------------------------------------------------------------------------
// EXACTLY the same as 'initExample1', but with an Euler solver.
// Note how poorly it works -- the ball is excessively energetic, and it
// never stops bouncing!
{
    initExample1();         // repeat the same setup, BUT
    //bball.solverType = NU_SOLV_EULER;

}

void CPartSys::applyAllForces(CPart *pS, CForcer *pF)
//------------------------------------------------------------------------------
// Fill the 'force accumulators' in the state vector found at pS;
// find the total force applied to all the particles by all the forces in the
// list found at pF.
{
int i,j, myType;
GLdouble mag;

    // Begin by clearing the 'force accumulators' for each particle in state S:
   for(i=0; i< partCount; i++)
    {
        pS[i].xftot = 0.0;
        pS[i].yftot = 0.0;
        pS[i].zftot = 0.0;
    }
    if(forcerCount <= 0) cout << "!?!? No CForcer Objects!??" << endl;
    for(j=0; j<forcerCount; j++)
    {
        myType = pF[j].forceType;   // what kind of force?
        if(myType <= 0) continue;   // SKIP all 'dead' or disabled forces.
        switch(myType)              // select what kind of force to apply;
        {
        case F_GRAV_E:
            if (flocking == 1)
            {
                for (i=0; i<partCount; i++)
                {
                    for (int k=0; k<partCount; k++)
                    {
                        int xdiff = pS[i].xpos - pS[k].xpos;
                        int ydiff = pS[i].ypos - pS[k].ypos;
                        int zdiff = pS[i].zpos - pS[k].zpos;
                        //repulsive force
                        if(abs(xdiff) < 5 && abs(ydiff) < 5 && abs(zdiff) < 5 &&
                           abs(xdiff) != 0 && abs(ydiff) != 0 && abs(zdiff) != 0)
                        {
                            pS[i].xftot+=mag*xdiff;
                            pS[k].xftot-=mag*xdiff;

                            pS[i].yftot+=mag*ydiff;
                            pS[k].yftot-=mag*ydiff;

                            pS[i].zftot+=mag*zdiff;
                            pS[k].zftot+=mag*zdiff;
                        }
                        //attractive force
                        if(abs(xdiff) > 10 && abs(ydiff) > 10 && abs(zdiff) > 10)
                        {
                            pS[i].xftot-=mag*xdiff;
                            pS[k].xftot+=mag*xdiff;

                            pS[i].yftot-=mag*ydiff;
                            pS[k].yftot+=mag*ydiff;

                            pS[i].zftot-=mag*zdiff;
                            pS[k].zftot-=mag*zdiff;
                        }
                    }
                }
            }
            for(i=0; i<partCount; i++)    // for every particle,
        {
            // F=ma: gravity's accel is gravConst, so the
            // magnitude of gravity's force is (mass*gravConst);
            mag = pS[i].mass * pF[j].gravConst;
            // make a vector: scale our unit vector in the 'down' direction:
            pS[i].xftot += mag * pF[j].downDir[0];
            if( sqrt(pS[i].xpos * pS[i].xpos + pS[i].zpos * pS[i].zpos <= 3))
                    pS[i].yftot -= 5*mag * pF[j].downDir[1];
            pS[i].yftot += mag * pF[j].downDir[1];
            pS[i].zftot += mag * pF[j].downDir[2];
        }
        if(flocking)
            j=forcerCount;
            break;
        case F_WIND:
            for(i=0; i < partCount; i++)
            {
                mag = pS[i].mass;
                if (pS[i].xpos < -5)
                    pS[i].xftot += mag * 1;
                else if (pS[i].xpos > 5)
                    pS[i].xftot -= mag * 1;

                if (pS[i].ypos < -5)
                    pS[i].yftot += mag * 1;
                else if (pS[i].ypos > 10)
                    pS[i].yftot -= mag * 1;

                if (pS[i].zpos < -5)
                    pS[i].zftot += mag * 1;
                else if (pS[i].zpos > 5)
                    pS[i].zftot -= mag * 1;

                if(flocking)
                {
                    i+=partCount -1;
                }
            }
            break;
        case F_BUBBLE:
            for(i=0; i < partCount; i++)
            {
                mag = pS[i].mass;
                double x = pS[i].xpos;
                double y = pS[i].ypos;
                double z = pS[i].zpos;

                double distance = sqrt(z*z + x*x + y*y);
                if(distance <= 5) //distance >= -5) &&
                {
                    pS[i].xftot += mag * x/distance; //should be unitized
                    pS[i].yftot += mag * y/distance; //should be unitized
                    pS[i].zftot += mag * z/distance; //should be unitized
                }
            }
            if(flocking)
            {
                i+=partCount -1;
            }
            break;
        case F_SPRING:
            if (spring)
            {
                for(i=0; i < partCount/2; i+=2)
                {
                    mag = .9*pS[i].mass;
                    CPart * p1 = &pS[i];
                    CPart * p2 = &pS[i+1];
                    double mass1 = p1->mass;
                    double mass2 = p2->mass;

                    double distance = sqrt((p1->xpos - p2->xpos)*(p1->xpos - p2->xpos)+
                                           (p1->ypos - p2->ypos)*(p1->ypos - p2->ypos)+
                                           (p1->zpos - p2->zpos)*(p1->zpos - p2->zpos));
                    double restLength = 1;

                    if(distance > restLength)
                    {
                        pS[i].xftot -= mag * (p1->xpos-p2->xpos) * mass2;
                        pS[i].yftot -= mag * (p1->ypos-p2->ypos) * mass2;
                        pS[i].zftot -= mag * (p1->zpos-p2->zpos) * mass2;

                        pS[i+1].xftot += mag * (p2->xpos-p1->xpos) * mass1;
                        pS[i+1].yftot += mag * (p2->ypos-p1->ypos) * mass1;
                        pS[i+1].zftot += mag * (p2->zpos-p1->zpos) * mass1;
                    }
                    else if (restLength > distance)
                    {
                        p1->xftot -= mag * (p1->xpos-p2->xpos) * mass2;
                        p1->yftot -= mag * (p1->ypos-p2->ypos) * mass2;
                        p1->zftot -= mag * (p1->zpos-p2->zpos) * mass2;

                        p2->xftot += mag * (p2->xpos-p1->xpos) * mass1;
                        p2->yftot += mag * (p2->ypos-p1->ypos) * mass1;
                        p2->zftot += mag * (p2->zpos-p1->zpos) * mass1;
                    }

                    glColor3d(1,0,0);
                    glBegin(GL_LINES);
                    glVertex3f(pS0[i].xpos, pS0[i].ypos, pS0[i].zpos);
                    glVertex3f(pS0[i+1].xpos, pS0[i+1].ypos, pS0[i+1].zpos);
                    glEnd();
                }
            }
            else if(spring2)//draw a big spring
            {
                for (int i = 0; i < partCount-1; i++)
                {
                    mag = .9*pS[i].mass;
                    double x1 = pS[i].xpos;
                    double y1 = pS[i].ypos;
                    double z1 = pS[i].zpos;

                    double x2 = pS[i+1].xpos;
                    double y2 = pS[i+1].ypos;
                    double z2 = pS[i+1].zpos;

                    double distance = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2));
                    double restLength = 40;
                    if (distance > restLength)
                    {
                        pS[i].xftot += mag*(x1-x2);
                        pS[i].yftot += mag*(y1-y2);
                        pS[i].zftot += mag*(z1-z2);

                        pS[i+1].xftot -= mag*(x1-x2);
                        pS[i+1].yftot -= mag*(y1-y2);
                        pS[i+1].zftot -= mag*(z1-z2);
                    }
                    else if (distance < restLength)
                    {
                        pS[i].xftot -= mag*(x1-x2);
                        pS[i].yftot -= mag*(y1-y2);
                        pS[i].zftot -= mag*(z1-z2);

                        pS[i+1].xftot += mag*(x1-x2);
                        pS[i+1].yftot += mag*(y1-y2);
                        pS[i+1].zftot += mag*(z1-z2);
                    }
                    glColor3d(1,0,0);
                    glBegin(GL_LINES);
                    glVertex3f(pS0[i].xpos, pS0[i].ypos, pS0[i].zpos);
                    glVertex3f(pS0[i+1].xpos, pS0[i+1].ypos, pS0[i+1].zpos);
                    glEnd();
                }
            }
            else
                break;
            break;
        default:
                cout << "!?! CPartSys::applyAllForces(): Unknown force type: ";
                cout << myType << endl;
            break;
        }
    }
}

void CPartSys::dotMaker(CPart *pDotDest, CPart *pSrc, CForcer *pF, CWall *pC)
//------------------------------------------------------------------------------
// Find pDotDest, the time-derivative of the state vector pSrc. Time derivatives
// are the one thing we can ALWAYS find exactly for any given state of any given
// particle, because it is the direct result of Newton's Law (F=ma) and all the
// force-making objects in the list at pF (how many? CPartSys::forceCount).
// If we don't MOVE the particles (e.g. if we don't change pSrc) then we don't
// have any changes due to any constraint-making objects in the list at pC0.
{
int i;

    // Find the total of ALL forces on ALL particles in state vector pSrc;
    applyAllForces(pSrc, pF);
    //-------------------------------
    // Clear all params of all particles in the state vector at pDotDest.
    stateVecZero(pDotDest);
    // Now set any non-zero values:
    if(partCount<=0) cout << "!?!?No Particles?!?!" << endl;
    for(i=0; i<partCount; i++)    // for each particle in the state vector pSrc,
    {
        // Time-derivative of pSrc state's position is pDotDest's velocity;
        pDotDest[i].xpos = pSrc[i].xvel;    // Just copy!
        pDotDest[i].ypos = pSrc[i].yvel;
        pDotDest[i].zpos = pSrc[i].zvel;
        // Time-derivative of pSrc state's velocity is its acceleration;
        // Newton's Law says F=ma, so a = F/m. NU_EPSILON ensures nonzero mass.
        pDotDest[i].xvel = pSrc[i].xftot /
                          (NU_EPSILON + pSrc[i].mass);
        pDotDest[i].yvel = pSrc[i].yftot /
                          (NU_EPSILON + pSrc[i].mass);
        pDotDest[i].zvel = pSrc[i].zftot /
                          (NU_EPSILON + pSrc[i].mass);

        // What other state variables do we have left?
        // RECALL CPart members were mass,pos,vel, and force total (ftot)
        // we've set time-derivative for pos, vel;
        // let's keep zero value as time-derivative for mass and force-total.
    }
}

void CPartSys::dot2Maker(CPart *pDot2Dest, CPart *pSrc, CForcer *pF, CWall *pC)
//------------------------------------------------------------------------------
// Find pDotDest, 2nd time-derivative of the state vector pSrc. Time derivatives
// are the one thing we can ALWAYS find exactly for any given state of any given
// particle, because it is the direct result of Newton's Law (F=ma) and all the
// force-making objects in the list at pF (how many? CPartSys::forceCount).
// If we don't MOVE the particles (e.g. if we don't change pSrc) then we don't
// have any changes due to any constraint-making objects in the list at pC0.
{
int i;

    // Find the total of ALL forces on ALL particles in state vector pSrc;
    applyAllForces(pSrc, pF);
    //-------------------------------
    // Clear all params of all particles in the state vector at pDotDest.
    stateVecZero(pDot2Dest);
    // Now set any non-zero values:
    if(partCount<=0) cout << "!?!?No Particles?!?!" << endl;
    for(i=0; i<partCount; i++)    // for each particle in the state vector pSrc,
    {
        // 2nd time-derivative of pSrc state's position is its acceleration;
        // Newton's Law says F=ma, so a = F/m. NU_EPSILON ensures nonzero mass.
        pDot2Dest[i].xpos = pSrc[i].xftot /
                          (NU_EPSILON + pSrc[i].mass);
        pDot2Dest[i].ypos = pSrc[i].yftot /
                          (NU_EPSILON + pSrc[i].mass);
        pDot2Dest[i].zpos = pSrc[i].zftot /
                          (NU_EPSILON + pSrc[i].mass);

        // What other state variables do we have left?
        // RECALL CPart members were mass,pos,vel, and force total (ftot)
        // we've set 2nd time-derivative for pos.
        // let's keep zero value as 2nd time-derivative
        // for vel, mass and force-total.
    }
}

void CPartSys::render(CPart *pS, CForcer *pF, CWall *pC)
//------------------------------------------------------------------------------
// Depict on-screen the:
// --State vector at pS (a list of CPart objects. How many? partCount),
// --force-making objects at pF (list of CForcer objects. How many? forcerCount)
// --constraint objects at pC (list of CWall objects.  How many? wallCount).
{
    cout < "press h for the help menu.\n";
    int i;
    //cout << pS0[0].xpos << " " << pS0[0].ypos << " " << pS0[0].zpos << endl;
    // Draw all CPart particle objects in current state s0

    for(i=0; i<partCount; i++)      //------------------------------------------
    {
        glColor3f(0,1,0);       // bright yellow.
        glPointSize(5.0);               // draw 5x5 pixel 'points'
        glBegin(GL_POINTS);
            glVertex3d(pS0[i].xpos, pS0[i].ypos, pS0[i].zpos);
        glEnd();
       // glColor3d(1.0, 1.0, 1.0);       // Bright white
        //drawText3D(helv10, pS0[i].xpos-0.05, 0.0, pS0[i].zpos+0.05,"pS0");
    }
    // Draw all (visible) CForcer force-making objects in current list at pF0
    for(i=0; i<forcerCount; i++)    //------------------------------------------
    {
        pF0[i].drawMe();            // (for drawing springs, wind-lines, etc)
    }
    // Draw all (visible) CWall constraint-making objects in current list at pW0
    for(i=0; i<wallCount; i++)
    {
        pC0[i].drawMe();             // (for drawing walls, rods, cables, etc)
    }
}

void CPartSys::solver(void)
//------------------------------------------------------------------------------
// Use the selected solver type to find the next state for the particle system
// in s1; then swap s1 <-> s0 and swap s1dot <->s0dot.
{
int i;

    switch(solverType)
    {
        case NU_SOLV_AD_HOC:

        dotMaker(pS0dot, pS0, pF0, pC0);    // make s0dot
        dot2Maker(pS0dot2, pS0, pF0, pC0);  // make s0dot2
        stateVecAplusBtimesC(pS1, pS0dot, pS0dot2, 0.5*timeStep);
                                // make pS1 = (pS0dot + 0.5*h*pS0dot2)
        stateVecAplusBtimesC(pS1, pS0, pS1,timeStep);
                                // make pS1 = pS0 + h*(pS1), or
                                //          = pS0 + h*(s0dot + 0.5*h*s0dot2)
        // Now 'correct' s1 to include the damping term:
        for(i=0; i<partCount; i++)
        {
            pS1[i].xvel += (myDamp - 1.0)*pS0[i].xvel;
            pS1[i].yvel += (myDamp - 1.0)*pS0[i].yvel;
            pS1[i].zvel += (myDamp - 1.0)*pS0[i].zvel;
        }
        break;

        case NU_SOLV_MIDPOINT:          // Midpoint method (see lecture notes)
                                        // Euler method with half-timestep to
            stateVecZero(pSM);          // clear the midpoint state vector,
                                        // just to be SURE it's all zero.
            stateVecAplusBtimesC(pSM, pS0, pS0dot, timeStep/2); //Find midpoint.
            dotMaker(pSMdot,pSM, pF0, pC0);  // Find time-derivative at midpoint.
                                        // Use midpoint time derivative (pSMdot)
                                        // with full timeStep from s0 to find s1.
            stateVecAplusBtimesC(pS1, pS0, pSMdot, timeStep);

			break;
        case NU_SOLV_BACK_MIDPT:
            dotMaker(pS0dot,pS0,pF0,pC0);
            stateVecAplusBtimesC(pSM,pS0,pS0dot,timeStep);
            dotMaker(pSMdot,pSM,pF0,pC0);
            stateVecAplusBtimesC(pSerr,pS0dot,pSMdot,timeStep);
            stateVecZero(pSMdot);
            stateVecAplusBtimesC(pSerr,pSMdot,pSerr,timeStep);
            stateVecAplusBtimesC(pS1,pSM,pSerr,.5);
            break;
       // case NU_SOLV_ADAMS_BASH:        // Adams-Bashforth explicit integrator
        //case NU_SOLV_RUNGEKUTTA:        // Runga-kutta, degree set by solvDegree
        case NU_SOLV_VERLET:
            stateVecAplusBtimesC(pS1,pS0,pS0dot,timeStep);
            for(int i = 0; i < partCount; i++)
            {
                float magnitude = sqrt(pS1[i].xvel*pS1[i].xvel +
                                       pS1[i].yvel*pS1[i].yvel +
                                       pS1[i].zvel*pS1[i].zvel);
                pS1[i].xvel +=pS0dot[i].xvel/magnitude;
                pS1[i].yvel +=pS0dot[i].yvel/magnitude;
                pS1[i].zvel +=pS0dot[i].zvel/magnitude;
            }
            break;
             // Verlet semi-implicit integration
        //case NU_SOLV_VEL_VERLET:        // 'velocity-Verlet' semi-implicit integ.
            cout << "\n ERROR!  SOLVER NOT DONE YET: Defaulted to Euler Integ.\n" << endl;
            break;
        case NU_SOLV_EULER:             // Euler integration
                                        //  (explicit or forward integration)
        default:                        // (also our default solver)
            stateVecAplusBtimesC(pS1, pS0, pS0dot, timeStep);
                                        // s1 = s0 + s0dot*timestep
            break;
    }

}

void CPartSys::stateVecZero(CPart *pDest)
//------------------------------------------------------------------------------
// Clear the state vector at pDest; for all CPart particles in the list, set
// all parameters to zero.
{
int i;

    // Set s0dot vector to zero to ensure we have no 'stale' values.
    for(i=0; i<partCount; i++)
    {
        pDest[i].xpos = 0.0;     // AWKWARD! we have to set each param
        pDest[i].ypos = 0.0;
        pDest[i].zpos = 0.0;
                                    // by name.  In later version of CPart,
        pDest[i].xvel = 0.0;     // you can replace all these individually
        pDest[i].yvel = 0.0;
        pDest[i].zvel = 0.0;
                                // named params with a single array val[]
        pDest[i].mass = 0.0;     // and a set of #define directives that

        pDest[i].xftot = 0.0;    // assign meaning to each val[] element.
        pDest[i].yftot = 0.0;
        pDest[i].zftot = 0.0;
    }
}
void CPartSys::stateVecAplusBtimesC(CPart *pDest, CPart *pA, CPart *pB, double scl)
//------------------------------------------------------------------------------
// State Vector math:  dest = A + scl*B
// (OK for pDest == pA or pB)
{
int i;
    for(i=0; i< partCount; i++) // for all particles,
    {
        pDest[i].xpos = pA[i].xpos + scl*pB[i].xpos;
        pDest[i].ypos = pA[i].ypos + scl*pB[i].ypos;
        pDest[i].zpos = pA[i].zpos + scl*pB[i].zpos;
        pDest[i].xvel = pA[i].xvel + scl*pB[i].xvel;
        pDest[i].yvel = pA[i].yvel + scl*pB[i].yvel;
        pDest[i].zvel = pA[i].zvel + scl*pB[i].zvel;
        pDest[i].mass = pA[i].mass + scl*pB[i].mass;
    }
}

void CPartSys::doConstraints(CPart *pS_now, CPart *pS_prev,  CWall *pWsrc)
//------------------------------------------------------------------------------
// Apply all the constraints (CWall objects) in the list found at pW0 to the
// 'now' state-vector (CPart object list) at pS_now. Some constraints may need
// the previous state at pS_prev.
// Apply all the constraints (CWall objects) in the list at pW0 to the 'now'
// state-vector (CPart object list at pS_now). Some constraints will need the
// 'previous' state vector; supply it at pS_prev.
{
int i,j;

    if(wallCount <=0) cout << "!?!? No Constraints !?!" << endl;
    for(i=0; i<wallCount; i++)          // for every constraint-making object,
    {
        switch(pC0[i].wallType)
        {
        case WTYPE_XWALL_LO:            // LEFT WALL----------------------------
            for(j=0; j<partCount; j++)  // for every particle in pS_now list,
            {
                if(pS_now[j].xpos < pC0[i].xmin)    // IF particle passed through wall,
                {
                    pS_now[j].xpos = pC0[i].xmin;   // re-position it at wall surface,
                    if(pS_now[j].xvel < 0.0 )   // moving past wall? Bounce!
                    {
                        pS_now[j].xvel = -pC0[i].Kbouncy *
                                        0.5*(pS_now[j].xvel +pS_prev[j].xvel);
                        pS_now[j].yvel =  pC0[i].Kbouncy *
                                          0.5*(pS_now[j].yvel +pS_prev[j].yvel);
                        pS_now[j].zvel =  pC0[i].Kbouncy *
                                        0.5*(pS_now[j].zvel +pS_prev[j].zvel);
                    }
                }
            }
            break;
        case WTYPE_XWALL_HI:
            for(j=0; j<partCount; j++)  // for every particle in pS_now list,
            {
                if(pS_now[j].xpos > pC0[i].xmax)   // IF particle passed through wall,
                {
                    pS_now[j].xpos = pC0[i].xmax;  // re-position it at wall surface,
                    if(pS_now[j].xvel > 0.0 )   // moving past wall? Bounce!
                    {  // Apply this wall's coeff. of restitution to velocity's
                        pS_now[j].xvel = -pC0[i].Kbouncy *  // expected value:
                                          0.5*(pS_now[j].xvel +pS_prev[j].xvel);
                        pS_now[j].yvel =  pC0[i].Kbouncy *
                                          0.5*(pS_now[j].yvel +pS_prev[j].yvel);
                        pS_now[j].zvel =  pC0[i].Kbouncy *
                                          0.5*(pS_now[j].zvel +pS_prev[j].zvel);
                    }
                }
            }
            break;
        case WTYPE_YWALL_LO:
            for(j=0; j<partCount; j++)  // for every particle in pS_now list,
            {
                if(pS_now[j].ypos < pC0[i].ymin)   // IF particle passed through wall,
                {
                    pS_now[j].ypos = pC0[i].ymin;  // re-position it at wall surface,
                    if(pS_now[j].yvel < 0.0 )   // moving past wall? Bounce!
                    {   // Apply this wall's coeff. of restitution to velocity's
                        pS_now[j].xvel =  pC0[i].Kbouncy *  // expected value:
                                          0.5*(pS_now[j].xvel +pS_prev[j].xvel);
                        pS_now[j].yvel =  -pC0[i].Kbouncy *
                                          0.5*(pS_now[j].yvel +pS_prev[j].yvel);
                        pS_now[j].zvel = -pC0[i].Kbouncy *
                                          0.5*(pS_now[j].zvel +pS_prev[j].zvel);
                    }
                }
            }
            break;
        case WTYPE_YWALL_HI:
            for(j=0; j<partCount; j++)  // for every particle in pS_now list,
            {
                if(pS_now[j].ypos > pC0[i].ymax)   // IF particle passed through wall,
                {
                    pS_now[j].ypos = pC0[i].ymax;  // re-position it at wall surface,
                    if(pS_now[j].yvel > 0.0 )   // moving past wall? Bounce!
                    {   // Apply this wall's coeff. of restitution to velocity's
                        pS_now[j].xvel =  pC0[i].Kbouncy *  // expected value:
                                    //      0.5*(pS_now[j].xvel +pS_prev[j].xvel);
                                          pS_prev[j].xvel;
                        pS_now[j].yvel = -pC0[i].Kbouncy *
                                          0.5*(pS_now[j].yvel +pS_prev[j].yvel);
                        pS_now[j].zvel = pC0[i].Kbouncy *
                                    //      0.5*(pS_now[j].zvel +pS_prev[j].zvel);
                                          pS_prev[j].zvel;
                    }
                }
            }
            break;
        case WTYPE_ZWALL_LO:
            for(j=0; j<partCount; j++)  // for every particle in pS_now list,
            {
                if(pS_now[j].zpos < pC0[i].zmin)   // IF particle passed through wall,
                {
                    pS_now[j].zpos = pC0[i].zmin;  // re-position it at wall surface,
                    if(pS_now[j].zvel < 0.0 )   // moving past wall? Bounce!
                    {   // Apply this wall's coeff. of restitution to velocity's
                        pS_now[j].xvel =  pC0[i].Kbouncy *  // expected value:
                                          0.5*(pS_now[j].xvel +pS_prev[j].xvel);
                        pS_now[j].yvel =  pC0[i].Kbouncy *
                                          0.5*(pS_now[j].yvel +pS_prev[j].yvel);
                        pS_now[j].zvel = -pC0[i].Kbouncy *
                                          0.5*(pS_now[j].zvel +pS_prev[j].zvel);
                    }
                }
            }
            break;
        case WTYPE_ZWALL_HI:
            for(j=0; j<partCount; j++)  // for every particle in pS_now list,
            {
                if(pS_now[j].zpos > pC0[i].zmax)   // IF particle passed through wall,
                {
                    pS_now[j].zpos = pC0[i].zmax;  // re-position it at wall surface,
                    if(pS_now[j].zvel > 0.0 )   // moving past wall? Bounce!
                    {   // Apply this wall's coeff. of restitution to velocity's
                        pS_now[j].xvel =  pC0[i].Kbouncy *  // expected value:
                                    //      0.5*(pS_now[j].xvel +pS_prev[j].xvel);
                                          pS_prev[j].xvel;
                        pS_now[j].yvel = pC0[i].Kbouncy *
                                          0.5*(pS_now[j].yvel +pS_prev[j].yvel);
                        pS_now[j].zvel = -pC0[i].Kbouncy *
                                    //      0.5*(pS_now[j].zvel +pS_prev[j].zvel);
                                          pS_prev[j].zvel;
                    }
                }
            }
            break;
        default:
            cout << "!!Constraint # " << i<< ",wallType '" << pC0[i].wallType;
            cout <<"' not implemented!!"<< endl;
             break;
        }
    }
}

void CPartSys::stateVecSwap(CPart **ppA, CPart **ppB)
//------------------------------------------------------------------------------
// swap state-vector contents by swapping pointer values.
{
    CPart *pTmp;

    if(0==partCount || NULL==ppA[0] || NULL==ppB[0])
    {
        cout << "\nERROR!!! CPartSys::stateSwap() mismatched vectors!?!?\n" << endl;
        return;             // error exit
    }
    // Swap pointer values:
    pTmp = ppA[0];          // save A
    ppA[0] = ppB[0];        // A gets B
    ppB[0] = pTmp;           // B gets saved A.
}

//==========================
//
// CTheApp Member Functions
//
//==========================

CTheApp::CTheApp(void)
//------------------------------------------------------------------------------
// Default constructor: set up display window, mouse state, etc.
{

    window_width    = 640;	// OpenGL/GLUT display window position &size
    window_height   = 640;	// (set to initial values here)
    window_xpos     = 200;	// retrieve with getDisplayXXX() functions.
    window_ypos     = 100;

    anim_doMore = 1;        // ==1 to run animation, ==0 to pause.
                        // !!!DON'T MESS WITH anim_doMore; call runAnimTimer()!
    // Mouse state:
    xMclik=0.0; yMclik=0.0; // last mouse button down position, in pixels
    xMrot=0.0;  yMrot=0.0;  // mouse-driven rotation angles, in degrees.

}

CTheApp::~CTheApp(void)
//------------------------------------------------------------------------------
// Default destructor
{

}

void CTheApp::oglStartup(int argc, char *argv[])
//------------------------------------------------------------------------------
// Do all graphics startup: openGL/GLUT/GLM, etc;
// then enter GLUT's event-loop and stay there forever.
//  ***ALWAYS PUT THIS CALL AT THE VERY END OF MAIN()***
// because a call to oglStartup() will never return!
{
    //--GLUT/openGL Setup-------------------
	glutInit( &argc, argv );	    // init GLUT user-interface library;
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH  );
                    // set rendering mode to RGB (not RGBA) color,
					// single-buffering GLUT_SINGLE, or
					// double-buffering GLUT_DOUBLE, where you'd need to call
					// glutSwapBuffers() after drawing operations to show
					// on-screen what you draw)
	glutInitWindowSize(myApp.window_width, myApp.window_height);
    glutInitWindowPosition(myApp.window_xpos, myApp.window_ypos);
                    // set display window size & position from global vars
	glutCreateWindow( "EECS351-2 Project A" ); // set window title-bar name
    //-----------------------------------------
    //  !CAREFUL! Don't make ANY changes to OpenGL state, none! until AFTER you
    // call 'glutCreateWindow()', or you may get unpredictable results,
    // such as segmentation faults on Apple machines.
    //------------------------------------------
    glEnable(GL_DEPTH);             // Render with correct 3D occlusion
                                    // (Enables Z-buffer (disabled by default)

    if (debug == 1)
        cout << "registering callbacks\n";
	// Register our callback functions in GLUT:
    if (debug == 1)
        cout << "entering display\n";
	glutDisplayFunc( onDisplay );	// callback for display window redraw
    if (debug == 1)
        cout << "exiting display\n";
	if (debug == 1)
        cout << "entering reshape\n";
	glutReshapeFunc( onReshape);    // callback to create or re-size window
	if (debug == 1)
        cout << "exiting reshape and entering hidden\n";
	glutVisibilityFunc(onHidden);   // callback for display window cover/uncover
	if (debug == 1)
        cout << "exiting hidden and entering keyboard\n";
	glutKeyboardFunc( onKeyboard);  // callback for alpha-numeric keystroke
	glutSpecialFunc( onKeySpecial);// callback for all others: arrow keys,etc.
	glutMouseFunc( onMouseClik );   // callback for mouse button events
	glutMotionFunc( onMouseMove );  // callback for mouse dragging events
    runAnimTimer(1);                // start our animation loop.
    if (debug == 1)
        cout << "callbacks registered\n";

    if (debug == 1)
        cout << "entering main loop\n";
	glutMainLoop();// enter GLUT's event-handler; NEVER EXITS.
}


void groundPlane(GLdouble siz, GLdouble xygap)
{
glColor3d(1,1,1);
int ij, ijmax;

    if(xygap <= 0.0)
    {
        xygap = 1.0;
        cout <<"\n\n!!! drawGndPlane() defaulted to xygap=1.0!!!\n\n" << endl;
    }
    ijmax = (int)(2.0*siz / xygap);    // how many lines to draw.

    // Draw lines parallel to x axis;
    glBegin(GL_LINES);
        for(ij = 0; ij <= ijmax; ij++)
        {
            glVertex3d(-siz,0,-siz + ij*xygap);
            glVertex3d(siz,0,-siz + ij*xygap);
        }
    glEnd();
    // Draw lines parallel to y axis;
    glBegin(GL_LINES);
        for(ij = 0; ij <= ijmax; ij++)
        {
            glVertex3d(-siz + ij*xygap, 0, -siz);
            glVertex3d(-siz + ij*xygap, 0, siz);
        }
    glEnd();
}
