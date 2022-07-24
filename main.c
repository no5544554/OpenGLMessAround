#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define DEG_TO_RAD 0.01745329
#define NORMALIZED 0.707107

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define FPS 50

#define ESCAPE_KEY 27

#define CHECKERBOARD_WIDTH  64
#define CHECKERBOARD_HEIGHT 64

#define GRAVITY 0.003f

#define PLAYER_SPEED 0.1f
#define PLAYER_TURN_SPEED 2.0f
#define PLAYER_JUMP_SPEED 0.05f



static GLubyte checkerBoard[CHECKERBOARD_WIDTH * CHECKERBOARD_HEIGHT][4];


/* TODO: Turn into a struct */
float px  = 0.0f;
float pz  = 0.0f;
float py  = 0.0f;
float pa  = 0.0f;
float pav = 0.0f;
float ph = 1.0f;
float pvspeed = 0.0f;

BOOL keys[256];
BOOL keysPrev[256];
BOOL leftArrow, rightArrow, upArrow, downArrow;

BOOL toggleJump = TRUE;

static GLuint texture;

struct
{
    int start;
    int end;
} timer;

int window_ID;


/**************************
 * Function declarations
 **************************/
void MakeCheckerboard(void);
void DrawWorld(void);
void Update(void);

void Display(void);
void RunMainLoop(int val);
void KeyboardDown(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);
void SpecialDown(int key, int x, int y);
void SpecialUp(int key, int x, int y);

void CreateLight(void);

void DrawSky(void);
void DrawGrid(void);
void DrawCylinder(void);
void DrawGun(void);
void DrawCube(void);


void MakeCheckerboard(void)
{
    int x, y, c;
    for (y = 0; y < CHECKERBOARD_HEIGHT; y++)
    {
        for (x = 0; x < CHECKERBOARD_WIDTH; x++)
        {
            c = ((((y&0x8)==0)^(((x&0x8))==0))*255);
            checkerBoard[y * CHECKERBOARD_WIDTH + x][0] = (GLubyte) c;
            checkerBoard[y * CHECKERBOARD_WIDTH + x][1] = (GLubyte) c;
            checkerBoard[y * CHECKERBOARD_WIDTH + x][2] = (GLubyte) c;
            checkerBoard[y * CHECKERBOARD_WIDTH + x][3] = (GLubyte) 255;
        }
    }
}


void DrawWorld(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 640, 480, 0);

    /* Draw the sky */
    glDisable(GL_LIGHTING);
    DrawSky();


    /* Draw the world */
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(pav, 1.0f, 0.0f, 0.0f);
    glRotatef(pa, 0.0f, 1.0f, 0.0f);
    glTranslatef(-px, (-py) - ph, -pz);

    /* Draw the ground*/
    glColor3f(0.2f, 0.5f, 0.1f);
    glPushMatrix();
    glBegin(GL_QUADS);
    glVertex3f(-50, 0, 50);
    glVertex3f(50, 0, 50);
    glVertex3f(50, 0, -50);
    glVertex3f(-50, 0, -50);
    glEnd();
    glPopMatrix();
    glEnable(GL_LIGHTING);

    /* Draw Grid */
    DrawGrid();

    /* Draw brown cylinder */
    DrawCylinder();

    /* Draw checkered cube */
    DrawCube();

    /* Draw "gun" cylinder */
    DrawGun();

    /* Flush */
    glFlush();
}


