/*
 * @Author: Yiye.Chen 
 * @Class: ECE6122
 * @Last Date Modified:   2019-11-30
 * 
 * Description:
 *  
 * MPI communication function defining file
 * Define all functions for MPI communication
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cmath>
#include <math.h>
using namespace std;

#include "MPIComm.h"


// /* Declare global variable defined in main file */
extern StateUAV* allStates;
extern const int numTask;
const int mass = 1; // mass is 1kg
const double timeInterv = 0.1; //interval is 100 msec, 0.1sec


/* Initialize States for all UAVs 
    Input: StateUAV* allStates - a State List that contains all UAV's State
           int numUAV - number of UAVs
*/
void initStates()
{
    memset(allStates, 0, numTask * sizeof(allStates));
    // set 0th allState to the location of virtual ball
    allStates[0].X = 0;
    allStates[0].Y = 0;
    allStates[0].Z = 50;
    // set all UAV's initial position
    for (size_t i = 0; i < numTask-1; i++)
    {
        allStates[i+1].X = yard2Meter( -50.0 + 25.0 * (i % 5) );
        allStates[i+1].Y = yard2Meter( -25.0 + 25.0*(floor(i/5)) );
        //cout << allStates[i+1].X << ' ' << allStates[i+1].Y << endl;
    }
    return;
}

/* Check the collision and swap state is collision happens*/
void collisionCheck()
{
    for(int i=1; i < numTask - 1; i++)
    {
        for(int j = i + 1; j < numTask; j++)
        {
            if(dist(allStates[i], allStates[j]) < 0.01)
            {
                // cout << "collision happens! Distance between" 
                // << i << "and " << j << "equals" << dist(allStates[i], allStates[j]) << endl;
                StateUAV tem = allStates[j];
                allStates[j] = allStates[i];
                allStates[i] = tem;
            }
        }
    }
}

/* Calculate the position for each UAV
    Input: int& rank - the rank for this processor
            StateUAV& myState - reference of current UAV's state data
 */
void calcualteUAVsLocation(int& rank, StateUAV& myState)
{
    // change random seed
    srand(rank);

    // decide engine force
    decideForce(myState);

    // update speed and location
    calculateLocationSpeed(myState);

    return;
}


/* update speed and location according to the force

    Input: StateUAV& myState - reference of current UAV's state data
*/
void calculateLocationSpeed(StateUAV& myState)
{
    /* Update the location and speed according to the forces
        Using Newton second law to determine the acceleration.
        And then use equation of motion to update loc & speed
     */

    // update acceleration. F = am
    // z-direction need to minus gravity
    double aX, aY, aZ;
    aX = myState.Fx / mass;
    aY = myState.Fy / mass;
    aZ = (myState.Fz - mass * 10) / mass; // minus gravity


    // update speed. V = V0 + at
    double Vx_new, Vy_new, Vz_new;
    Vx_new = myState.Vx + aX * timeInterv;
    Vy_new = myState.Vy + aY * timeInterv;
    Vz_new = myState.Vz + aZ * timeInterv;

    // update location. x = x0 + v0t + 1/2 a*t^2
    double X_new, Y_new, Z_new;
    X_new = myState.X + myState.Vx * timeInterv + 1/2 * aX * timeInterv * timeInterv;
    Y_new = myState.Y + myState.Vy * timeInterv+ 1/2 * aY * timeInterv * timeInterv;
    Z_new = myState.Z + myState.Vz * timeInterv + 1/2 * aZ * timeInterv * timeInterv;

    // store new information. Velocity and position
    myState.Vx = Vx_new;
    myState.Vy = Vy_new;
    myState.Vz = Vz_new;
    myState.X = X_new;
    myState.Y = Y_new;
    myState.Z = Z_new;

}

