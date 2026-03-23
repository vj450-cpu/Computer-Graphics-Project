#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>

float trainPosition = -70.0f;
float trainSpeed = 0.15f;
float doorAngle = 0.0f;
bool doorOpen = false;
float cloudOffset = 0.0f;
bool isMoving = false;

// 360 ROTATION VARIABLES
float cameraAngleH = 0.0f;  // Horizontal rotation (360 degrees)
float cameraAngleV = 12.0f; // Vertical angle
float cameraDistance = 35.0f;

float passengerBoardingProgress[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
bool passengersBoarding = false;
bool passengersDisappeared[6] = {false, false, false, false, false, false};

GLfloat lightAmbient[] = {0.4f, 0.4f, 0.5f, 1.0f};
GLfloat lightDiffuse[] = {0.7f, 0.7f, 0.8f, 1.0f};
GLfloat lightPosition[] = {15.0f, 25.0f, -15.0f, 1.0f};
GLfloat lightSpecular[] = {0.8f, 0.8f, 0.9f, 1.0f};

void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
}

void drawMoon() {
    glPushMatrix();
    glTranslatef(15.0f, 25.0f, -15.0f);

    glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(0.9f, 0.95f, 1.0f, 0.3f);
    glutSolidSphere(3.5, 24, 24);

    glColor4f(0.95f, 0.95f, 1.0f, 0.7f);
    glutSolidSphere(2.8, 24, 24);

    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidSphere(2.2, 24, 24);

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawCloud(float x, float y, float z, float size) {
    glDisable(GL_LIGHTING);
    glColor3f(0.7f, 0.75f, 0.8f);

    glPushMatrix();
    glTranslatef(x, y, z);

    glutSolidSphere(size, 14, 14);
    glTranslatef(size * 0.85f, 0.0f, 0.0f);
    glutSolidSphere(size * 1.0f, 14, 14);
    glTranslatef(-size * 1.7f, 0.0f, 0.0f);
    glutSolidSphere(size * 0.9f, 14, 14);
    glTranslatef(size * 0.85f, size * 0.35f, 0.0f);
    glutSolidSphere(size * 0.8f, 14, 14);

    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawClouds() {
    drawCloud(-5.0f + cloudOffset, 22.0f, -18.0f, 2.0f);
    drawCloud(20.0f + cloudOffset, 24.0f, -16.0f, 2.3f);
}

void drawPlatform() {
    glColor3f(0.5f, 0.5f, 0.55f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-80.0f, 0.0f, -15.0f);
    glVertex3f(80.0f, 0.0f, -15.0f);
    glVertex3f(80.0f, 0.0f, -5.0f);
    glVertex3f(-80.0f, 0.0f, -5.0f);
    glEnd();

    glColor3f(1.0f, 0.9f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-80.0f, 0.01f, -5.2f);
    glVertex3f(80.0f, 0.01f, -5.2f);
    glVertex3f(80.0f, 0.01f, -5.0f);
    glVertex3f(-80.0f, 0.01f, -5.0f);
    glEnd();
}

void drawTracks() {
    glColor3f(0.25f, 0.25f, 0.3f);
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 2.0f);
    glScalef(150.0f, 0.2f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 5.0f);
    glScalef(150.0f, 0.2f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(0.35f, 0.22f, 0.13f);
    for(float x = -80.0f; x <= 80.0f; x += 3.0f) {
        glPushMatrix();
        glTranslatef(x, 0.1f, 3.5f);
        glScalef(0.3f, 0.15f, 4.0f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
}

void drawCanopy() {
    glColor3f(0.6f, 0.6f, 0.65f);
    for(float x = -70.0f; x <= 70.0f; x += 15.0f) {
        glPushMatrix();
        glTranslatef(x, 5.0f, -10.0f);
        glScalef(0.5f, 10.0f, 0.5f);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    glColor3f(0.7f, 0.7f, 0.75f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-80.0f, 10.0f, -15.0f);
    glVertex3f(80.0f, 10.0f, -15.0f);
    glVertex3f(80.0f, 10.0f, -5.0f);
    glVertex3f(-80.0f, 10.0f, -5.0f);
    glEnd();

    glColor3f(1.0f, 1.0f, 0.8f);
    for(float x = -65.0f; x <= 65.0f; x += 15.0f) {
        glPushMatrix();
        glTranslatef(x, 9.5f, -10.0f);
        glutSolidSphere(0.5, 16, 16);
        glPopMatrix();
    }
}

void drawBench(float x, float z) {
    glColor3f(0.4f, 0.27f, 0.18f);
    glPushMatrix();
    glTranslatef(x, 1.5f, z);
    glScalef(4.0f, 0.3f, 1.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, 2.2f, z - 0.6f);
    glScalef(4.0f, 1.2f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(0.28f, 0.28f, 0.32f);
    for(int i = 0; i < 2; i++) {
        float legX = (i == 0) ? -1.8f : 1.8f;
        glPushMatrix();
        glTranslatef(x + legX, 0.7f, z - 0.5f);
        glScalef(0.2f, 1.4f, 0.2f);
        glutSolidCube(1.0);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(x + legX, 0.7f, z + 0.5f);
        glScalef(0.2f, 1.4f, 0.2f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
}

void drawPassenger(float x, float z, float rotation, bool boarding, float boardProgress, bool disappeared) {
    if(disappeared) return;

    glPushMatrix();
    if(boarding) {
        glTranslatef(x + boardProgress * 5.0f, 0.0f, z + boardProgress * 8.5f);
    } else {
        glTranslatef(x, 0.0f, z);
    }
    glRotatef(rotation, 0.0f, 1.0f, 0.0f);

    glColor3f(0.85f, 0.72f, 0.65f);
    glPushMatrix();
    glTranslatef(0.0f, 4.2f, 0.0f);
    glutSolidSphere(0.4, 16, 16);
    glPopMatrix();

    glColor3f(0.18f, 0.27f, 0.55f);
    glPushMatrix();
    glTranslatef(0.0f, 2.8f, 0.0f);
    glScalef(0.8f, 1.5f, 0.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.6f, 3.0f, 0.0f);
    glScalef(0.2f, 1.2f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.6f, 3.0f, 0.0f);
    glScalef(0.2f, 1.2f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(0.13f, 0.13f, 0.18f);
    glPushMatrix();
    glTranslatef(0.25f, 1.2f, 0.0f);
    glScalef(0.25f, 1.4f, 0.25f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.25f, 1.2f, 0.0f);
    glScalef(0.25f, 1.4f, 0.25f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

void drawDoor(float xOffset, float doorSlide) {
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(xOffset + doorSlide, 0.0f, 1.85f);
    glScalef(1.9f, 2.9f, 0.15f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(0.3f, 0.5f, 0.7f);
    glPushMatrix();
    glTranslatef(xOffset + doorSlide, 0.7f, 1.87f);
    glScalef(1.5f, 1.8f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(0.1f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(xOffset + doorSlide + 0.7f, 0.0f, 1.88f);
    glScalef(0.15f, 0.5f, 0.08f);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawCompartment(float length, float r, float g, float b) {
    glColor3f(r, g, b);
    glPushMatrix();
    glScalef(length, 3.2f, 3.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(0.75f, 0.75f, 0.78f);
    glPushMatrix();
    glTranslatef(0.0f, 1.8f, 0.0f);
    glScalef(length + 0.2f, 0.4f, 3.7f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(0.3f, 0.65f, 0.85f);
    int numWindows = (int)(length / 2.5f);
    for(int i = 0; i < numWindows; i++) {
        float xPos = -length/2 + 1.5f + i * 2.5f;
        glPushMatrix();
        glTranslatef(xPos, 0.8f, 1.77f);
        glScalef(2.2f, 1.8f, 0.05f);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    glColor3f(0.9f, 0.75f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 1.77f);
    glScalef(length - 0.5f, 0.25f, 0.04f);
    glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(0.18f, 0.18f, 0.22f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 1.8f);
    glScalef(4.5f, 3.15f, 0.08f);
    glutSolidCube(1.0);
    glPopMatrix();

    float doorSlide = doorOpen ? doorAngle : 0.0f;
    drawDoor(-2.2f, -doorSlide);
    drawDoor(2.2f, doorSlide);

    glColor3f(0.28f, 0.28f, 0.32f);
    glPushMatrix();
    glTranslatef(-length/2 - 0.3f, -0.5f, 0.0f);
    glScalef(0.4f, 2.0f, 3.0f);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawWheels(float length) {
    glColor3f(0.18f, 0.18f, 0.22f);
    for(int i = 0; i < 2; i++) {
        float xPos = (i == 0) ? -length/2 + 1.8f : length/2 - 1.8f;

        glPushMatrix();
        glTranslatef(xPos, -1.1f, 2.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glutSolidTorus(0.15, 0.5, 12, 16);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(xPos, -1.1f, -2.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glutSolidTorus(0.15, 0.5, 12, 16);
        glPopMatrix();
    }
}

void drawTrain() {
    glPushMatrix();
    glTranslatef(trainPosition, 2.2f, 3.5f);

    glPushMatrix();
    drawCompartment(12.0f, 0.8f, 0.13f, 0.13f);
    drawWheels(12.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-12.5f, 0.0f, 0.0f);
    drawCompartment(12.0f, 0.13f, 0.27f, 0.65f);
    drawWheels(12.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-25.0f, 0.0f, 0.0f);
    drawCompartment(12.0f, 0.18f, 0.55f, 0.27f);
    drawWheels(12.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-37.5f, 0.0f, 0.0f);
    drawCompartment(12.0f, 0.75f, 0.68f, 0.18f);
    drawWheels(12.0f);
    glPopMatrix();

    glPopMatrix();
}

void drawGround() {
    glColor3f(0.15f, 0.25f, 0.15f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-150.0f, -0.1f, -30.0f);
    glVertex3f(150.0f, -0.1f, -30.0f);
    glVertex3f(150.0f, -0.1f, 30.0f);
    glVertex3f(-150.0f, -0.1f, 30.0f);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 360 DEGREE CAMERA ROTATION
    float camX = trainPosition - 12.0f + cameraDistance * sin(cameraAngleH * M_PI / 180.0f);
    float camY = cameraAngleV;
    float camZ = 3.5f + cameraDistance * cos(cameraAngleH * M_PI / 180.0f);

    gluLookAt(camX, camY, camZ,
              trainPosition - 12.0f, 3.0, 3.5,
              0.0, 1.0, 0.0);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    drawMoon();
    drawClouds();

    drawGround();
    drawTracks();
    drawPlatform();
    drawCanopy();

    drawBench(-30.0f, -8.0f);
    drawBench(-10.0f, -8.0f);
    drawBench(10.0f, -8.0f);
    drawBench(30.0f, -8.0f);

    drawPassenger(-28.0f, -7.0f, 90.0f, passengersBoarding, passengerBoardingProgress[0], passengersDisappeared[0]);
    drawPassenger(-25.0f, -7.5f, 120.0f, false, 0.0f, passengersDisappeared[1]);
    drawPassenger(-8.0f, -7.0f, 90.0f, passengersBoarding, passengerBoardingProgress[2], passengersDisappeared[2]);
    drawPassenger(12.0f, -7.5f, 80.0f, false, 0.0f, passengersDisappeared[3]);
    drawPassenger(15.0f, -7.0f, 110.0f, passengersBoarding, passengerBoardingProgress[4], passengersDisappeared[4]);
    drawPassenger(32.0f, -7.0f, 90.0f, false, 0.0f, passengersDisappeared[5]);

    drawTrain();

    glutSwapBuffers();
}

void timer(int value) {
    if(isMoving) {
        trainPosition += trainSpeed;

        if(trainPosition > 90.0f) {
            trainPosition = -70.0f;
            doorOpen = false;
            doorAngle = 0.0f;
            passengersBoarding = false;
            trainSpeed = 0.15f;
            for(int i = 0; i < 6; i++) {
                passengerBoardingProgress[i] = 0.0f;
                passengersDisappeared[i] = false;
            }
        }

        if(trainPosition >= -5.0f && trainPosition <= 5.0f && !doorOpen) {
            trainSpeed = 0.0f;
            doorOpen = true;
            passengersBoarding = true;
        }

        if(doorOpen && doorAngle < 2.2f) {
            doorAngle += 0.08f;
        } else if(doorOpen && doorAngle >= 2.2f) {
            if(passengerBoardingProgress[0] < 1.0f) {
                passengerBoardingProgress[0] += 0.03f;
            } else {
                passengersDisappeared[0] = true;
            }

            if(passengerBoardingProgress[2] < 1.0f) {
                passengerBoardingProgress[2] += 0.025f;
            } else {
                passengersDisappeared[2] = true;
            }

            if(passengerBoardingProgress[4] < 1.0f) {
                passengerBoardingProgress[4] += 0.02f;
            } else {
                passengersDisappeared[4] = true;
            }

            if(passengersDisappeared[0] && passengersDisappeared[2] && passengersDisappeared[4]) {
                doorOpen = false;
                passengersBoarding = false;
                trainSpeed = 0.3f;
            }
        }
    }

    cloudOffset += 0.02f;
    if(cloudOffset > 50.0f) cloudOffset = -20.0f;

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// KEYBOARD WITH 360 ROTATION
void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 's':
        case 'S':
            isMoving = !isMoving;
            break;
        case 'a':
        case 'A':
            cameraAngleH -= 5.0f; // Rotate left
            break;
        case 'd':
        case 'D':
            cameraAngleH += 5.0f; // Rotate right
            break;
        case 'w':
        case 'W':
            cameraAngleV += 2.0f; // Move camera up
            if(cameraAngleV > 40.0f) cameraAngleV = 40.0f;
            break;
        case 'x':
        case 'X':
            cameraAngleV -= 2.0f; // Move camera down
            if(cameraAngleV < 2.0f) cameraAngleV = 2.0f;
            break;
        case 'r':
        case 'R':
            trainPosition = -70.0f;
            trainSpeed = 0.15f;
            doorAngle = 0.0f;
            doorOpen = false;
            isMoving = false;
            cameraAngleH = 0.0f;
            cameraAngleV = 12.0f;
            cloudOffset = 0.0f;
            passengersBoarding = false;
            for(int i = 0; i < 6; i++) {
                passengerBoardingProgress[i] = 0.0f;
                passengersDisappeared[i] = false;
            }
            break;
        case 27:
            exit(0);
            break;
    }
}

// SPECIAL KEYS FOR ARROW KEYS
void specialKeys(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_LEFT:
            cameraAngleH -= 5.0f;
            break;
        case GLUT_KEY_RIGHT:
            cameraAngleH += 5.0f;
            break;
        case GLUT_KEY_UP:
            cameraAngleV += 2.0f;
            if(cameraAngleV > 40.0f) cameraAngleV = 40.0f;
            break;
        case GLUT_KEY_DOWN:
            cameraAngleV -= 2.0f;
            if(cameraAngleV < 2.0f) cameraAngleV = 2.0f;
            break;
    }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void init() {
    glClearColor(0.05f, 0.08f, 0.15f, 1.0f);
    initLighting();
    glEnable(GL_NORMALIZE);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Snow World");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys); // ARROW KEYS
    glutTimerFunc(0, timer, 0);

    printf("Controls:\n");
    printf("S - Start/Stop train\n");
    printf("A/D or LEFT/RIGHT arrows - Rotate camera 360 degrees\n");
    printf("W/X or UP/DOWN arrows - Move camera up/down\n");
    printf("R - Reset scene\n");
    printf("ESC - Exit\n");

    glutMainLoop();
    return 0;
}
