/*
 * @Author: Yiye.Chen 
 * @Class: ECE6122
 * @Last Date Modified:   2019-11-30
 * 
 * Description:
 *  
 * Defining data struct.
 * Declaring yard to meter converting function.
 */

// header include guard. Avoiding included for multiple time
#ifndef STATEUAV_H
#define STATEUAV_H

/* Data to be reported for each UAV.
    Including 3 position X,Y,Z, 3 velocity Vx, Vy, Vz, 
              3 forc Fx, Fy, Fz, and a flag indicates whether randomized the path or not
 */
struct StateUAV
{
    double X;
    double Y;
    double Z;
    double Vx;
    double Vy;
    double Vz;
    double Fx;
    double Fy;
    double Fz;
    bool surfReach;
};

/* yard to meter */
float yard2Meter(float yard);

#endif