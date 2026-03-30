#include <glew.h>
#include <freeglut.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define M_PI 3.14159265358979323846

static float cameraPosX = 0.0f;
static float cameraPosY = -0.3f;
static float cameraPosZ = 0.0f;
static float yaw = 0.0f;
static float pitch = 0.0f;
static float sensitivity = 0.15f;
static float cameraSpeed = 0.05f;

struct Vec3 { float x, y, z; };

GLuint grassTexture, mountainTexture;
GLuint frontTexture, backTexture, leftTexture, rightTexture, topTexture, bottomTexture;
GLuint textureRoad1, textureRoad2, textureCurve;
GLuint textureTreeTrunk, textureTreeLeaf;
GLuint textureBenchSeat, textureBenchLeg;
GLuint texturePole, texturePoleTop;
GLuint textureCar, textureWheel, textureHeadlight;

float carX = 0.0f, carY = -0.50f, carZ = 0.6f;
float carAngle = 0.0f;
float carSize = 0.12f;   // Mașina are ~0.25 lungime, deci 0.12 e jumătate
float treeSize = 0.05f;  // Trunchiul e subțire
float benchSize = 0.08f; // Banca e puțin mai lată
float lampSize = 0.04f;

float treeCoords[][3] = {
    {-1.85f, -0.51f, -0.85f}, {-1.23f, -0.51f, 1.2f}, {-0.47f, -0.51f, 1.5f},
    {0.56f, -0.51f, 1.45f}, {1.12f, -0.51f, 1.33f}, {-1.23f, -0.51f, -1.2f},
    {-0.47f, -0.51f, -1.5f}, {0.56f, -0.51f, -1.45f}, {1.12f, -0.51f, -1.33f},
    {1.7f, -0.51f, -1.2f}, {1.5f, -0.51f, -1.5f}, {-1.4f, -0.51f, -1.45f}, {-1.12f, -0.51f, -1.33f}
};

float benchCoords[][3] = {
    {0.25f, -0.5f, 0.9f}, {0.7f, -0.5f, 0.9f}, {0.0f, -0.5f, 0.9f},
    {-0.25f, -0.5f, 0.9f}, {-0.7f, -0.5f, 0.9f}, {0.25f, -0.5f, -0.9f},
    {0.7f, -0.5f, -0.9f}, {0.0f, -0.5f, -0.9f}, {-0.25f, -0.5f, -0.9f}, {-0.7f, -0.5f, -0.9f}
};

float lampCoords[][3] = {
    {0.4f, -0.4f, -0.9f}, {-0.4f, -0.4f, 0.9f}, {0.4f, -0.4f, 0.9f}
};

GLuint loadTexture(const char* path) {
	GLuint textureID;
	int nrChannels, width, height;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum fmt = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "EROARE: nu pot incarca textura: " << path << std::endl;
    }
	stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

bool checkCollision2D(float cx1, float cz1, float halfSize1,
    float cx2, float cz2, float halfSize2) {
    return (fabs(cx1 - cx2) < (halfSize1 + halfSize2)) &&
        (fabs(cz1 - cz2) < (halfSize1 + halfSize2));
}

void setupLighting() {
    GLfloat lightPos[] = { 2.0f, 1.9f, 2.0f, 1.0f };
    GLfloat ambientLight[] = {0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);
}

void updateCamera() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float radYaw = (float)(yaw * M_PI / 180.0);
    float radPitch = (float)(pitch * M_PI / 180.0);

    float dirX = (float)(cos(radPitch) * sin(radYaw));
    float dirY = (float)(sin(radPitch));
    float dirZ = -(float)(cos(radPitch) * cos(radYaw));

    gluLookAt(cameraPosX, cameraPosY, cameraPosZ,
        cameraPosX + dirX, cameraPosY + dirY, cameraPosZ + dirZ,
        0.0, 1.0, 0.0);
}

