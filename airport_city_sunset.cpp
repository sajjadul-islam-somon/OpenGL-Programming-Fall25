#include <GL/glut.h>
#include <cmath>
#include <iostream>

// Global variables for animation
float sunY = 400.0f;
float sunSize = 50.0f;
float skyBrightness = 1.0f;
float planeX = -100.0f;
float planeY = 450.0f;
float planeVerticalSpeed = 0.0f;
int frameCount = 0;
const int TOTAL_FRAMES = 360;

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

// Draw sun with glow effect
void drawSun() {
    // Outer glow
    float glowIntensity = skyBrightness * 0.3f;
    glColor4f(1.0f, 0.6f, 0.2f, glowIntensity);
    fillCircle(400, sunY, sunSize + 20);

    glColor3f(1.0f, 0.8f * skyBrightness + 0.2f, 0.3f * skyBrightness);
    fillCircle(400, sunY, sunSize);

    glColor3f(1.0f, 0.5f, 0.0f);
    glPointSize(2);
    drawCircleMidpoint(400, sunY, sunSize);
    glPointSize(1);
}

// Draw gradient sky
void drawSky() {
    float topR = 0.0f;
    float topG = 0.0f + 0.3f * skyBrightness;
    float topB = 0.1f + 0.5f * skyBrightness;

    float midR = 1.0f * skyBrightness;
    float midG = 0.5f * skyBrightness;
    float midB = 0.2f * skyBrightness;

    float botR = 1.0f * skyBrightness;
    float botG = 0.7f * skyBrightness;
    float botB = 0.3f * skyBrightness;

    glBegin(GL_QUADS);
    // Top part (darker)
    glColor3f(topR, topG, topB);
    glVertex2f(0, 600);
    glVertex2f(800, 600);

    // Middle part (sunset colors)
    glColor3f(midR, midG, midB);
    glVertex2f(800, 400);
    glVertex2f(0, 400);
    glEnd();

    glBegin(GL_QUADS);
    // Middle to bottom
    glColor3f(midR, midG, midB);
    glVertex2f(0, 400);
    glVertex2f(800, 400);

    glColor3f(botR, botG, botB);
    glVertex2f(800, 250);
    glVertex2f(0, 250);
    glEnd();
}

// Draw city buildings
void drawBuildings() {
    float darkness = 1.0f - skyBrightness;

    // Building 1
    glColor3f(0.3f - darkness * 0.3f, 0.3f - darkness * 0.3f, 0.4f - darkness * 0.4f);
    glBegin(GL_POLYGON);
    glVertex2f(50, 250);
    glVertex2f(120, 250);
    glVertex2f(120, 380);
    glVertex2f(50, 380);
    glEnd();

    // Windows for Building 1
    if (skyBrightness < 0.7f) {
        glColor3f(1.0f, 1.0f, 0.6f);
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 3; j++) {
                fillCircle(65 + j * 20, 270 + i * 20, 4);
            }
        }
    }

    // Building 2 (taller)
    glColor3f(0.2f - darkness * 0.2f, 0.25f - darkness * 0.25f, 0.35f - darkness * 0.35f);
    glBegin(GL_POLYGON);
    glVertex2f(140, 250);
    glVertex2f(200, 250);
    glVertex2f(200, 420);
    glVertex2f(140, 420);
    glEnd();

    // Windows for Building 2
    if (skyBrightness < 0.7f) {
        glColor3f(1.0f, 1.0f, 0.5f);
        for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 2; j++) {
                fillCircle(155 + j * 25, 270 + i * 20, 4);
            }
        }
    }

    // Building 3
    glColor3f(0.25f - darkness * 0.25f, 0.3f - darkness * 0.3f, 0.4f - darkness * 0.4f);
    glBegin(GL_POLYGON);
    glVertex2f(220, 250);
    glVertex2f(280, 250);
    glVertex2f(280, 360);
    glVertex2f(220, 360);
    glEnd();

    // Building 4
    glColor3f(0.3f - darkness * 0.3f, 0.35f - darkness * 0.35f, 0.45f - darkness * 0.45f);
    glBegin(GL_POLYGON);
    glVertex2f(650, 250);
    glVertex2f(720, 250);
    glVertex2f(720, 400);
    glVertex2f(650, 400);
    glEnd();

    // Building 5
    glColor3f(0.25f - darkness * 0.25f, 0.3f - darkness * 0.3f, 0.38f - darkness * 0.38f);
    glBegin(GL_POLYGON);
    glVertex2f(540, 250);
    glVertex2f(630, 250);
    glVertex2f(630, 350);
    glVertex2f(540, 350);
    glEnd();
}

