/*
 * @Author: Yiye.Chen 
 * @Class: ECE6122
 * @Last Date Modified:   2019-11-30
 * 
 * Description:
 *  
 * Defining yard to meter converting function.
 */

#include "utils.h"
/* Convert yard to meter

    Input: float yard - yard number
    Output: converted meter number
*/
float yard2Meter(float yard)
{
    return 0.9144 * yard;
}