void mouseMotion(int x, int y) {
    static int lastX = 400, lastY = 300;

    float offsetX = (x - lastX) * sensitivity;
    float offsetY = (lastY - y) * sensitivity;

    lastX = x;
    lastY = y;

    yaw += offsetX;
    pitch += offsetY;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glutPostRedisplay();
}

void drawSkyBox() {
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, frontTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(-2, -2, 2);
    glTexCoord2f(1, 1); glVertex3f(2, -2, 2);
    glTexCoord2f(1, 0); glVertex3f(2, 2, 2);
    glTexCoord2f(0, 0); glVertex3f(-2, 2, 2);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, backTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 1); glVertex3f(-2, -2, -2);
    glTexCoord2f(0, 1); glVertex3f(2, -2, -2);
    glTexCoord2f(0, 0); glVertex3f(2, 2, -2);
    glTexCoord2f(1, 0); glVertex3f(-2, 2, -2);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, leftTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(-2, -2, -2);
    glTexCoord2f(1, 1); glVertex3f(-2, -2, 2);
    glTexCoord2f(1, 0); glVertex3f(-2, 2, 2);
    glTexCoord2f(0, 0); glVertex3f(-2, 2, -2);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, rightTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 1); glVertex3f(2, -2, -2);
    glTexCoord2f(0, 1); glVertex3f(2, -2, 2);
    glTexCoord2f(0, 0); glVertex3f(2, 2, 2);
    glTexCoord2f(1, 0); glVertex3f(2, 2, -2);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, topTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-2, 2, -2);
    glTexCoord2f(1, 0); glVertex3f(2, 2, -2);
    glTexCoord2f(1, 1); glVertex3f(2, 2, 2);
    glTexCoord2f(0, 1); glVertex3f(-2, 2, 2);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, bottomTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-2, -2, -2);
    glTexCoord2f(1, 0); glVertex3f(2, -2, -2);
    glTexCoord2f(1, 1); glVertex3f(2, -2, 2);
    glTexCoord2f(0, 1); glVertex3f(-2, -2, 2);
    glEnd();
}

void drawGrass() {
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(-2, -0.5f, -2);
    glTexCoord2f(1, 1); glVertex3f(2, -0.5f, -2);
    glTexCoord2f(1, 0); glVertex3f(2, -0.5f, 2);
    glTexCoord2f(0, 0); glVertex3f(-2, -0.5f, 2);
	glEnd();
}

void drawMountain(float x, float y, float z, float width, float height) {
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, mountainTexture);
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.5f, 1); glVertex3f(x, y + height, z);
    glTexCoord2f(0, 0); glVertex3f(x - width, y, z + width);
    glTexCoord2f(1, 0); glVertex3f(x + width, y, z + width);

    glTexCoord2f(0.5f, 1); glVertex3f(x, y + height, z);
    glTexCoord2f(0, 0); glVertex3f(x + width, y, z + width);
    glTexCoord2f(1, 0); glVertex3f(x + width, y, z - width);

    glTexCoord2f(0.5f, 1); glVertex3f(x, y + height, z);
    glTexCoord2f(0, 0); glVertex3f(x + width, y, z - width);
    glTexCoord2f(1, 0); glVertex3f(x - width, y, z - width);

    glTexCoord2f(0.5f, 1); glVertex3f(x, y + height, z);
    glTexCoord2f(0, 0); glVertex3f(x - width, y, z - width);
    glTexCoord2f(1, 0); glVertex3f(x - width, y, z + width);
    glEnd();
        }

