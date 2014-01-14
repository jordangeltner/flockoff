//===============  OpenGL and GLUT on *ANY* CodeBlocks platform  ===============
//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
// (J. Tumblin: I like consistent 80-char column width)
//
// How do I set up CodeBlocks, openGL and GLUT on my machine?
//	See instructions on Blackboard.
//
// multi-platform includes for the GLUT/OpenGL libraries:
#if defined(_WIN32)
#include <GL/freeglut.h>// open-source GLUT functions for windows, keybd, mouse
/*---*OR*--- use:
// #include <GL/glut.h> // for 100% backwards-compatibility with Mark Kilgard's
                        // SGI-copyrighted GLUT library. Careful! this excludes
                        // freeglut's callbacks for mouse wheel, etc.
*/
/*
// freeglut.h already contains these other includes that were necessary for
// using Kilgard's original GLUT library:
#include <GL/GL.h>      // OpenGL 3D graphics rendering functions
#include <GL/Glu.h>     // OpenGL utilties functions
#include <GL/glext.h>   // #defines for all OpenGL features on your machine.
*/
#elif defined(__APPLE__)
#include <GLUT/glut.h>  // GLUT functions for windows, keybd, mouse
#include <OpenGL/GL.h>  // OpenGL 3D graphics rendering functions
#include <OpenGL/Glu.h> // OpenGL utilties functions
#include <OpenGL/GLext.h>   // #defines for all OpenGL features on your machine.
// UNIX, Linux of all flavors;
#else
#include <unistd.h>         // defines symbolic constants, POSIX versions, etc.
#include <GL/glew.h>        // OpenGL 'extensions wrangler' for non-core fcns.
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#endif


//==============================================================================
#include <iostream>       // for cin, cout, etc.
#include <math.h>         // for sin, cos.

#define NU_PAUSE 10  // wait time (in milliseconds (mS)) that we use for GLUT's
                    // timer callback function between each screen redraw.
                    // Most PCs refresh their screens every 16.6mS (60Hz); we
                    // do nothing for NU_PAUSE mS to let the operating system
                    // work on other tasks; if we finish our re-drawing in
                    // the remaining time, we get smooth animation on-screen.
                    // If animation looks jumpy/irregular/jittery,
                    // shorten NU_PAUSE.
                    // Longer NU_PAUSE values (e.g. 300mS) slow the animation;
                    // use long values to see your animation run step-by-step.

#ifndef NU_EPSILON                  // Make sure it's defined everywhere:
#define NU_EPSILON 10E-15           // tiny amount; a minimum vector length
                                    // to use to avoid 'divide-by-zero'
#endif

// font strings for drawText2D() and drawText3D();
// (use as value for pFont argument)
//-------------------------------
#define rom10 GLUT_BITMAP_TIMES_ROMAN_10
#define rom24 GLUT_BITMAP_TIMES_ROMAN_24
#define helv10 GLUT_BITMAP_HELVETICA_10
#define helv12 GLUT_BITMAP_HELVETICA_12
#define helv18 GLUT_BITMAP_HELVETICA_18

/*------------------------------------------------------------------------------
C-style function prototypes for GLUT/freeGLUT callbacks.
------------------------------------------------------------------------------*/
void onReshape( int width, int height );
                            // Called if user opens or re-sizes display window.
void onDisplay(void);		// Called by GLUT when screen re-draw needed;
void onKeyboard(unsigned char key, int xw, int yw);
							// Called by GLUT when users press any alphabetic
							// or numerical 'key' on keyboard, and gives
							// current mouse position xw,yw in window pixels.
                            //  (most window systems put origin at UPPER left).
void onKeySpecial(int key, int xw, int yw);
							// Called by GLUT when users press 'arrow' keys,
							// 'Fcn' keys, or other non-alphanumeric keys;
							// xw,yw value gives mouse position (window pixels).
                            // (some window systems put origin at UPPER left).
							// Search glut.h for #define literals for special
							// keys such as GLUT_KEY_F1, GLUT_KEY_UP, etc.
void onMouseClik(int buttonID, int upDown, int xw, int yw);
                            // Called by GLUT on mouse-button click or un-click
                            // When user clicks a mouse button,
                            //  buttonID== 0 for left mouse button,
                            //		   (== 1 for middle mouse button?)
                            //			== 2 for right mouse button;
                            //	upDown  == 0 if mouse button was pressed down,
                            //	        == 1 if mouse button released.
                            //	xw,yw == mouse position in window-system pixels.
                            //  (most window systems put origin at UPPER left).
