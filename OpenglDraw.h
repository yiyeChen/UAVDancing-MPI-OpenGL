/*
 * @Author: Yiye.Chen 
 * @Class: ECE6122
 * @Last Date Modified:   2019-11-30
 * 
 * Description:
 * 
 * The header file for OpenGL funcion. 
 * Declaring all functions needed for OpenGL drawing the scene
 */

/* Initialize texture */
void initTexture();
/* Window-size change callback */
void changeSize(int w, int h);
/* Draw the entire scene */
void renderScene();
/* Timer function. sleep for 100 msec */
void timerFunction(int id);
/* Idle function. For redisplay */
void update(void);
/* Keyboard func. For better observation */
void processNormalKeys(unsigned char key, int xx, int yy);

/* Draw football field */
void displayFootballField();
/* draw virtual floating ball */
void displayVirtualBall();
/* Draw UAVs */
void drawUAVs();

/* Change color */
void colorOsci();

/*  Main function for OpenGL */
void mainOpenGL(int argc, char**argv);