void Update(void)
{
    double normalized = 0.2;


    if (toggleJump)
    {
        if (py > 0)
        {
            pvspeed -= GRAVITY;
            if (pvspeed < -0.2f) pvspeed = -0.2f;
        }
        else
        {
            pvspeed = 0;
            if (keys[' '])
            {
                py = PLAYER_JUMP_SPEED;
                pvspeed = PLAYER_JUMP_SPEED;
            }
        }

        py += pvspeed;
    }



    if ((keys['d'] || keys['a']) && (keys['w'] || keys['s']))
    {
        normalized = PLAYER_SPEED * NORMALIZED;
    }
    else
    {
        normalized = PLAYER_SPEED;
    }

    if (keys['d'])
    {
        px += normalized * cos(pa * DEG_TO_RAD);
        pz += normalized * sin(pa * DEG_TO_RAD);
    }
    else if (keys['a'])
    {
        px -= normalized * cos(pa * DEG_TO_RAD);
        pz -= normalized * sin(pa * DEG_TO_RAD);
    }

    if (keys['w'])
    {
        px += normalized * sin(pa * DEG_TO_RAD);
        pz -= normalized * cos(pa * DEG_TO_RAD);
    }
    else if (keys['s'])
    {
        px -= normalized * sin(pa * DEG_TO_RAD);
        pz += normalized * cos(pa * DEG_TO_RAD);
    }

    if (!toggleJump)
    {
        if (keys['e'])
        {
            py += 0.2f;
        }
        else if (keys['q'])
        {
            py -= 0.2f;
        }
    }

    if (keys['t'] && !keysPrev['t'])
    {
        toggleJump = !toggleJump;
    }


    if (rightArrow)
    {
        pa += 2.0f;
        if (pa > 360) pa -= 360;
    }
    else if (leftArrow)
    {
        pa -= 2.0f;
        if (pa < 0) pa += 360;
    }
    if (upArrow)
    {
        pav -= 2.0f;
        if (pav < -90) pav = -90;
    }
    else if (downArrow)
    {
        pav += 2.0f;
        if (pav > 90) pav = 90;
    }

    if (keys[ESCAPE_KEY])
    {
        glutLeaveMainLoop();
    }

    memcpy(keysPrev, keys, sizeof(keys));
}


void Display(void)
{

    if (timer.start - timer.end >= (1000 / FPS))
    {
        Update();
        DrawWorld();

        timer.end = timer.start;

        glutSwapBuffers();
    }

    timer.start = glutGet(GLUT_ELAPSED_TIME);
    glutPostRedisplay();


}


void RunMainLoop(int val)
{

}


void KeyboardDown(unsigned char key, int x, int y)
{
    keys[key] = TRUE;
}


void KeyboardUp(unsigned char key, int x, int y)
{
    keys[key] = FALSE;
}


void SpecialDown(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT)   leftArrow  = TRUE;
    if (key == GLUT_KEY_RIGHT)  rightArrow = TRUE;
    if (key == GLUT_KEY_UP)     upArrow    = TRUE;
    if (key == GLUT_KEY_DOWN)   downArrow  = TRUE;


}


void SpecialUp(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT)   leftArrow  = FALSE;
    if (key == GLUT_KEY_RIGHT)  rightArrow = FALSE;
    if (key == GLUT_KEY_UP)     upArrow    = FALSE;
    if (key == GLUT_KEY_DOWN)   downArrow  = FALSE;
}


void MouseMove(int x, int y)
{
    int centerX, centerY;
    int dx, dy;

    centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    dx = x - centerX;
    dy = y - centerY;

    if (dx != 0 || dy != 0)
    {
        pa += dx * 0.2f;
        pav += dy * 0.2f;

        if (pav < -90) pav = -90;
        if (pav > 90)  pav = 90;

        glutWarpPointer(centerX, centerY);
    }
}


int main(int argc, char * argv[])
{
    glutInit(&argc, argv);
    glutInitContextVersion(2, 1);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    window_ID = glutCreateWindow("OpenGL");
    glutSetCursor(GLUT_CURSOR_NONE);


    /* Create texture */
    MakeCheckerboard();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERBOARD_WIDTH, CHECKERBOARD_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerBoard);

    /* set projection */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT, 0.5, 50.0);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glShadeModel(GL_SMOOTH);

    CreateLight();


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
    glutDisplayFunc(Display);
    glutTimerFunc(1000 / FPS, RunMainLoop, 0);
    glutKeyboardFunc(KeyboardDown);
    glutKeyboardUpFunc(KeyboardUp);
    glutSpecialFunc(SpecialDown);
    glutSpecialUpFunc(SpecialUp);
    glutPassiveMotionFunc(MouseMove);
    glutMainLoop();
    return 0;
}


void CreateLight(void)
{
    GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 1.0, 10.0, 1.0, 0.0 };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHT0);
}


void DrawSky(void)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(0.3f, 0.7f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(0, 480);
    glVertex2f(640, 480);
    glVertex2f(640, 0);
    glVertex2f(0, 0);
    glEnd();

    /*
    glColor3f(0.2f, 0.5f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0, 480);
    glVertex2f(640, 480);
    glVertex2f(640, 240);
    glVertex2f(0, 240);
    glEnd();*/

}


