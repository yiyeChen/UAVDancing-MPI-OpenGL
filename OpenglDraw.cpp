/*
 * @Author: Yiye.Chen 
 * @Class: ECE6122
 * @Last Date Modified:   2019-11-30
 * 
 * Description:
 *  
 * OpenGL function defining file.
 * Define all functions needed for OpenGL drawing the scene
 */

#include <GL/glut.h>
#include <chrono>
#include <iostream>
#include "ECE_Bitmap.h"
using namespace std;

#include "OpenglDraw.h"
#include "utils.h"

extern StateUAV* allStates;
extern const int numUAV;
extern const int numTask;
extern float eye_x;
extern float eye_y;
extern float eye_z;
extern int rotateAngle;

// color for UAV
float colorUnNorm = 255.0;
// color change direction
bool colorChangeDirect = true; // true: color increase; false: color decrease

extern const float fieldHeight;
extern const float fieldWidth;

// data for texture
// read in bitmap
BMP inBitmap;
// store texture 
GLuint texture;


/* setup the texture.
    Read in the image, and create a corresponding texture
 */
void initTexture()
{
    // read image
    inBitmap.read("AmFBfield.bmp");

    // Create Textures
    glGenTextures(1, &texture);
    
    // Setup first texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture

    // bind to image
    glTexImage2D(GL_TEXTURE_2D, 0, 3, inBitmap.bmp_info_header.width, inBitmap.bmp_info_header.height, 0,
        GL_BGR_EXT, GL_UNSIGNED_BYTE, &inBitmap.data[0]);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

/* Reshape callback

    Set perspective angle to 80 degrees, ratio equals to window size ration
    Input: w - window width
            h - window height
 */
void changeSize(int w, int h)
{
    float ratio = ((float)w) / ((float)h); // window aspect ratio
    glMatrixMode(GL_PROJECTION); // projection matrix is active
    glLoadIdentity(); // reset the projection
    gluPerspective(80.0, ratio, 0.1, 1000.0); // perspective transformation
    glMatrixMode(GL_MODELVIEW); // return to modelview mode
    glViewport(0, 0, w, h); // set viewport (drawing area) to entire window
}


/* Draw football field 

    Draw a square field, whose center locates at original point.
    Turn on the texture
*/
void displayFootballField()
{
    glColor3f(0.0, 1.0, 0.0);

    // enable texute
    glEnable(GL_TEXTURE_2D);

    // specify texture
    glBindTexture(GL_TEXTURE_2D, texture);

    // draw the quad, corresponding to texture space point
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(-fieldWidth/2, -fieldHeight/2, 0.0);
        glTexCoord2f(1, 0);
        glVertex3f(fieldWidth/2, -fieldHeight/2, 0.0);
        glTexCoord2f(1, 1);
        glVertex3f(fieldWidth/2, fieldHeight/2, 0.0);
        glTexCoord2f(0, 1);
        glVertex3f(-fieldWidth/2, fieldHeight/2, 0.0);
    glEnd();

    // disable texure, or other thing will be add on this texture too
    glDisable(GL_TEXTURE_2D);
}

/* Draw virtual floating ball 

    place the ball 50m above ground, with r = 10.
    set it to white color, and has a alpha number for color
*/
void displayVirtualBall()
{
    glColor4f(1.0, 1.0, 1.0, 0.25); // set color to white
    glPushMatrix();
        glTranslatef(0.0, 0.0, 50.0); // move up 50 meter
        glutWireSphere(10.0, 15, 15);
    glPopMatrix();
}

/*  Change color 
    Oscillate UAV colors from 255 to 128, and then back to 255
*/
void colorOsci()
{
    // if reach border, switch color change direction
    if (colorUnNorm < 128.5 || colorUnNorm > 254.5)
    {
        colorChangeDirect = !colorChangeDirect;
    }
    // if colorChangeDirect is increase, increase
    if(colorChangeDirect)
    {
        colorUnNorm += 1.0;
    }
    // if colorChangeDirect is decrease, decrease
    else
    {
        colorUnNorm -= 1.0;
    }
    
}

/* Draw all UAVs 

    Go over all location and draw a octahedron. 
    Set color to blue, intensity according to oscillate color, and scale to reasonable size
*/
void drawUAVs()
{
    glColor3f(0.0, 0.0, colorUnNorm / 255); //set color to blue. Normalize to [0, 1]
    // draw each uav
    for(int i=1; i< numTask; i++)
    {
        glPushMatrix();
            // move to corresponding position
            glTranslatef(allStates[i].X, allStates[i].Y, allStates[i].Z);
            // scale the size. Defaulst size is r=1, d=2
            float scaleFactor = 1.2;
            glScalef(scaleFactor, scaleFactor, scaleFactor);
            // draw octahedron
            glutSolidOctahedron();
        glPopMatrix();
    }
    return;
}

//----------------------------------------------------------------------
// timerFunction  - called whenever the timer fires
//
// for each time step, change the color, and redisplay the whole scene(with updated data)
// Input: int id - the id number for this processor, 
//                  which also indicates which UAV to control for this processor
//----------------------------------------------------------------------
void timerFunction(int id)
{
    // change color
    colorOsci();
    // fire after 100 msec
    glutPostRedisplay();
    // restart timer
    glutTimerFunc(100, timerFunction, 0);
}

/*  Only for testing.
    Allow changing sight for better observation

    Input: key - the key that pressed
            xx - location of mouse in X direction
            yy - location of mouse in Y direction
 */
void processNormalKeys(unsigned char key, int xx, int yy)
{
    /* Process keyboard command
     Input: key - key pressed
            xx - mouse x position 
            yy - mouse y position
     */
    switch (key)
    {
    // rotate the chess set
    case 'r':
    case 'R':
        rotateAngle = (rotateAngle + 10) % 360;
        break;
    // rotate the chess set
    case 't':
    case 'T':
        rotateAngle = (rotateAngle - 10) % 360;
        break;
    // eye loaction down
    case 'd':
    case 'D':
        eye_z = eye_z - 0.25;
        break;
    // eye location up
    case 'u':
    case 'U':
        eye_z = eye_z + 0.25;
        break;
    // // turn on or off light0
    // case '0':
    //     light0On = !light0On;
    //     break;
    // // turn on or off light1
    // case '1':
    //     light1On = !light1On;
    //     break;
    // // move pawn
    // case 'p':
    // case 'P':
    //     movePawn();
    //     break;
    // // move knights
    // case 'k':
    // case 'K':
    //     moveKnight();
    //     break;
    // // enhance mode
    // case 'E':
    // case 'e':
    //     enhance = !enhance;
    //     break;
    }
}