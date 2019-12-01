/*
 * @Author: Yiye.Chen 
 * @Class: ECE6122
 * @Last Date Modified:   2019-11-30
 * 
 * Description:
 *  
 * MPI communication function declaring file
 * Declaring all functions for MPI communication
 */

#include "utils.h"

/* Initialize States for all UAVs */
void initStates();

/* Check the collision and whether need to swap speed or not */
void collisionCheck();

/* Calculate position */
void calcualteUAVsLocation(int& rank, StateUAV& myState); 

/* Decide engine force */
void decideForce(StateUAV& myState);

/* calculate location and speed according to force */
void calculateLocationSpeed(StateUAV& myState);

/* This function is to compute the distance according to coordinate 
    Input: StateUAV& state1 - the StateUAV of 1st UAV or virtual ball
            StateUAV& state2 - the StateUAV of 2nd UAV or virtual ball
    Output: distance between this two objects
*/
double dist(StateUAV& state1, StateUAV& state2);

/* L2 Norm. Calculate the L2 norm of a given vector
    Input: double x - vector's x coordinate
            double y - vector's y coordinate
            double z - vector's z coordinate
    Output: the norm of this vector
*/
double normL2(double x, double y, double z);