void onMouseMove(int xw, int yw);
							// Called by GLUT when user moves mouse while
							// while pressing one or more mouse buttons, giving
							// current mouse position xw,yw in window pixels.
                            //  (most window systems put origin at UPPER left).
//  ANIMATION FUNCTIONS
//=============================
// runAnimTimer(), onTimer(), and myHidden() functions work together to:
// 1) ensure this program continually re-draws the screen when running;
// 2) impose a delay of at least NU_PAUSE milliseconds between each re-draw,
//      to  a) let the operating system work on tasks other than our program,
//      and b) avoid revising screen contents more often than it gets displayed
//                  (typically every 16.6mSec -- 60Hz refresh).
// 3) stop any screen re-drawing when our display window isn't visible.
void runAnimTimer(int isOn);//  --to STOP/PAUSE animation,call runAnimTimer(0);
                            //  --to RESUME animation,    call runAnimTimer(1);
                            //  --to TOGGLE animation,    call runAnimTimer(2);
                            //  --to STEP animation by 1, call runAnimTimer(3);

void onTimer(int value);    // DON'T CALL THIS FUNCTION YOURSELF! Why? it is a
                            // callback function for GLUT/freeGLUT used only by
                            // the runAnimTimer() function to control animation.

void onHidden(int isVisible); // DON'T CALL THIS FUNCTION YOURSELF! Why? it is a
                            // callback function for GLUT/freeGLUT, called each
                            // time the OS tells us our on-screen display window
                            // was covered (visibility==0) or uncovered (==1)

//===============================================================================
// C-style function prototypes; better to use C++ classes for drawing.
// HINTS:
// --(Creating prototypes ing glutStart.h allows us to arrange the functions
//     bodies in glutStart.cpp in any order we wish.
// --(Please organize your own functions into C++ classes, not C as done here.)

// Read the current size and position of the graphics display window
int getDisplayHeight(void);
int getDisplayWidth(void);
int getDisplayXpos(void);
int getDisplayYpos(void);

//=============================================================================
// Handy drawing functions
void drawText3D(void *pFont,double x0,double y0,double z0, const char* pString);
                                        // Write text 'pString' on-screen at
                                        // position x0,y0 using current color
                                        // and coord system. Example:
                                        // drawText3D(helv18, 0.3,0.4,"Hello!");
void drawText2D(void *pFont, double x0, double y0, const char* pString);
                                        // Write text 'pString' on-screen in
                                        // white on top of any other contents in
                                        // the CVV; be sure x0,y0 fit into +/-1
void drawAxes(void);				// draw red x-axis, green yaxis, blue zaxis

//CPart class: describe one single particle
//==============================================================================
// TEMPORARILY SIMPLE!  see 'PartSysHeaderFiles' posted on Blackboard to see how
// to make much more flexible, useful particle classes.
class CPart
{
public:

    GLdouble xpos, ypos, zpos;        // position
    GLdouble xvel, yvel, zvel;        // velocity
    GLdouble mass;              // mass
    GLdouble xftot, yftot, zftot;      // force-accumulator
// AWKWARD! we have to set each param by name.  In later version of CPart,  you
// can replace all these individually named params with a single array val[]
// and a set of #define directives that assign meaning to each val[] element.

    CPart(void);                // default constructor, destructor
    ~CPart(void);
};

//---------------------------------
// Valid CForcer::forceType values:
//---------------------------------
#define F_NONE      0       // Non-existent force: ignore this CForcer object
#define F_MOUSE     1       // Spring-like connection to the mouse cursor; lets
                            // you 'grab' and 'wiggle' one particle(or several).
#define F_GRAV_E    2       // Earth-gravity: pulls all particles 'downward'.
#define F_GRAV_P    3       // Planetary-gravity; particle-pair (e0,e1) attract
                            // each other with force== grav* mass0*mass1/ dist^2
#define F_WIND      4       // Blowing-wind-like force-field;fcn of 3D position
#define F_BUBBLE    5       // Constant inward force towards centerpoint if
                            // particle is > max_radius away from centerpoint.
#define F_DRAG      6       // Viscous drag -- proportional to neg. velocity.
#define F_SPRING    7       // ties together 2 particles; distance sets force
#define F_SPRINGSET 8       // a big collection of identical springs; lets you
                            // make cloth & rubbery shapes as one force-making
                            // object, instead of many many F_SPRING objects.
