/*
 * @Author: Yiye.Chen 
 * @Class: ECE6122
 * @Last Date Modified:   2019-11-30
 * 
 * Description:
 * 
 * The main file for this project. 
 * Containing main function, OpenGL main function, and drawing scene function
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "iomanip"
#include <cmath>
#include <math.h>
#include <cstdlib>
#include <GL/glut.h>
#include <chrono>
#include <thread>

#include "MPIComm.h"
#include "OpenglDraw.h"
#include "utils.h"

using namespace std;
using namespace std::chrono;

// field size
extern const float fieldHeight = yard2Meter(60);
extern const float fieldWidth = yard2Meter(125);
// number of UAV and number of Task
extern const int numUAV = 15;
extern const int numTask = 16;
// eye location
float eye_z = 75.0;
float eye_x = 0.0;
float eye_y = - 1.1*fieldHeight;
// rotate angle
int rotateAngle = 0;

// data type for MPI communication
MPI_Datatype MPI_StateType;
// array for collecting data
StateUAV* allStates = new StateUAV[numTask];


//----------------------------------------------------------------------
// mainOpenGL  - standard GLUT initializations and callbacks
// Input: argc - command line argument number
//        argv - command line argument
//----------------------------------------------------------------------
void mainOpenGL(int argc, char**argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(400, 400);

    glutCreateWindow("Final Project");
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0); // background color is black
    glShadeModel(GL_SMOOTH);    // use smooth shade mode
    glEnable(GL_COLOR_MATERIAL);   
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable( GL_BLEND );


    // initial texture
    initTexture();

    // Setup lights as needed
    // ...
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glutReshapeFunc(changeSize);
    glutDisplayFunc(renderScene);
    glutKeyboardFunc(processNormalKeys);
    //glutIdleFunc(update);
    glutTimerFunc(100, timerFunction, 0);
    glutMainLoop();
}

//----------------------------------------------------------------------
// Draw the entire scene
//
// First set the camera location based on its distance from the
// origin and its direction.
// Then draw football field, UAV, virtual ball separately
// Finaly use MPI to collect info
//----------------------------------------------------------------------
void renderScene()
{

    // Clear color and depth buffers
    glClearColor(0.0, 0.0, 0.0, 1.0); // change background color to black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset transformations
    glLoadIdentity();

    gluLookAt(eye_x, eye_y, eye_z, 
              0, 0, 20,
              0.0, 0.0, 1.0);

    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
        glRotatef(rotateAngle, 0, 0, 1.0);
        // display football field
        displayFootballField();

        //display virtual ball
        displayVirtualBall();

        // draw all uavs
        drawUAVs();
    glPopMatrix();

    glutSwapBuffers(); // Make it all visible

    // collect info
    StateUAV myState = allStates[0];
    MPI_Allgather(&myState, 1, MPI_StateType, 
                allStates, 1, MPI_StateType, 
                MPI_COMM_WORLD);
    // std::cout << "Position of UAV1" << allStates[1].X << ' ' 
    //                                 << allStates[1].Y << ' ' 
    //                                 << allStates[1].Z << endl;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Main entry point determines rank of the process and follows the 
// correct program path
// Input: argc - command line argument number
//        argv - command line argument
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int main(int argc, char**argv)

{
    // initialize MPI process
    int rc = MPI_Init(&argc, &argv);

    if (rc != MPI_SUCCESS) 
    {
        printf("Error starting MPI program. Terminating.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }
    /*------------------------Create a new MPI-type corresponding to StateUAV struct ------------------------------*/
    // data type of member in the struct
    MPI_Datatype type[10] = { MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, 
                            MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,
                            MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,
                            MPI_C_BOOL};
    // number of corresponding data type for each member
    int blockLen[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    // off set of each member in memory
    MPI_Aint disp[10];
    disp[0] = offsetof(StateUAV, X);//offset of member X
    disp[1] = offsetof(StateUAV, Y);//offset of member Y
    disp[2] = offsetof(StateUAV, Z);//offset of member Z
    disp[3] = offsetof(StateUAV, Vx);//offset of member Vx
    disp[4] = offsetof(StateUAV, Vy);//offset of member Vy
    disp[5] = offsetof(StateUAV, Vz);//offset of member Vz
    disp[6] = offsetof(StateUAV, Fx);//offset of member Fx
    disp[7] = offsetof(StateUAV, Fy);//offset of member Fy
    disp[8] = offsetof(StateUAV, Fz);//offset of member Fz
    disp[9] = offsetof(StateUAV, surfReach);//offset of member randRun
    // create a MPI struct type
    MPI_Type_create_struct(10, blockLen, disp, type, &MPI_StateType);
    // broadcast this type to all processors
    MPI_Type_commit(&MPI_StateType);

    /* ------------------------ Start Task ---------------------- */
    int rank; // rank of processor

    // state for each indvidual processor
    StateUAV myState;

    // initialize all states
    initStates();

    // // get the total number of tasks. 
    // MPI_Comm_size(MPI_COMM_WORLD, &numTask);
    // get the rank number
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // initialize mystate. Each processor takes its corresponding state
    myState = allStates[rank];

    if (rank == 0) 
    {
        // main processor draw the whole scene using OpenGL
        mainOpenGL(argc, argv);
    }
    else
    {
        // other processor controls a UAV separately

        // Sleep for 5 seconds
        std::this_thread::sleep_for(std::chrono::seconds(5));
        // start time
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        // end time
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        // time duration
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

        // only run 60 seconds
        while(time_span.count() <= 60.0)
        {
            // check for collision
            collisionCheck();
            // fetch state after collision
            myState = allStates[rank];
            // calculate state update
            calcualteUAVsLocation(rank, myState); 
            // share info
            MPI_Allgather(&myState, 1, MPI_StateType, 
                    allStates, 1, MPI_StateType, 
                    MPI_COMM_WORLD);

            // count time again
            t2 = high_resolution_clock::now();
            time_span = duration_cast<duration<double>>(t2 - t1);
        }
    }
    // end MPI process
    MPI_Finalize();
    return 0;
}