void drawRoad() {
    glBindTexture(GL_TEXTURE_2D, textureRoad1);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.6f, -0.49f, 0.8f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.6f, -0.49f, 0.8f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.6f, -0.49f, 0.4f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.6f, -0.49f, 0.4f);

    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.6f, -0.49f, -0.4f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.6f, -0.49f, -0.4f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.6f, -0.49f, -0.8f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.6f, -0.49f, -0.8f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, textureRoad2);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.6f, -0.49f, 0.4f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -0.49f, 0.4f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -0.49f, -0.4f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.6f, -0.49f, -0.4f);

    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.6f, -0.49f, 0.4f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -0.49f, 0.4f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -0.49f, -0.4f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.6f, -0.49f, -0.4f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, textureCurve);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.6f, -0.49f, 0.4f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -0.49f, 0.4f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -0.49f, 0.8f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.6f, -0.49f, 0.8f);

    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.6f, -0.49f, 0.4f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -0.49f, 0.4f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -0.49f, 0.8f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.6f, -0.49f, 0.8f);

    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.6f, -0.49f, -0.4f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -0.49f, -0.4f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -0.49f, -0.8f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.6f, -0.49f, -0.8f);

    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.6f, -0.49f, -0.4f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -0.49f, -0.4f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -0.49f, -0.8f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.6f, -0.49f, -0.8f);
        glEnd();
    }

void drawTree(float baseX, float baseY, float baseZ, float trunkHeight, float trunkRadius) {
    glPushMatrix();
    glTranslatef(baseX, baseY, baseZ);

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    glBindTexture(GL_TEXTURE_2D, textureTreeTrunk);
    GLUquadric* qt = gluNewQuadric();
    gluQuadricTexture(qt, GL_TRUE);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(qt, trunkRadius, trunkRadius, trunkHeight, 16, 4);
    glPopMatrix();
    gluDeleteQuadric(qt);

    glBindTexture(GL_TEXTURE_2D, textureTreeLeaf);
    GLUquadric* ql = gluNewQuadric();
    gluQuadricTexture(ql, GL_TRUE);
    float f = trunkRadius / 0.03f;
    float ty = trunkHeight;

    glPushMatrix(); glTranslatef(0, ty + 0.01f * f, 0);
    gluSphere(ql, 0.10f * f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.10f * f, ty + 0.08f * f, 0);
    gluSphere(ql, 0.08f * f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef(0.10f * f, ty + 0.08f * f, 0);
    gluSphere(ql, 0.08f * f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef(0, ty + 0.16f * f, 0);
    gluSphere(ql, 0.08f * f, 12, 12); glPopMatrix();

    gluDeleteQuadric(ql);
    glPopMatrix();
}

void drawTreeShadow(float baseX, float baseY, float baseZ, float trunkHeight, float trunkRadius) {
    glPushMatrix();
    glTranslatef(baseX, baseY, baseZ);

    GLUquadric* qt = gluNewQuadric();
    gluQuadricTexture(qt, GL_TRUE);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(qt, trunkRadius, trunkRadius, trunkHeight, 16, 4);
    glPopMatrix();
    gluDeleteQuadric(qt);

    GLUquadric* ql = gluNewQuadric();
    gluQuadricTexture(ql, GL_TRUE);
    float f = trunkRadius / 0.03f;
    float ty = trunkHeight;

    glPushMatrix(); glTranslatef(0, ty + 0.01f * f, 0);
    gluSphere(ql, 0.10f * f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.10f * f, ty + 0.08f * f, 0);
    gluSphere(ql, 0.08f * f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef(0.10f * f, ty + 0.08f * f, 0);
    gluSphere(ql, 0.08f * f, 12, 12); glPopMatrix();
    glPushMatrix(); glTranslatef(0, ty + 0.16f * f, 0);
    gluSphere(ql, 0.08f * f, 12, 12); glPopMatrix();

    gluDeleteQuadric(ql);
    glPopMatrix();
}

void drawBench(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    glBindTexture(GL_TEXTURE_2D, textureBenchSeat);
    glPushMatrix(); glTranslatef(0, 0.03f, 0); glScalef(0.15f, 0.025f, 0.08f);
    glutSolidCube(1); glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, textureBenchLeg);
    float px[] = { -0.06f, 0.06f,-0.06f, 0.06f };
    float pz[] = { 0.03f, 0.03f,-0.03f,-0.03f };
    for (int i = 0; i < 4; i++) {
        glPushMatrix(); glTranslatef(px[i], -0.01f, pz[i]);
        glScalef(0.015f, 0.05f, 0.015f); glutSolidCube(1); glPopMatrix();
    }
    glPopMatrix();
}

void drawBenchShadow(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    glPushMatrix(); glTranslatef(0, 0.03f, 0); glScalef(0.15f, 0.025f, 0.08f);
    glutSolidCube(1); glPopMatrix();

    float px[] = { -0.06f, 0.06f,-0.06f, 0.06f };
    float pz[] = { 0.03f, 0.03f,-0.03f,-0.03f };
    for (int i = 0; i < 4; i++) {
        glPushMatrix(); glTranslatef(px[i], -0.01f, pz[i]);
        glScalef(0.015f, 0.05f, 0.015f); glutSolidCube(1); glPopMatrix();
    }
    glPopMatrix();
}

void drawLampPost(float x, float y, float z) {
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(0.3f, 0.3f, 0.3f);

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texturePole);
    glTranslatef(0.0f, 0.5f, 0.0f);
    glRotatef(90, 1.0f, 0.0f, 0.0f);
    GLUquadricObj* quadratic = gluNewQuadric();
    gluQuadricTexture(quadratic, GL_TRUE);
    gluCylinder(quadratic, 0.05f, 0.05f, 1.0f, 16, 16);
    glPopMatrix();


    GLfloat lightColor[] = { 1.0f, 1.0f, 0.8f, 1.0f };
    GLfloat lightPos1[] = { x - 0.15f, y + 0.45f, z + 0.0f, 1.0f };
    GLfloat lightPos2[] = { x + 0.15f, y + 0.45f, z + 0.0f, 1.0f };

    glPushMatrix();
    glTranslatef(-0.15f, 0.45f, 0.0f);
    glColor3f(1.0f, 1.0f, 0.0f);
    glDisable(GL_TEXTURE_2D);
    glutSolidSphere(0.1f, 16, 16);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
    glEnable(GL_LIGHT1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.15f, 0.45f, 0.0f);
    glColor3f(1.0f, 1.0f, 0.0f);
    glutSolidSphere(0.1f, 16, 16);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT2, GL_POSITION, lightPos2);
    glEnable(GL_LIGHT2);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    glPopMatrix();

}