#define F_CHARGE    9       // attract/repel by charge and inverse distance;
                            // applies to all charged particles.
#define F_MAXKINDS  10      // 'max' is always the LAST name in our list;
                            // gives the total number of choices for forces.

//CForcer class: describe one single force-making object
//==============================================================================
// TEMPORARILY SIMPLE! see 'PartSysHeaderFiles' posted on Blackboard to see how
// to make much more flexible, useful forcer classes.
class CForcer
{
    public:
    int forceType;          // sets the kind of force this object describes

    // F_GRAV_E  Earth Gravity variables........................................
    GLdouble gravConst;     // Gravitational constant: the acceleration rate in
                            // in the 'down' direction due to gravity.
    GLdouble downDir[3];    // the 'down' direction vector for gravity.

    CForcer(void);          // default constructor/destructor
    ~CForcer(void);
    void drawMe(void);      // draw object on-screen; useful for springs,
                            // for drawing wind-lines, etc.
};

//-------------------------------
// Valid CWall::wallType values:
//-------------------------------
#define WTYPE_DEAD       0  // DEAD CONSTRAINT!!!  Abandoned, not in use, no
                            // meaningful values, available for re-use.
#define WTYPE_GROUND     1  // y=0 ground-plane; Kbouncy=0; keeps particle y>=0.
#define WTYPE_XWALL_LO   2  // planar X wall; keeps particles >= xmin
#define WTYPE_XWALL_HI   3  // planar X wall; keeps particles <= xmax
#define WTYPE_YWALL_LO   4  // planar Y wall; keeps particles >= ymin
#define WTYPE_YWALL_HI   5  // planar Y wall; keeps particles <= ymax
#define WTYPE_ZWALL_LO   6  // planar Z wall; keeps particles >= zmin
#define WTYPE_ZWALL_HI   7  // planar Z wall; keeps particles <= zmax
#define WTYPE_MAXKINDS   8  // 'maxkinds' is always the LAST name in our list;
                            // gives total number of choices for constraints.

//CWall class: describe one single constraint-imposing object
//==============================================================================
class CWall
{
public:
    int wallType;           // Constraint type; not required, but a) helps you
                            // identify the intended purpose of each constraint,
                            // and b) gives you an easy way to enable/disable
                            // each constraint:
                            //  wallType  >0 == active constraint; use it!
                            //                   the value describes its use
                            //  wallType ==0 == 'dead' constraint, abandoned,
                            //                  ignored, available for re-use.
                            //  wallType  <0 == temporarily disabled 'frozen';
                            //                  to re-enable this particle,
                            //                  set wallType = -wallType;
    double Kbouncy;         // Coeff. of restoration for constraint surfaces:
                            // Particles moving at speed ||V|| will bounce off
                            // a constraint surface and lose some energy; after
                            // the bounce, speed is ||V||*Kbouncy.
                            //   0.0 <= Kbouncy <= 1.0;     'no bounce'== 0.0;
                            //                          'perfect bounce'==1.0.
    double xmin,xmax,ymin,ymax,zmin,zmax;   // min/max 3D positions

    CWall(void);                 // Default constructor/destructor
    ~CWall(void);
    void drawMe(void);      // Draw the constraint specified by wallType.
};

// Literals to select what kind of solver to use in CPartSys:
//------------------------------------------------------------------------------
// EXPLICIT methods: GOOD!
//          ++ simple, easy to understand, fast, but
//          -- Requires tiny time-steps for stable stiff systems, because
//          -- Errors tend to 'add energy' to any dynamical system, driving
//              many systems to instability even with small time-steps.
#define NU_SOLV_AD_HOC      0       // Ad-hoc state-space solver made to exactly
                                    // match 'spaghetti-code' solver method
                                    // we used in the early starter code
                                    // (2013_01glutBall version A,B,C).
#define NU_SOLV_EULER       1       // Euler integration: forward,explicit,...
#define NU_SOLV_MIDPOINT    2       // Midpoint Method (see Pixar Tutorial)
#define NU_SOLV_ADAMS_BASH  3       // Adams-Bashforth Explicit Integrator
#define NU_SOLV_RUNGEKUTTA  4       // Arbitrary degree, set by 'solvDegree'