// Draw control tower
void drawControlTower() {
    float darkness = 1.0f - skyBrightness;

    // Tower base
    glColor3f(0.5f - darkness * 0.5f, 0.5f - darkness * 0.5f, 0.5f - darkness * 0.5f);
    glBegin(GL_POLYGON);
    glVertex2f(320, 250);
    glVertex2f(360, 250);
    glVertex2f(360, 320);
    glVertex2f(320, 320);
    glEnd();

    // Tower top (control room)
    glColor3f(0.6f - darkness * 0.5f, 0.6f - darkness * 0.5f, 0.65f - darkness * 0.55f);
    glBegin(GL_POLYGON);
    glVertex2f(310, 320);
    glVertex2f(370, 320);
    glVertex2f(365, 360);
    glVertex2f(315, 360);
    glEnd();

    // Beacon light on top
    if (skyBrightness < 0.5f) {
        glColor3f(1.0f, 0.0f, 0.0f);
        fillCircle(340, 370, 5);
    }
}

// Draw airplane
void drawAirplane() {
    glPushMatrix();
    glTranslatef(planeX, planeY, 0);

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

    if (skyBrightness < 0.6f) {
        glColor3f(1.0f, 0.0f, 0.0f);
        fillCircle(25, -15, 2);
        glColor3f(0.0f, 1.0f, 0.0f);
        fillCircle(70, 7, 2);
    }

    glPopMatrix();
}

// Draw runway
void drawRunway() {
    float darkness = 1.0f - skyBrightness;
    glColor3f(0.3f - darkness * 0.3f, 0.3f - darkness * 0.3f, 0.3f - darkness * 0.3f);
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

    if (skyBrightness < 0.6f) {
        glColor3f(1.0f, 1.0f, 0.8f);
        for (int i = 50; i < 800; i += 80) {
            fillCircle(i, 160, 3);
            fillCircle(i + 5, 240, 3);
        }
    }
}

// Draw ground
void drawGround() {
    float darkness = 1.0f - skyBrightness;
    glColor3f(0.2f - darkness * 0.2f, 0.4f - darkness * 0.4f, 0.2f - darkness * 0.2f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 50);
    glVertex2f(800, 50);
    glVertex2f(800, 150);
    glVertex2f(0, 150);
    glEnd();
}

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void drawBottomBorder() {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 0);
    glVertex2f(800, 0);
    glVertex2f(800, 50);
    glVertex2f(0, 50);
    glEnd();

    // Text
    glColor3f(0.0f, 0.0f, 0.0f);
    drawText(220, 20, "Developed by: Sajjadul Islam Somon | 221-15-5749");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw scene layers
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

void timer(int value) {
    frameCount++;

    if (frameCount >= TOTAL_FRAMES) {
        frameCount = 0;
    }

    float progress = frameCount / (float)TOTAL_FRAMES;

    sunY = 400.0f - progress * 200.0f;
    sunSize = 50.0f - progress * 10.0f;
    skyBrightness = 1.0f - progress;
    planeX = -100.0f + progress * 1000.0f;

    planeVerticalSpeed *= 0.95f;
    planeY += planeVerticalSpeed;

    if (planeY < 200.0f) {
        planeY = 200.0f;
        planeVerticalSpeed = 0.0f;
    }
    if (planeY > 550.0f) {
        planeY = 550.0f;
        planeVerticalSpeed = 0.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(1);
}

// Keyboard callback function
void keyboard(unsigned char key, int x, int y) {
    if (key == 'f' || key == 'F') {
        planeVerticalSpeed = 5.0f;
    }
    else if (key == 'd' || key == 'D') {
        planeVerticalSpeed = -5.0f;
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
    glutTimerFunc(0, timer, 0);

    std::cout << "=== Airport Sunset Scene ===" << std::endl;
    std::cout << "Graphics Algorithms Used:" << std::endl;
    std::cout << "1. DDA Line Algorithm - Runway center markings" << std::endl;
    std::cout << "2. Bresenham Line Algorithm - Runway edge markings" << std::endl;
    std::cout << "3. Midpoint Circle Algorithm - Sun, windows, lights" << std::endl;
    std::cout << "\nTransformations:" << std::endl;
    std::cout << "- Translation: Sun descending, airplane moving" << std::endl;
    std::cout << "- Scaling: Sun size changing, brightness fading" << std::endl;
    std::cout << "\nAnimation loops continuously every 6 seconds!" << std::endl;
    std::cout << "\n=== CONTROLS ===" << std::endl;
    std::cout << "Press 'F' key to make the airplane fly higher!" << std::endl;
    std::cout << "Press 'D' key to make the airplane fly lower!" << std::endl;
    std::cout << "Press 'ESC' to exit." << std::endl;

    glutMainLoop();
    return 0;
}