void drawCar(float x, float y, float z) {
    glPushMatrix();

    // 1. Translația și Rotația (Ordinea contează!)
    glTranslatef(x, y + 0.05f, z);
    glRotatef(carAngle, 0.0f, 1.0f, 0.0f); // Rotire pe axa orizontală

    // Activăm normalizarea pentru ca scalarea să nu afecteze intensitatea luminii
    glEnable(GL_NORMALIZE);
    glScalef(0.5f, 0.5f, 0.5f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureCar);
    glColor3f(1.0f, 1.0f, 1.0f);

    // --- CORPUL INFERIOR AL MAȘINII ---
    float w1 = 0.12f, h1 = 0.06f, d1 = 0.25f;
    glBegin(GL_QUADS);
    // Față
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0, 0); glVertex3f(-w1, -h1, d1);
    glTexCoord2f(1, 0); glVertex3f(w1, -h1, d1);
    glTexCoord2f(1, 1); glVertex3f(w1, h1, d1);
    glTexCoord2f(0, 1); glVertex3f(-w1, h1, d1);
    // Spate
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0, 0); glVertex3f(w1, -h1, -d1);
    glTexCoord2f(1, 0); glVertex3f(-w1, -h1, -d1);
    glTexCoord2f(1, 1); glVertex3f(-w1, h1, -d1);
    glTexCoord2f(0, 1); glVertex3f(w1, h1, -d1);
    // Sus
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(-w1, h1, d1);
    glTexCoord2f(1, 0); glVertex3f(w1, h1, d1);
    glTexCoord2f(1, 1); glVertex3f(w1, h1, -d1);
    glTexCoord2f(0, 1); glVertex3f(-w1, h1, -d1);
    // Jos
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(-w1, -h1, -d1);
    glTexCoord2f(1, 0); glVertex3f(w1, -h1, -d1);
    glTexCoord2f(1, 1); glVertex3f(w1, -h1, d1);
    glTexCoord2f(0, 1); glVertex3f(-w1, -h1, d1);
    // Stânga
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(-w1, -h1, -d1);
    glTexCoord2f(1, 0); glVertex3f(-w1, -h1, d1);
    glTexCoord2f(1, 1); glVertex3f(-w1, h1, d1);
    glTexCoord2f(0, 1); glVertex3f(-w1, h1, -d1);
    // Dreapta
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(w1, -h1, d1);
    glTexCoord2f(1, 0); glVertex3f(w1, -h1, -d1);
    glTexCoord2f(1, 1); glVertex3f(w1, h1, -d1);
    glTexCoord2f(0, 1); glVertex3f(w1, h1, d1);
    glEnd();

    // --- CABINA (PARTEA DE SUS) ---
    float w2 = 0.10f, h2 = 0.06f, d2 = 0.12f;
    glPushMatrix();
    glTranslatef(0.0f, h1 + h2 - 0.01f, -0.05f);
    glBegin(GL_QUADS);
    // Față cabina
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0, 0); glVertex3f(-w2, -h2, d2); glTexCoord2f(1, 0); glVertex3f(w2, -h2, d2);
    glTexCoord2f(1, 1); glVertex3f(w2, h2, d2); glTexCoord2f(0, 1); glVertex3f(-w2, h2, d2);
    // Spate cabina
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0, 0); glVertex3f(w2, -h2, -d2); glTexCoord2f(1, 0); glVertex3f(-w2, -h2, -d2);
    glTexCoord2f(1, 1); glVertex3f(-w2, h2, -d2); glTexCoord2f(0, 1); glVertex3f(w2, h2, -d2);
    // Sus cabina
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(-w2, h2, d2); glTexCoord2f(1, 0); glVertex3f(w2, h2, d2);
    glTexCoord2f(1, 1); glVertex3f(w2, h2, -d2); glTexCoord2f(0, 1); glVertex3f(-w2, h2, -d2);
    // Jos (nu prea se vede, dar e bine să aibă normală)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(-w2, -h2, -d2); glTexCoord2f(1, 0); glVertex3f(w2, -h2, -d2);
    glTexCoord2f(1, 1); glVertex3f(w2, -h2, d2); glTexCoord2f(0, 1); glVertex3f(-w2, -h2, d2);
    // Stânga cabina
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(-w2, -h2, -d2); glTexCoord2f(1, 0); glVertex3f(-w2, -h2, d2);
    glTexCoord2f(1, 1); glVertex3f(-w2, h2, d2); glTexCoord2f(0, 1); glVertex3f(-w2, h2, -d2);
    // Dreapta cabina
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(w2, -h2, d2); glTexCoord2f(1, 0); glVertex3f(w2, -h2, -d2);
    glTexCoord2f(1, 1); glVertex3f(w2, h2, -d2); glTexCoord2f(0, 1); glVertex3f(w2, h2, d2);
    glEnd();
    glPopMatrix();

    // --- ROȚI ---
    glBindTexture(GL_TEXTURE_2D, textureWheel);
    GLUquadric* q = gluNewQuadric();
    gluQuadricTexture(q, GL_TRUE);

    float wheelX = 0.13f, wheelY = -0.05f, wheelZ = 0.16f;
    float positions[4][3] = { {wheelX, wheelY, wheelZ}, {-wheelX, wheelY, wheelZ},
                              {wheelX, wheelY, -wheelZ}, {-wheelX, wheelY, -wheelZ} };

    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(positions[i][0], positions[i][1], positions[i][2]);
        // Orientăm roata (normala cilindrului se calculează automat în gluCylinder)
        glRotatef(90, 0, 1, 0);
        glTranslatef(0, 0, -0.02f);
        gluCylinder(q, 0.05f, 0.05f, 0.04f, 16, 16);

        // Capacele roților (necesită normale pentru iluminare corectă)
        glNormal3f(0, 0, -1);
        gluDisk(q, 0, 0.05f, 16, 1);
        glTranslatef(0, 0, 0.04f);
        glNormal3f(0, 0, 1);
        gluDisk(q, 0, 0.05f, 16, 1);
        glPopMatrix();
    }

    // --- FARURI ---
    glBindTexture(GL_TEXTURE_2D, textureHeadlight);
    glPushMatrix();
    glTranslatef(-0.07f, 0.0f, d1 + 0.01f);
    gluSphere(q, 0.025f, 10, 10);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.07f, 0.0f, d1 + 0.01f);
    gluSphere(q, 0.025f, 10, 10);
    glPopMatrix();

    gluDeleteQuadric(q);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_NORMALIZE);
    glPopMatrix();
}

