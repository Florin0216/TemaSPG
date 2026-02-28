#include <freeglut.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint grassTexture, frontTexture, backTexture, leftTexture, rightTexture, topTexture, bottomTexture;

struct Vec3 {
    float x, y, z;
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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
	return textureID;
}

void drawSkyBox() {
    glBindTexture(GL_TEXTURE_2D, frontTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, -2.0f, 2.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(2.0f, -2.0f, 2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(2.0f, 2.0f, 2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, 2.0f, 2.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, backTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-2.0f, -2.0f, -2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(2.0f, -2.0f, -2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.0f, 2.0f, -2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.0f, 2.0f, -2.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, leftTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, -2.0f, -2.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-2.0f, -2.0f, 2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.0f, 2.0f, 2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, 2.0f, -2.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, rightTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(2.0f, -2.0f, -2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(2.0f, -2.0f, 2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.0f, 2.0f, 2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(2.0f, 2.0f, -2.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, topTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, 2.0f, -2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(2.0f, 2.0f, -2.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(2.0f, 2.0f, 2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, 2.0f, 2.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, bottomTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, -2.0f, -2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(2.0f, -2.0f, -2.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(2.0f, -2.0f, 2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, -2.0f, 2.0f);
    glEnd();
}

void drawGrass() {
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, -0.5f, -2.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(2.0f, -0.5f, -2.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(2.0f, -0.5f, 2.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, -0.5f, 2.0f);
	glEnd();
}

void drawHillyTerrain() {
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    float resolution = 0.2f; // Cât de detaliat este relieful
    float size = 2.0f;       // Trebuie să coincidă cu baza cubului tău

    for (float x = -size; x < size; x += resolution) {
        glBegin(GL_TRIANGLE_STRIP);
        for (float z = -size; z <= size; z += resolution) {
            // Formula pentru relief: y depinde de x și z
            float y1 = 0.3f * sin(x * 3.0f) * cos(z * 3.0f);
            float y2 = 0.3f * sin((x + resolution) * 3.0f) * cos(z * 3.0f);

            glTexCoord2f((x + size) / (2 * size), (z + size) / (2 * size));
            glVertex3f(x, y1 - 0.5f, z);

            glTexCoord2f((x + resolution + size) / (2 * size), (z + size) / (2 * size));
            glVertex3f(x + resolution, y2 - 0.5f, z);
        }
        glEnd();
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    drawSkyBox();
	drawGrass();
    drawHillyTerrain();

    glutSwapBuffers();
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
	glutCreateWindow("Scena 3D - P1");

	glEnable(GL_DEPTH_TEST);

    frontTexture = loadTexture("textures/sky_front.jpg");
    backTexture = loadTexture("textures/sky_back.jpg");
    leftTexture = loadTexture("textures/sky_left.jpg");
    rightTexture = loadTexture("textures/sky_right.jpg");
    topTexture = loadTexture("textures/sky_top.jpg");
    bottomTexture = loadTexture("textures/sky_bottom.jpg");
	grassTexture = loadTexture("textures/grass.jpg");

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;
}