/* decide engine force 
    Input: StateUAV& myState - the State data of a particular UAV
*/
void decideForce(StateUAV& myState)
{
    // initial force to zero
    myState.Fx = 0;
    myState.Fy = 0;
    myState.Fz = 0;
    // retrieve virtual ball's position
    StateUAV ballState = allStates[0];
    
    /* --------------------------------Decide force------------------
        only allow 10 N for expedition, another 10 N for offset gravity
     */

    // force direction
    double unitForceX = 0.0;
    double unitForceY = 0.0;
    double unitForceZ = 0.0;
    // force magnitude
    double forceMag = 0.0;
    // distance between virtual ball and UAV
    double distBallUAV = dist(myState, ballState);

    if (distBallUAV > 10.0 && !myState.surfReach)
    {
        // ---------------------Stage 1 -------------------------
        // if has never reach the ball, and is still far,
        // exert the force directed to ball's center, and force speed to 1.8 m/s

        // Direction Of Force: point to ball center, normalized
        unitForceX = (ballState.X - myState.X) / distBallUAV;
        unitForceY = (ballState.Y - myState.Y) / distBallUAV;
        unitForceZ = (ballState.Z - myState.Z) / distBallUAV;

        // Magnitute Of Force: urge the speed to be 1.8 m/s (<2)
        forceMag = (1.8 - normL2(myState.Vx, myState.Vy, myState.Vz)) / timeInterv * mass;
        
        // combine force magnitute and direction
        myState.Fx = unitForceX * forceMag;
        myState.Fy = unitForceY * forceMag;
        myState.Fz = unitForceZ * forceMag; 

    }
    else if (distBallUAV <= 10 && !myState.surfReach)
    {
        // ---------------------Stage 2 -------------------------
        // if 1st time reach the ball, add a random direction force tangent to the ball

        // set surfReach flag to 1
        myState.surfReach = true;
        // get relative location
        double deltaX = ballState.X - myState.X;
        double deltaY = ballState.Y - myState.Y;
        double deltaZ = ballState.Z - myState.Z;

        // Direction Of Force: tangent to ball, normalized
        unitForceX = rand() % 5 - 2;
        unitForceY = rand() % 5 - 2;
        unitForceZ = - ( unitForceX * deltaX - unitForceY * deltaY ) / deltaZ;
        // normalized
        double normForce = normL2(unitForceX, unitForceY, unitForceZ);
        unitForceX = unitForceX / normForce;
        unitForceY = unitForceY / normForce;
        unitForceZ = unitForceZ / normForce;

        //Magnitude of Force: 2 times current speed
        forceMag = 2 * normL2(myState.Vx, myState.Vy, myState.Vz);

        // combine force magnitute and direction
        myState.Fx = unitForceX * forceMag;
        myState.Fy = unitForceY * forceMag;
        myState.Fz = unitForceZ * forceMag; 

    } 
    else
    {
        // ---------------------Stage 3 -------------------------
        // if already reach the ball, maintain staying on ball, and keep speed in [2, 10]
        
        double forceMag1, forceMag2;
        
        // compute speed
        double speed = normL2(myState.Vx, myState.Vy, myState.Vz);

        // Direction Of Force1: point to ball center, normalized
        double unitForceX1 = (ballState.X - myState.X) / distBallUAV;
        double unitForceY1 = (ballState.Y - myState.Y) / distBallUAV;
        double unitForceZ1 = (ballState.Z - myState.Z) / distBallUAV;

        // decompose speed to toward ball proportion, and tangent proportion
        double toBallSpeed = myState.Vx * unitForceX1 
                            + myState.Vy * unitForceY1 
                            + myState.Vz * unitForceZ1;

        // tangent velocity and speed:
        double tangentVx = myState.Vx - toBallSpeed * unitForceX1;
        double tangentVy = myState.Vy - toBallSpeed * unitForceY1;
        double tangentVz = myState.Vz - toBallSpeed * unitForceZ1;

        double tangentSpeed = normL2(tangentVx, tangentVy, tangentVz);

        // Direction of Force2: along the tangent speed direction
        double unitForceX2 = tangentVx / tangentSpeed;
        double unitForceY2 = tangentVy / tangentSpeed;
        double unitForceZ2 = tangentVz /tangentSpeed;

        // Magnitude of Force1 (PD): 
        forceMag1 = 2.0 * (distBallUAV - 10) 
                    - 5.0 * toBallSpeed;
        
        // Magnitude of Force2:
        forceMag2 = 0.05 * (6.0 - tangentSpeed);

        // combine force magnitute and direction
        myState.Fx = unitForceX1 * forceMag1 +  unitForceX2 * forceMag2;
        myState.Fy = unitForceY1 * forceMag1 +  unitForceY2 * forceMag2;
        myState.Fz = unitForceZ1 * forceMag1 +  unitForceZ2 * forceMag2;
        

    }

    /* If previous force exceeds 10N, scale down to 10 N */
    forceMag = normL2(myState.Fx, myState.Fy, myState.Fz);
    if (forceMag > 10)
    {
        myState.Fx = myState.Fx / forceMag * 10;
        myState.Fy = myState.Fy / forceMag * 10;
        myState.Fz = myState.Fz / forceMag * 10;
    }
    
    /* ------------- Add 10 N z-direction force to off-set gravity ---------------- */
    myState.Fz = myState.Fz + 10.0;
}



/* This function is to compute the distance according to coordinate 
    Input: StateUAV& state1 - the StateUAV of 1st UAV or virtual ball
            StateUAV& state2 - the StateUAV of 2nd UAV or virtual ball
    Output: distance between this two objects
*/
double dist(StateUAV& state1, StateUAV& state2)
{
    return normL2(state1.X-state2.X, 
                    state1.Y-state2.Y,
                    state1.Z-state2.Z);
}

/* L2 Norm. Calculate the L2 norm of a given vector
    Input: double x - vector's x coordinate
            double y - vector's y coordinate
            double z - vector's z coordinate
    Output: the norm of this vector
*/
double normL2(double x, double y, double z)
{
    return sqrt(x * x + y * y + z * z);
}