void calculateShadowMatrix(float shadowMat[16], float groundPlane[4], float lightPos[4]) {
    float dot = groundPlane[0] * lightPos[0] +
        groundPlane[1] * lightPos[1] +
        groundPlane[2] * lightPos[2] +
        groundPlane[3] * lightPos[3];

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            shadowMat[i * 4 + j] = -lightPos[j] * groundPlane[i];
            if (i == j) shadowMat[i * 4 + j] += dot;
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    updateCamera();

    drawSkyBox();
    drawGrass();
    drawMountain(0.0f, -0.5f, 0.0f, 0.35f, 1.2f);
    drawRoad();

    float groundPlane[] = { 0.0f, 1.0f, 0.0f, 0.499f };
    float shadowMat[16];

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glColor4f(0.0f, 0.0f, 0.0f, 0.4f);

    float sunPos[] = { 2.0f, 1.9f, 2.0f, 1.0f };
    calculateShadowMatrix(shadowMat, groundPlane, sunPos);
    glPushMatrix();
    glMultMatrixf(shadowMat);
    for (auto& pos : treeCoords) drawTreeShadow(pos[0], pos[1], pos[2], 0.3f, 0.03f);
    for (auto& pos : benchCoords) drawBenchShadow(pos[0], pos[1], pos[2]);
    glPopMatrix();

    for (int i = 0; i < 3; i++) {
        float bulbOffset = 0.15f * 0.3f;
        float bulbHeight = 0.45f * 0.3f;
        float lights[2][4] = {
            { lampCoords[i][0] - bulbOffset, lampCoords[i][1] + bulbHeight, lampCoords[i][2], 1.0f },
            { lampCoords[i][0] + bulbOffset, lampCoords[i][1] + bulbHeight, lampCoords[i][2], 1.0f }
        };

        for (int j = 0; j < 2; j++) {
            calculateShadowMatrix(shadowMat, groundPlane, lights[j]);
            glPushMatrix();
            glMultMatrixf(shadowMat);

            for (auto& pos : treeCoords) {
                float dist = sqrt(pow(pos[0] - lights[j][0], 2) + pow(pos[2] - lights[j][2], 2));
                if (dist < 0.8f) drawTreeShadow(pos[0], pos[1], pos[2], 0.3f, 0.03f);
            }
            for (auto& pos : benchCoords) {
                float dist = sqrt(pow(pos[0] - lights[j][0], 2) + pow(pos[2] - lights[j][2], 2));
                if (dist < 0.8f) drawBenchShadow(pos[0], pos[1], pos[2]);
            }
            glPopMatrix();
        }
    }
    glDisable(GL_BLEND);

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCar(carX, carY, carZ);
    for (auto& pos : treeCoords) drawTree(pos[0], pos[1], pos[2], 0.3f, 0.03f);
    for (auto& pos : benchCoords) drawBench(pos[0], pos[1], pos[2]);
    for (auto& pos : lampCoords) drawLampPost(pos[0], pos[1], pos[2]);

    glFlush();
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    float cameraSpeed = 0.05f;
    float step = 0.05f;
    float rotateStep = 5.0f;
    if (key == 27) { glutLeaveMainLoop(); return; }

    float nx = cameraPosX, nz = cameraPosZ, ny = cameraPosY;
    float radYaw = (float)(yaw * M_PI / 180.0);

    if (key == 'w') { nx += cameraSpeed * (float)sin(radYaw); nz -= cameraSpeed * (float)cos(radYaw); }
    if (key == 's') { nx -= cameraSpeed * (float)sin(radYaw); nz += cameraSpeed * (float)cos(radYaw); }
    if (key == 'a') { nx -= cameraSpeed * (float)cos(radYaw); nz -= cameraSpeed * (float)sin(radYaw); }
    if (key == 'd') { nx += cameraSpeed * (float)cos(radYaw); nz += cameraSpeed * (float)sin(radYaw); }
    if (key == 'r') ny += cameraSpeed;
    if (key == 'f') ny -= cameraSpeed;

    if (nx > -1.99f && nx < 1.99f) cameraPosX = nx;
    if (nz > -1.99f && nz < 1.99f) cameraPosZ = nz;
    if (ny > -0.48f && ny < 1.99f) cameraPosY = ny;

    float origCarX = carX;
    float origCarY = carY;
    float origCarZ = carZ;

    if (key == 'i') {
        carZ -= step;
    }
    if (key == 'j') {
        carZ += step;
    }
    if (key == 'k') {
        carX -= step;
    }
    if (key == 'l') {
        carX += step;
    }
    if (key == 'u') carAngle += rotateStep;
    if (key == 'o') carAngle -= rotateStep;

    bool collision = false;
    if (carX < -1.9f || carX > 1.9f || carZ < -1.9f || carZ > 1.9f) {
        collision = true;
    }

    // 2. Coliziune cu Copacii
    for (int i = 0; i < 13; i++) {
        if (checkCollision2D(carX, carZ, carSize, treeCoords[i][0], treeCoords[i][2], treeSize)) {
            collision = true;
            break;
        }
    }

    // 3. Coliziune cu Băncile
    if (!collision) { // Verificăm doar dacă nu am găsit deja o coliziune
        for (int i = 0; i < 10; i++) {
            if (checkCollision2D(carX, carZ, carSize, benchCoords[i][0], benchCoords[i][2], benchSize)) {
                collision = true;
                break;
            }
        }
    }

    // 4. Coliziune cu Stâlpii
    if (!collision) {
        for (int i = 0; i < 3; i++) {
            if (checkCollision2D(carX, carZ, carSize, lampCoords[i][0], lampCoords[i][2], lampSize)) {
                collision = true;
                break;
            }
        }
    }

    if (collision) {
        carX = origCarX;
        carY = origCarY;
        carZ = origCarZ;
    }



    glutPostRedisplay();
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
    glutCreateWindow("Prezentare 2+3");

    glewInit();
    setupLighting();

    frontTexture = loadTexture("textures/sky_front.jpg");
    backTexture = loadTexture("textures/sky_back.jpg");
    leftTexture = loadTexture("textures/sky_left.jpg");
    rightTexture = loadTexture("textures/sky_right.jpg");
    topTexture = loadTexture("textures/sky_top.jpg");
    bottomTexture = loadTexture("textures/sky_bottom.jpg");
	grassTexture = loadTexture("textures/grass.jpg");
    mountainTexture = loadTexture("textures/mountain.jpg");
    textureRoad1 = loadTexture("textures/street_3.jpg");
    textureCurve = loadTexture("textures/street_5.jpg");
    textureRoad2 = loadTexture("textures/street_2.jpg");
    textureTreeTrunk = loadTexture("textures/trunk.jpg");
    textureTreeLeaf = loadTexture("textures/bushy.jpg");
    textureBenchLeg = loadTexture("textures/pavement.jpg");
    textureBenchSeat = loadTexture("textures/bench.jpg");
    texturePole = loadTexture("textures/lamp_pole.jpg");
    texturePoleTop = loadTexture("textures/lamp_pole.jpg");
    textureCar = loadTexture("textures/car.jpg");
    textureWheel = loadTexture("textures/wheel.jpg");
    textureHeadlight = loadTexture("textures/headlight.jpg");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(mouseMotion);
    glClearColor(0.2f, 0.5f, 0.8f, 1.0f);
	glutMainLoop();
	return 0;
}