// IMPLICIT methods:  BETTER!
//          ++Permits larger time-steps for stiff systems, but
//          --More complicated, slower, less intuitively obvious,
//          ++Errors tend to 'remove energy' (ghost friction; 'damping') that
//              aids stability even for large time-steps.
//          --requires root-finding (iterative: often no analytical soln exists)
#define NU_SOLV_BACK_EULER  5       // Iterative implicit solver;'back-winding'
#define NU_SOLV_BACK_MIDPT  6
// OR SEMI-IMPLICIT METHODS: BEST?
//          --Permits larger time-steps for stiff systems,
//          ++Simpler, easier-to-understand than Implicit methods
//          ++Errors tend to 'remove energy) (ghost friction; 'damping') that
//              aids stability even for large time-steps.
//          ++ DOES NOT require the root-finding of implicit methods,
#define NU_SOLV_VERLET      7       // Verlet semi-implicit integrator;
#define NU_SOLV_VEL_VERLET  8       // 'Velocity-Verlet'semi-implicit integrator
#define NU_SOLV_MAX         9       // number of solver types available.


// CPartSys class:
//==============================================================================
// A complete particle system, consisting of a list of particles, a list of
// forces, a list of constraints on those particles, a way to depict them, and
// a several plausible ways (solvers) that find how they change with time.
class CPartSys
{
public:
//-------State Vectors & Operators/Fcns-----------------------
    int partCount;                  // # of particles held in each state vector.
                                    // (if <=0, state vectors set to NULL)
    CPart *pS0, *pSM, *pSMdot, *pS0dot, *pS0dot2, *pSerr;  // s0: state vector holding CURRENT state of
                                    // particles, and its time derivative s0dot.
    CPart *pS1, *pS1dot;            // s1 state vector holding NEXT state, and
                                    // its time-derivative.

// CHEEZY TEMPORARY FIX TO AVOID DYN. ALLOC. for 1 particle:
CPart ballA, ballB;                 // we will point to these with ps0 and pS1
CPart ballAdot, ballBdot;           // and point to  these with pS0dot, pS1dot
CPart ballDot2;

    void stateVecSwap(CPart **ppA, CPart **ppB);
                                    // swap state-vector contents by
                                    // swapping pointer values.
    void stateVecZero(CPart *pDest);// Set to zero all params of all CPart
                                    // particles at the list that starts at pDest
    void stateVecAplusBtimesC(CPart *pDest, CPart *pA, CPart *pB, double scl);
                                                    // dest = A + scl*B

//-------Force-List Vector(s)----------------
    CForcer *pF0;                   // f0; forcer-vector-- dyn. alloc'd list of
                                    // all CURRENT force-making objects,
    // *pF0dot;                     //  and its time-derivative f0dot
                                    // (==NULL if none).
    GLdouble myDamp;                // ad-hoc solver's
                                    // velocity-damping (pg 58 of textbook).

    int forcerCount;                // # of forcer-making objects held in the
                                    // dyn. alloc'd list at pF0 (if needed, in
                                    // pF1, pFM, pF0dot,pF1dot,pFMdot as well).

//CHEEZY TEMPORARY FIX TO AVOID DYN. ALLOC. for 1 force-making object: gravity!
    //CForcer gravForce;

//-------Constraint-Making-Vectors(s)---------------
    int wallCount;                  //# of constraint-making objects (CWall obj)
                                    // held in the dyn. alloc'd list at pC0.
    CWall *pC0;                     // c0; constraint-vector--dyn. alloc'd list
                                    // of all CURRENT constraint-making objects,

// CHEEZY TEMPORARY FIX TO AVOID DYN> ALLOC. for 4 walls
    //CWall wallBox[4]; //left,right,top,bot walls

//----------------------------HIGH-LEVEL MEMBER FUNCTIONS:
    CPartSys(void);                 // Default constructor/destructor
    ~CPartSys(void);

    // --------- initialize: ------------------ Set up a particle system.

