#include <GL/glut.h>
#include <cmath>
#include <iostream>

// Global variables
float planeY = 450.0f;

// DDA Line Drawing Algorithm
void drawLineDDA(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

    float xInc = dx / steps;
    float yInc = dy / steps;

    float x = x1, y = y1;

    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex2f(round(x), round(y));
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// Bresenham Line Drawing Algorithm
void drawLineBresenham(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x1, y1);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
    glEnd();
}

// Midpoint Circle Algorithm
void drawCircleMidpoint(float cx, float cy, float r) {
    int x = 0;
    int y = r;
    int d = 1 - r;

    glBegin(GL_POINTS);
    while (x <= y) {
        glVertex2f(cx + x, cy + y);
        glVertex2f(cx - x, cy + y);
        glVertex2f(cx + x, cy - y);
        glVertex2f(cx - x, cy - y);
        glVertex2f(cx + y, cy + x);
        glVertex2f(cx - y, cy + x);
        glVertex2f(cx + y, cy - x);
        glVertex2f(cx - y, cy - x);

        if (d < 0) {
            d += 2 * x + 3;
        }
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
    glEnd();
}

// Fill circle using scan line
void fillCircle(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 360; i++) {
        float angle = i * 3.14159 / 180;
        glVertex2f(cx + r * cos(angle), cy + r * sin(angle));
    }
    glEnd();
}

// Draw sun
void drawSun() {
    glColor3f(1.0f, 0.9f, 0.3f);
    fillCircle(400, 400, 50);

    // Sun outline using Midpoint Circle
    glColor3f(1.0f, 0.5f, 0.0f);
    glPointSize(2);
    drawCircleMidpoint(400, 400, 50);
    glPointSize(1);
}

// Draw sky
void drawSky() {
    glBegin(GL_QUADS);
    // Top part
    glColor3f(0.0f, 0.3f, 0.6f);
    glVertex2f(0, 600);
    glVertex2f(800, 600);

    // Bottom part (sunset colors)
    glColor3f(1.0f, 0.7f, 0.3f);
    glVertex2f(800, 250);
    glVertex2f(0, 250);
    glEnd();
}

// Draw city buildings
void drawBuildings() {
    // Building 1
    glColor3f(0.3f, 0.3f, 0.4f);
    glBegin(GL_POLYGON);
    glVertex2f(50, 250);
    glVertex2f(120, 250);
    glVertex2f(120, 380);
    glVertex2f(50, 380);
    glEnd();

    // Building 2 (taller)
    glColor3f(0.2f, 0.25f, 0.35f);
    glBegin(GL_POLYGON);
    glVertex2f(140, 250);
    glVertex2f(200, 250);
    glVertex2f(200, 420);
    glVertex2f(140, 420);
    glEnd();

    // Building 3
    glColor3f(0.25f, 0.3f, 0.4f);
    glBegin(GL_POLYGON);
    glVertex2f(220, 250);
    glVertex2f(280, 250);
    glVertex2f(280, 360);
    glVertex2f(220, 360);
    glEnd();

    // Building 4
    glColor3f(0.3f, 0.35f, 0.45f);
    glBegin(GL_POLYGON);
    glVertex2f(650, 250);
    glVertex2f(720, 250);
    glVertex2f(720, 400);
    glVertex2f(650, 400);
    glEnd();

    // Building 5
    glColor3f(0.25f, 0.3f, 0.38f);
    glBegin(GL_POLYGON);
    glVertex2f(540, 250);
    glVertex2f(630, 250);
    glVertex2f(630, 350);
    glVertex2f(540, 350);
    glEnd();
}

// Draw control tower
void drawControlTower() {
    // Tower base
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_POLYGON);
    glVertex2f(320, 250);
    glVertex2f(360, 250);
    glVertex2f(360, 320);
    glVertex2f(320, 320);
    glEnd();

    // Tower top (control room)
    glColor3f(0.6f, 0.6f, 0.65f);
    glBegin(GL_POLYGON);
    glVertex2f(310, 320);
    glVertex2f(370, 320);
    glVertex2f(365, 360);
    glVertex2f(315, 360);
    glEnd();
}

// Draw airplane
void drawAirplane() {
    glPushMatrix();
    glTranslatef(400, planeY, 0);

    // Fuselage
    glColor3f(0.8f, 0.8f, 0.9f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 0);
    glVertex2f(60, 0);
    glVertex2f(70, 5);
    glVertex2f(70, 10);
    glVertex2f(0, 10);
    glEnd();

    // Wings
    glBegin(GL_TRIANGLES);
    glVertex2f(25, 5);
    glVertex2f(25, -15);
    glVertex2f(45, 5);
    glEnd();

    // Tail
    glBegin(GL_TRIANGLES);
    glVertex2f(5, 10);
    glVertex2f(5, 20);
    glVertex2f(15, 10);
    glEnd();

    // Windows using Midpoint Circle
    glColor3f(0.3f, 0.5f, 0.7f);
    fillCircle(20, 7, 2);
    fillCircle(30, 7, 2);
    fillCircle(40, 7, 2);
    fillCircle(50, 7, 2);

    glPopMatrix();
}

// Draw runway
void drawRunway() {
    // Main runway
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 150);
    glVertex2f(800, 180);
    glVertex2f(800, 250);
    glVertex2f(0, 250);
    glEnd();

    // Runway markings using Bresenham
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(2);
    for (int i = 0; i < 800; i += 60) {
        drawLineBresenham(i, 200, i + 30, 205);
    }
    glPointSize(1);
}

// Draw ground
void drawGround() {
    glColor3f(0.2f, 0.4f, 0.2f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 50);
    glVertex2f(800, 50);
    glVertex2f(800, 150);
    glVertex2f(0, 150);
    glEnd();
}

// Draw text
void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// Draw bottom border with developer info
void drawBottomBorder() {
    // White border background
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 0);
    glVertex2f(800, 0);
    glVertex2f(800, 50);
    glVertex2f(0, 50);
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f);
    drawText(220, 20, "Developed by: Sajjadul Islam Somon | 221-15-5749");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawSky();
    drawSun();
    drawBuildings();
    drawControlTower();
    drawAirplane();
    drawRunway();
    drawGround();
    drawBottomBorder();
    glutSwapBuffers();
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glPointSize(1);
}

// Keyboard callback function
void keyboard(unsigned char key, int x, int y) {
    if (key == 'f' || key == 'F') {
        planeY += 10.0f;
        if (planeY > 550.0f) planeY = 550.0f;
        glutPostRedisplay();
    }
    else if (key == 'd' || key == 'D') {
        planeY -= 10.0f;
        if (planeY < 200.0f) planeY = 200.0f;
        glutPostRedisplay();
    }
    else if (key == 27) {
        exit(0);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Airport Sunset Scene - Computer Graphics Project");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    std::cout << "=== Airport Scene ===" << std::endl;
    std::cout << "Graphics Algorithms Used:" << std::endl;
    std::cout << "1. DDA Line Algorithm" << std::endl;
    std::cout << "2. Bresenham Line Algorithm" << std::endl;
    std::cout << "3. Midpoint Circle Algorithm" << std::endl;
    std::cout << "\n=== CONTROLS ===" << std::endl;
    std::cout << "Press 'F' to move airplane up" << std::endl;
    std::cout << "Press 'D' to move airplane down" << std::endl;
    std::cout << "Press 'ESC' to exit" << std::endl;

    glutMainLoop();
    return 0;
}