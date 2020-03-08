/*
 * This is a very basic Windows C application for testing GLUT (and compatible
 * implementations such as freeglut). It displays a red square, and exits when
 * the escape key is pressed.
 */

#include <stdlib.h>
#include <GL/glut.h>

/* Keyboard callback function */
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        /* Exit on escape key press */
    case '\x1B':
    {
        exit(EXIT_SUCCESS);
        break;
    }
    }
}

/* Display callback function */
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    /* Display a red square */
    glColor3f(1.0f, 0.0f, 0.0f);

    glBegin(GL_POLYGON);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f(0.5f, -0.5f);
    glVertex2f(0.5f, 0.5f);
    glVertex2f(-0.5f, 0.5f);
    glEnd();

    glFlush();
}

/* Main method */
int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    /* Create a single window with a keyboard and display callback */
    glutCreateWindow("GLUT Test");
    glutKeyboardFunc(&keyboard);
    glutDisplayFunc(&display);

    /* Run the GLUT event loop */
    glutMainLoop();

    return EXIT_SUCCESS;
}