    void initExample1(void);        // one single 'bouncy ball' at 0,-0.1,0
                                    // with velocity (0.05, 0.0, 0.05) inside
                                    // a box of +/-0.9 meters centered at origin
                                    // that EXACTLY MATCHES the ad-hoc bouncy
                                    // ball we made in earlier.
                                    // Exact match required a rather quirky,
                                    // complicated solver.
    void initExample2(void);         // SAME as initExample1() but uses a simple
                                    // Euler solver. Note how poorly this works;
                                    // errors add energy to system, and the
                                    // ball never stops bouncing, even with
                                    // walls with low coeff. of restitution
                                    // (adjust it with the W and w keys)
    //----------- Run: ------------------------
    int solverType;                 // Type of solver to use: see #defines above
    GLdouble timeStep;              // Timestep used by the solver(); init'd
                                    // in the CPartSys constructor.
    void applyAllForces(CPart *pS, CForcer *pF);
                                    // Fill the 'force accumulators' in state
                                    // vector found at pS; find the total force
                                    // applied to all the particles by all the
                                    // forces in the CForcer list found at pF.
    void dotMaker(CPart *pDotDest, CPart *pSrc, CForcer *pF, CWall *pC);
                                    // Find pDotDest, the time-derivative of the
                                    // state vector pSrc.  Time derivatives are
                                    // the one thing we can ALWAYS find exactly
                                    // for any given state of any given particle,
                                    // because it is the direct result of 2nd
                                    // Newton's Law (F=ma) and all force-making
                                    // objects in the list at pF (how many?
                                    // see CPartSys::forceCount).
                                    //  If we don't MOVE the particles (e.g. if
                                    // we don't change pSrc) then we don't have
                                    // changes from constraint-making objects in
                                    // the list at pC0.
    void dot2Maker(CPart *pDot2Dest, CPart *pSrc, CForcer *pF, CWall *pC);
                                    // Find pDot2Dest, 2nd time-derivative of
                                    // state vector pSrc.  Time derivatives are
                                    // the one thing we can ALWAYS find exactly
                                    // for any given state of any given particle,
                                    // because it is the direct result of 2nd
                                    // Newton's Law (F=ma) and all force-making
                                    // objects in the list at pF (how many?
                                    // see CPartSys::forceCount).
                                    //  If we don't MOVE the particles (e.g. if
                                    // we don't change pSrc) then we don't have
                                    // changes from constraint-making objects in
                                    // the list at pC0.
    void doConstraints(CPart *pS_now, CPart *pS_prev,  CWall *pWsrc);
                                    // Apply all the constraints (CWall objects)
                                    // in the list found at pW0 to the 'now'
                                    // state-vector(CPart object list at pS_now).
                                    // Some constraints will need the previous
                                    // state vector; supply it at pS_prev.
    void render(CPart *pS, CForcer *pF, CWall *pC);
                                    // Depict the particle system on-screen with
                                    // state vector at pS, forces listed at pF,
                                    // and constraints listed at pC.
    //QUESTION: Why do dotMaker(),applyAllForces(),doConstraints(), render()
    // member functions have ANY arguments? Won't we always use dotMaker() to
    // find s0dot from s0? Won't we always use the forces at pF0, and the
    // constraints at pC0? Why not 'hard-code' those lists into the functions?
    // ANSWER: Flexibility!
    // This way we can apply these functions to ANY list of particles, using ANY
    // list of forces, and ANY list of constraints from ANY timestep.  This will
    // prove very useful later when we make more-sophisticated solvers for our
    // particle system, such as Iterative-Implicit Midpoint method, Verlet, etc.

    // To advance particle system to the next timestep, call the 'solver()'
    // function in your GLUT_registered onDisplay() callback, or use onIdle()
    // callback to control animation as I did here.
    void solver(void);              // Use the selected solver type to find the
                                    // next state for the particle system in s1;
                                    // then swap s1 <-> s0 and s1dot <->s0dot.
};

// CTheApp class:
//==============================================================================
// Collects together ALL of the global variables (bad idea) made necessary by
// GLUT/freeGLUT callbacks.  We make just one instance of this class, the
// global variable called 'myApp'.
// Also contains (as member functions) all the call
//Note that it might contain SEVERAL particle
// systems, such as the 4 you need for Project A...
class CTheApp
{
public:
    // Display window state:
int window_width;	        // OpenGL/GLUT display window position &size
int window_height;	        // (set to initial values here)
int window_xpos ;	        // retrieve with getDisplayXXX() functions.
int window_ypos;

// Mouse state:
GLdouble xMclik, yMclik;    // last mouse button down position, in pixels
GLdouble xMrot,  yMrot;     // mouse-driven rotation angles, in degrees.

                            // !!!DON'T MESS WITH THIS; call runAnimTimer()!
int anim_doMore;             // ==1 to run animation, ==0 to pause.

public:
    CTheApp(void);                   // Default constructor/destructor
    ~CTheApp(void);
void oglStartup(int argc, char *argv[]);
                            // Do all graphics startup: openGL/GLUT/GLM, etc;
                            // then enter GLUT's event-loop and stay there.
                            // !ALWAYS PUT THIS CALL AT THE VERY END OF MAIN()
                            // because call to oglStartup() will never return!
};

void groundPlane(GLdouble siz, GLdouble xygap);