void DrawGrid(void)
{
    int i;

    glTranslatef(-10, -0.5, -10);
    for (i = 0; i < 40; i++)
    {
        glPushMatrix();
        glColor3f(0.0f, 0.0f, 0.0f);
        if (i < 20)
        {
            glTranslatef(0.0f, 0.6f, i);
        }
        else
        {
            glTranslatef(i - 20, 0.6f, 0);
            glRotatef(-90, 0.0f, 1.0f, 0.0f);
        }
        glBegin(GL_LINES);
        glVertex3f(-20, 0.0f, 0.0f);
        glVertex3f(20, 0.0f, 0.0f);
        glEnd();
        glPopMatrix();
    }
}


void DrawCylinder(void)
{
    GLfloat mat_diffuse[] = { 0.8, 0.4, 0.3, 1.0 };
    GLfloat mat_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);

    glColor3f(0.5f, 0.25f, 0.2f);
    glPushMatrix();
    glTranslatef(5, 4, 5);
    glScalef(0.5f, 4.0f, 0.5f);
    glRotatef(90, 1, 0, 0);
    glutSolidCylinder(1.0, 1.0, 16, 1);
    glPopMatrix();
}


void DrawGun(void)
{
    GLfloat mat_diffuse[] = { 0.5, 0.4, 0.5, 1.0 };
    GLfloat mat_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glClear(GL_DEPTH_BUFFER_BIT);
    glColor3f(0.5f, 0.4f, 0.5f);

    glLoadIdentity();
    glPushMatrix();
    glTranslatef(1.5f, -1.5f, -4.0f);
    glScalef(0.5f, 0.5f, 3.0f);
    glutSolidCylinder(1.0, 1.0, 16, 1);
    glPopMatrix();
}


void DrawCube(void)
{
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPushMatrix();
    glTranslatef(-5, 2, 0);

    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);   glVertex3f(-1.0, -1.0, 1.0);
    glTexCoord2f(0.0f, 1.0f);   glVertex3f(1.0, -1.0, 1.0);
    glTexCoord2f(1.0f, 1.0f);   glVertex3f(1.0, 1.0, 1.0);
    glTexCoord2f(1.0f, 0.0f);   glVertex3f(-1.0, 1.0, 1.0);
    glEnd();
    glPopMatrix();

    glRotatef(90, 0, 1, 0);

    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);   glVertex3f(-1.0, -1.0, 1.0);
    glTexCoord2f(0.0f, 1.0f);   glVertex3f(1.0, -1.0, 1.0);
    glTexCoord2f(1.0f, 1.0f);   glVertex3f(1.0, 1.0, 1.0);
    glTexCoord2f(1.0f, 0.0f);   glVertex3f(-1.0, 1.0, 1.0);
    glEnd();
    glPopMatrix();

    glRotatef(90, 0, 1, 0);

    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);   glVertex3f(-1.0, -1.0, 1.0);
    glTexCoord2f(0.0f, 1.0f);   glVertex3f(1.0, -1.0, 1.0);
    glTexCoord2f(1.0f, 1.0f);   glVertex3f(1.0, 1.0, 1.0);
    glTexCoord2f(1.0f, 0.0f);   glVertex3f(-1.0, 1.0, 1.0);
    glEnd();
    glPopMatrix();

    glRotatef(90, 0, 1, 0);

    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);   glVertex3f(-1.0, -1.0, 1.0);
    glTexCoord2f(0.0f, 1.0f);   glVertex3f(1.0, -1.0, 1.0);
    glTexCoord2f(1.0f, 1.0f);   glVertex3f(1.0, 1.0, 1.0);
    glTexCoord2f(1.0f, 0.0f);   glVertex3f(-1.0, 1.0, 1.0);
    glEnd();
    glPopMatrix();

    glRotatef(90, 0, 1, 0);
    glRotatef(-90, 1, 0, 0);

    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);   glVertex3f(-1.0, -1.0, 1.0);
    glTexCoord2f(0.0f, 1.0f);   glVertex3f(1.0, -1.0, 1.0);
    glTexCoord2f(1.0f, 1.0f);   glVertex3f(1.0, 1.0, 1.0);
    glTexCoord2f(1.0f, 0.0f);   glVertex3f(-1.0, 1.0, 1.0);
    glEnd();
    glPopMatrix();

    glRotatef(180, 1, 0, 0);

    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);   glVertex3f(-1.0, -1.0, 1.0);
    glTexCoord2f(0.0f, 1.0f);   glVertex3f(1.0, -1.0, 1.0);
    glTexCoord2f(1.0f, 1.0f);   glVertex3f(1.0, 1.0, 1.0);
    glTexCoord2f(1.0f, 0.0f);   glVertex3f(-1.0, 1.0, 1.0);
    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}
