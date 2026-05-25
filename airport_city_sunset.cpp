#include <GL/glut.h>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float PLAYER_X = 150.0f;
constexpr float PLAYER_WIDTH = 86.0f;
constexpr float PLAYER_HEIGHT = 32.0f;
constexpr float GROUND_TOP = 150.0f;
constexpr float RUNWAY_TOP = 250.0f;
constexpr int MAX_HITS = 3;
constexpr char HIGH_SCORE_FILE[] = "airport_high_score.txt";
constexpr float PI = 3.14159265358979323846f;

enum class ObstacleType {
    Building,
    Cloud,
    Bird
};

struct Rect {
    float x;
    float y;
    float w;
    float h;
};

struct Obstacle {
    ObstacleType type;
    float x;
    float y;
    float w;
    float h;
    float speed;
    float wobble;
    bool scored;
};

float sunY = 420.0f;
float sunSize = 56.0f;
float skyBrightness = 1.0f;
float planeY = 360.0f;
float planeVerticalSpeed = 0.0f;
float planeTilt = 0.0f;
float sceneFrame = 0.0f;
int elapsedFrames = 0;
int score = 0;
int highScore = 0;
int hits = 0;
int passedObstacles = 0;
int spawnCooldown = 90;
int collisionFlashFrames = 0;
bool gameOver = false;
std::vector<Obstacle> obstacles;

float clampValue(float value, float minValue, float maxValue) {
    return std::max(minValue, std::min(value, maxValue));
}

Rect makeRect(float x, float y, float w, float h) {
    return Rect{ x, y, w, h };
}

bool intersects(const Rect& a, const Rect& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

void drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char character : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, character);
    }
}

void drawSmallText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char character : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, character);
    }
}

void drawLineDDA(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float steps = std::max(std::fabs(dx), std::fabs(dy));

    if (steps <= 0.0f) {
        glBegin(GL_POINTS);
        glVertex2f(std::round(x1), std::round(y1));
        glEnd();
        return;
    }

    float xInc = dx / steps;
    float yInc = dy / steps;
    float x = x1;
    float y = y1;

    glBegin(GL_POINTS);
    for (int i = 0; i <= static_cast<int>(steps); i++) {
        glVertex2f(std::round(x), std::round(y));
        x += xInc;
        y += yInc;
    }
    glEnd();
}

void drawLineBresenham(int x1, int y1, int x2, int y2) {
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x1, y1);
        if (x1 == x2 && y1 == y2) {
            break;
        }

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

void drawCircleMidpoint(float cx, float cy, float r) {
    int x = 0;
    int y = static_cast<int>(r);
    int d = 1 - y;

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

void fillCircle(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 360; i++) {
        float angle = i * PI / 180.0f;
        glVertex2f(cx + r * std::cos(angle), cy + r * std::sin(angle));
    }
    glEnd();
}

void fillRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void saveHighScore() {
    std::ofstream file(HIGH_SCORE_FILE, std::ios::trunc);
    if (file) {
        file << highScore;
    }
}

void loadHighScore() {
    std::ifstream file(HIGH_SCORE_FILE);
    if (file) {
        file >> highScore;
    }
}

void resetGame() {
    planeY = 360.0f;
    planeVerticalSpeed = 0.0f;
    planeTilt = 0.0f;
    sceneFrame = 0.0f;
    elapsedFrames = 0;
    score = 0;
    hits = 0;
    passedObstacles = 0;
    spawnCooldown = 75;
    collisionFlashFrames = 0;
    gameOver = false;
    obstacles.clear();
}

void drawStars() {
    if (skyBrightness > 0.68f) {
        return;
    }

    glColor3f(1.0f, 0.98f, 0.88f);
    glPointSize(2);
    glBegin(GL_POINTS);
    for (int i = 0; i < 60; i++) {
        float x = std::fmod(i * 97.0f + 41.0f, static_cast<float>(WINDOW_WIDTH));
        float y = 320.0f + std::fmod(i * 53.0f + 17.0f, 260.0f);
        glVertex2f(x, y);
    }
    glEnd();
    glPointSize(1);
}

void drawSky() {
    glColor3f(0.02f, 0.05f, 0.10f + 0.20f * skyBrightness);
    glBegin(GL_QUADS);
    glVertex2f(0, 600);
    glVertex2f(800, 600);
    glColor3f(0.05f + 0.20f * skyBrightness, 0.12f + 0.20f * skyBrightness, 0.28f + 0.28f * skyBrightness);
    glVertex2f(800, 430);
    glVertex2f(0, 430);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.30f + 0.18f * skyBrightness, 0.16f + 0.18f * skyBrightness, 0.34f + 0.16f * skyBrightness);
    glVertex2f(0, 430);
    glVertex2f(800, 430);
    glColor3f(0.92f * skyBrightness + 0.10f, 0.48f * skyBrightness + 0.18f, 0.16f * skyBrightness + 0.08f);
    glVertex2f(800, 240);
    glVertex2f(0, 240);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.92f * skyBrightness + 0.10f, 0.48f * skyBrightness + 0.18f, 0.16f * skyBrightness + 0.08f);
    glVertex2f(0, 240);
    glVertex2f(800, 240);
    glColor3f(0.98f * skyBrightness + 0.05f, 0.72f * skyBrightness + 0.08f, 0.34f * skyBrightness + 0.05f);
    glVertex2f(800, 150);
    glVertex2f(0, 150);
    glEnd();

    glColor4f(1.0f, 0.85f, 0.55f, 0.12f + 0.12f * skyBrightness);
    fillCircle(400, sunY, sunSize + 60.0f);

    glColor4f(1.0f, 0.55f, 0.20f, 0.20f + 0.16f * skyBrightness);
    glBegin(GL_TRIANGLES);
    glVertex2f(170, 240);
    glVertex2f(250, 410);
    glVertex2f(330, 240);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(500, 240);
    glVertex2f(585, 405);
    glVertex2f(675, 240);
    glEnd();
}

void drawSun() {
    float glowAlpha = 0.20f + 0.25f * skyBrightness;
    glColor4f(1.0f, 0.63f, 0.22f, glowAlpha);
    fillCircle(400, sunY, sunSize + 24.0f);

    glColor4f(1.0f, 0.74f + 0.12f * skyBrightness, 0.26f, 0.92f);
    fillCircle(400, sunY, sunSize);

    glColor3f(1.0f, 0.45f + 0.15f * skyBrightness, 0.0f);
    glPointSize(2);
    drawCircleMidpoint(400, sunY, sunSize);
    glPointSize(1);
}

void drawBackgroundCity() {
    float dusk = 1.0f - skyBrightness;

    struct BuildingSpec {
        float x;
        float w;
        float h;
    };

    const BuildingSpec buildings[] = {
        {35.0f, 70.0f, 120.0f},
        {115.0f, 60.0f, 170.0f},
        {185.0f, 55.0f, 110.0f},
        {560.0f, 75.0f, 100.0f},
        {650.0f, 60.0f, 150.0f},
        {720.0f, 45.0f, 115.0f}
    };

    for (const BuildingSpec& building : buildings) {
        float shade = 0.14f + 0.16f * dusk;
        float highlight = 0.08f + 0.08f * (1.0f - dusk);
        glColor3f(shade * 0.78f, shade * 0.88f, shade + 0.10f);
        fillRect(building.x, GROUND_TOP, building.w, building.h);

        glColor3f(shade + highlight, shade + highlight * 0.8f, shade + 0.03f);
        glBegin(GL_TRIANGLES);
        glVertex2f(building.x, GROUND_TOP + building.h);
        glVertex2f(building.x + building.w, GROUND_TOP + building.h);
        glVertex2f(building.x + building.w * 0.55f, GROUND_TOP + building.h + 12.0f);
        glEnd();

        glColor3f(shade * 0.55f, shade * 0.60f, shade * 0.70f);
        fillRect(building.x + building.w * 0.08f, GROUND_TOP, building.w * 0.14f, building.h);

        glColor3f(0.08f, 0.10f, 0.14f);
        glBegin(GL_LINES);
        glVertex2f(building.x + building.w * 0.22f, GROUND_TOP);
        glVertex2f(building.x + building.w * 0.22f, GROUND_TOP + building.h);
        glVertex2f(building.x + building.w * 0.78f, GROUND_TOP);
        glVertex2f(building.x + building.w * 0.78f, GROUND_TOP + building.h);
        glEnd();

        glColor3f(0.10f, 0.12f, 0.15f);
        glBegin(GL_LINES);
        for (int r = 1; r < 6; r++) {
            float roofY = GROUND_TOP + building.h + 2.0f + r * 1.7f;
            glVertex2f(building.x + 4.0f, roofY);
            glVertex2f(building.x + building.w - 4.0f, roofY);
        }
        glEnd();

        if (skyBrightness < 0.7f) {
            glColor3f(1.0f, 0.95f, 0.55f);
            for (int row = 0; row < static_cast<int>(building.h / 18.0f); row++) {
                for (int col = 0; col < static_cast<int>(building.w / 16.0f); col++) {
                    if (((row + col) % 3) != 0) {
                        fillRect(building.x + 8.0f + col * 14.0f, GROUND_TOP + 10.0f + row * 16.0f, 4.0f, 5.0f);
                    }
                }
            }
        }

        glColor3f(0.07f, 0.08f, 0.10f);
        glBegin(GL_POLYGON);
        glVertex2f(building.x, GROUND_TOP);
        glVertex2f(building.x + building.w, GROUND_TOP);
        glVertex2f(building.x + building.w * 0.86f, GROUND_TOP - 6.0f);
        glVertex2f(building.x + building.w * 0.14f, GROUND_TOP - 6.0f);
        glEnd();

        glColor3f(0.32f + 0.10f * dusk, 0.30f + 0.08f * dusk, 0.34f + 0.10f * dusk);
        glBegin(GL_LINES);
        glVertex2f(building.x + building.w * 0.5f, GROUND_TOP + building.h + 14.0f);
        glVertex2f(building.x + building.w * 0.5f, GROUND_TOP + building.h + 28.0f);
        glVertex2f(building.x + building.w * 0.46f, GROUND_TOP + building.h + 26.0f);
        glVertex2f(building.x + building.w * 0.54f, GROUND_TOP + building.h + 26.0f);
        glEnd();

        glColor3f(0.92f, 0.95f, 1.0f);
        fillCircle(building.x + building.w * 0.52f, GROUND_TOP + building.h + 28.0f, 1.8f);
    }
}

void drawControlTower() {
    float dusk = 1.0f - skyBrightness;

    glColor3f(0.42f - 0.12f * dusk, 0.43f - 0.10f * dusk, 0.46f - 0.12f * dusk);
    fillRect(320, GROUND_TOP, 40, 78);

    glColor3f(0.55f - 0.08f * dusk, 0.58f - 0.10f * dusk, 0.63f - 0.12f * dusk);
    glBegin(GL_POLYGON);
    glVertex2f(308, 320);
    glVertex2f(372, 320);
    glVertex2f(366, 362);
    glVertex2f(314, 362);
    glEnd();

    glColor3f(0.15f, 0.18f, 0.22f);
    fillRect(324, 335, 32, 15);

    glColor3f(0.95f, 0.40f, 0.18f);
    fillCircle(340, 370, 5);

    if (skyBrightness < 0.65f) {
        glColor3f(1.0f, 1.0f, 0.55f);
        drawLineDDA(340, 370, 352, 390);
        drawLineDDA(340, 370, 328, 390);
    }
}

void drawRunway() {
    float dusk = 1.0f - skyBrightness;

    glColor3f(0.20f + 0.08f * dusk, 0.20f + 0.07f * dusk, 0.22f + 0.07f * dusk);
    glBegin(GL_POLYGON);
    glVertex2f(0, 160);
    glVertex2f(800, 160);
    glVertex2f(800, 250);
    glVertex2f(0, 250);
    glEnd();

    glColor3f(0.14f, 0.14f, 0.16f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 166);
    glVertex2f(800, 166);
    glVertex2f(800, 172);
    glVertex2f(0, 172);
    glEnd();

    glColor3f(0.08f, 0.08f, 0.10f);
    drawLineDDA(0, 172, 800, 172);
    drawLineDDA(0, 160, 800, 160);
    drawLineDDA(0, 250, 800, 250);

    glColor3f(0.98f, 0.98f, 0.96f);
    glPointSize(2);
    for (int i = 0; i < 800; i += 80) {
        drawLineBresenham(i + 6, 204, i + 44, 204);
    }
    glPointSize(1);

    glColor3f(0.95f, 0.84f, 0.24f);
    for (int i = 20; i < 800; i += 74) {
        fillRect(i, 232, 18, 2.5f);
    }

    if (skyBrightness < 0.72f) {
        glColor3f(1.0f, 0.88f, 0.52f);
        for (int i = 40; i < 800; i += 72) {
            fillCircle(i, 162, 2.3f);
            fillCircle(i + 18, 162, 2.3f);
            fillCircle(i + 9, 240, 2.2f);
        }
    }

    glColor3f(0.38f, 0.40f, 0.42f);
    glBegin(GL_LINES);
    for (int i = 0; i <= 10; i++) {
        float x = 80.0f + i * 58.0f;
        glVertex2f(x, 160.0f);
        glVertex2f(x - 18.0f, 250.0f);
    }
    glEnd();

    glColor3f(0.20f, 0.20f, 0.22f);
    glBegin(GL_QUADS);
    glVertex2f(0, 250);
    glVertex2f(800, 250);
    glVertex2f(800, 256);
    glVertex2f(0, 256);
    glEnd();
}

void drawGround() {
    float dusk = 1.0f - skyBrightness;

    glColor3f(0.14f - 0.04f * dusk, 0.28f - 0.10f * dusk, 0.14f - 0.04f * dusk);
    glBegin(GL_POLYGON);
    glVertex2f(0, 50);
    glVertex2f(800, 50);
    glVertex2f(800, 150);
    glVertex2f(0, 150);
    glEnd();

    glColor3f(0.09f, 0.18f, 0.09f);
    glBegin(GL_LINES);
    for (int i = 0; i < 800; i += 35) {
        glVertex2f(i, 58);
        glVertex2f(i + 18, 90);
    }
    glEnd();

    glColor3f(0.18f, 0.12f, 0.07f);
    glBegin(GL_LINES);
    for (int i = 10; i < 800; i += 48) {
        glVertex2f(i, 62);
        glVertex2f(i + 10, 52);
    }
    glEnd();

    glColor3f(0.20f, 0.24f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(0, 50);
    glVertex2f(800, 50);
    glVertex2f(800, 54);
    glVertex2f(0, 54);
    glEnd();

    glColor3f(0.16f, 0.10f, 0.06f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 148);
    glVertex2f(800, 148);
    glVertex2f(800, 150);
    glVertex2f(0, 150);
    glEnd();

    glColor3f(0.26f, 0.31f, 0.25f);
    glBegin(GL_TRIANGLES);
    glVertex2f(120, 150);
    glVertex2f(170, 180);
    glVertex2f(220, 150);
    glVertex2f(520, 150);
    glVertex2f(585, 182);
    glVertex2f(650, 150);
    glEnd();
}

void drawBirdObstacle(const Obstacle& obstacle) {
    float wingLift = std::sin(obstacle.wobble + sceneFrame * 0.18f) * 6.0f;
    glColor3f(0.10f, 0.08f, 0.06f);

    glBegin(GL_TRIANGLES);
    glVertex2f(obstacle.x + obstacle.w * 0.10f, obstacle.y + obstacle.h * 0.50f);
    glVertex2f(obstacle.x + obstacle.w * 0.52f, obstacle.y + obstacle.h * 0.72f + wingLift);
    glVertex2f(obstacle.x + obstacle.w * 0.55f, obstacle.y + obstacle.h * 0.50f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(obstacle.x + obstacle.w * 0.50f, obstacle.y + obstacle.h * 0.50f);
    glVertex2f(obstacle.x + obstacle.w * 0.88f, obstacle.y + obstacle.h * 0.72f - wingLift);
    glVertex2f(obstacle.x + obstacle.w * 0.78f, obstacle.y + obstacle.h * 0.48f);
    glEnd();

    glColor3f(0.18f, 0.12f, 0.08f);
    fillCircle(obstacle.x + obstacle.w * 0.58f, obstacle.y + obstacle.h * 0.54f, 3.0f);
}

void drawCloudObstacle(const Obstacle& obstacle) {
    glColor4f(1.0f, 1.0f, 1.0f, 0.72f);
    fillCircle(obstacle.x + obstacle.w * 0.18f, obstacle.y + obstacle.h * 0.45f, obstacle.h * 0.34f);
    fillCircle(obstacle.x + obstacle.w * 0.38f, obstacle.y + obstacle.h * 0.30f, obstacle.h * 0.42f);
    fillCircle(obstacle.x + obstacle.w * 0.58f, obstacle.y + obstacle.h * 0.43f, obstacle.h * 0.48f);
    fillCircle(obstacle.x + obstacle.w * 0.76f, obstacle.y + obstacle.h * 0.34f, obstacle.h * 0.35f);
    fillRect(obstacle.x + obstacle.w * 0.10f, obstacle.y + obstacle.h * 0.18f, obstacle.w * 0.72f, obstacle.h * 0.26f);
}

void drawBuildingObstacle(const Obstacle& obstacle) {
    float dusk = 1.0f - skyBrightness;
    glColor3f(0.17f - 0.03f * dusk, 0.18f - 0.03f * dusk, 0.24f - 0.04f * dusk);
    fillRect(obstacle.x, obstacle.y, obstacle.w, obstacle.h);

    glColor3f(0.23f - 0.04f * dusk, 0.24f - 0.04f * dusk, 0.30f - 0.04f * dusk);
    glBegin(GL_TRIANGLES);
    glVertex2f(obstacle.x, obstacle.y + obstacle.h);
    glVertex2f(obstacle.x + obstacle.w, obstacle.y + obstacle.h);
    glVertex2f(obstacle.x + obstacle.w * 0.55f, obstacle.y + obstacle.h + 14.0f);
    glEnd();

    glColor3f(1.0f, 0.86f, 0.35f);
    for (int row = 0; row < static_cast<int>((obstacle.h - 20.0f) / 18.0f); row++) {
        for (int col = 0; col < static_cast<int>((obstacle.w - 16.0f) / 14.0f); col++) {
            if (((row + col) % 3) != 0) {
                fillRect(obstacle.x + 7.0f + col * 12.0f, obstacle.y + 8.0f + row * 16.0f, 4.0f, 5.0f);
            }
        }
    }

    glColor3f(0.95f, 0.35f, 0.25f);
    fillCircle(obstacle.x + obstacle.w * 0.52f, obstacle.y + obstacle.h + 8.0f, 4.0f);
}

void drawObstacle(const Obstacle& obstacle) {
    switch (obstacle.type) {
    case ObstacleType::Building:
        drawBuildingObstacle(obstacle);
        break;
    case ObstacleType::Cloud:
        drawCloudObstacle(obstacle);
        break;
    case ObstacleType::Bird:
        drawBirdObstacle(obstacle);
        break;
    }
}

void drawPlane() {
    glPushMatrix();
    glTranslatef(PLAYER_X, planeY, 0.0f);
    glRotatef(planeTilt, 0.0f, 0.0f, 1.0f);

    if (collisionFlashFrames > 0) {
        glColor3f(1.0f, 0.35f, 0.25f);
    }
    else {
        glColor3f(0.90f, 0.92f, 0.97f);
    }
    glBegin(GL_POLYGON);
    glVertex2f(0, 8);
    glVertex2f(65, 8);
    glVertex2f(78, 12);
    glVertex2f(82, 16);
    glVertex2f(78, 20);
    glVertex2f(65, 24);
    glVertex2f(0, 24);
    glEnd();

    glColor3f(0.76f, 0.80f, 0.88f);
    glBegin(GL_TRIANGLES);
    glVertex2f(24, 18);
    glVertex2f(30, 42);
    glVertex2f(44, 18);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(9, 24);
    glVertex2f(9, 40);
    glVertex2f(20, 24);
    glEnd();

    glColor3f(0.70f, 0.74f, 0.80f);
    glBegin(GL_TRIANGLES);
    glVertex2f(42, 18);
    glVertex2f(57, 8);
    glVertex2f(49, 18);
    glEnd();

    glColor3f(0.28f, 0.52f, 0.78f);
    for (int i = 0; i < 4; i++) {
        fillCircle(18.0f + i * 13.0f, 18.0f, 2.4f);
    }

    glColor3f(0.92f, 0.55f, 0.18f);
    fillCircle(70, 16, 2.8f);

    glColor3f(0.18f, 0.22f, 0.28f);
    fillRect(12, 12, 46, 2.0f);

    glPopMatrix();
}

void drawHUD() {
    glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
    fillRect(12.0f, 522.0f, 330.0f, 66.0f);

    std::ostringstream timeStream;
    timeStream << std::fixed << std::setprecision(1) << (elapsedFrames / 60.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(24.0f, 566.0f, "Plane Escape");
    drawSmallText(24.0f, 546.0f, "Time: " + timeStream.str() + " sec");
    drawSmallText(24.0f, 530.0f, "Score: " + std::to_string(score) + "   Best: " + std::to_string(highScore));
    drawSmallText(185.0f, 546.0f, "Hits: " + std::to_string(hits) + "/" + std::to_string(MAX_HITS));
    drawSmallText(185.0f, 530.0f, "W/S or Up/Down to climb and descend");

    if (gameOver) {
        glColor4f(0.0f, 0.0f, 0.0f, 0.60f);
        fillRect(170.0f, 210.0f, 460.0f, 150.0f);
        glColor3f(1.0f, 0.92f, 0.65f);
        drawText(255.0f, 300.0f, "GAME OVER");
        drawSmallText(230.0f, 270.0f, "Press R to restart from the beginning");
        drawSmallText(230.0f, 250.0f, "Highest record is saved automatically");
    }
}

void drawBottomBorder() {
    glColor3f(0.96f, 0.96f, 0.97f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 0);
    glVertex2f(800, 0);
    glVertex2f(800, 50);
    glVertex2f(0, 50);
    glEnd();

    glColor3f(0.10f, 0.10f, 0.10f);
    drawSmallText(20.0f, 18.0f, "Developed by: Sajjadul Islam Somon | 221-15-5749");
    drawSmallText(560.0f, 18.0f, "R: restart   ESC: quit");
}

void spawnObstacle() {
    Obstacle obstacle{};
    obstacle.x = WINDOW_WIDTH + 30.0f + static_cast<float>(std::rand() % 120);
    obstacle.wobble = static_cast<float>(std::rand() % 360) * PI / 180.0f;
    obstacle.scored = false;

    int roll = std::rand() % 100;
    if (roll < 38) {
        obstacle.type = ObstacleType::Cloud;
        obstacle.w = 110.0f + static_cast<float>(std::rand() % 70);
        obstacle.h = 40.0f + static_cast<float>(std::rand() % 22);
        obstacle.y = 300.0f + static_cast<float>(std::rand() % 170);
        obstacle.speed = 2.0f + static_cast<float>(std::rand() % 20) * 0.06f;
    }
    else if (roll < 68) {
        obstacle.type = ObstacleType::Bird;
        obstacle.w = 42.0f + static_cast<float>(std::rand() % 18);
        obstacle.h = 20.0f + static_cast<float>(std::rand() % 10);
        obstacle.y = 250.0f + static_cast<float>(std::rand() % 210);
        obstacle.speed = 4.2f + static_cast<float>(std::rand() % 25) * 0.05f;
    }
    else {
        obstacle.type = ObstacleType::Building;
        obstacle.w = 60.0f + static_cast<float>(std::rand() % 55);
        obstacle.h = 110.0f + static_cast<float>(std::rand() % 120);
        obstacle.y = GROUND_TOP;
        obstacle.speed = 2.2f + static_cast<float>(std::rand() % 12) * 0.05f;
    }

    obstacles.push_back(obstacle);
}

void updateGame() {
    if (gameOver) {
        if (collisionFlashFrames > 0) {
            collisionFlashFrames--;
        }
        return;
    }

    elapsedFrames++;
    sceneFrame += 1.0f;

    float sunsetCycle = 0.5f - 0.5f * std::cos(sceneFrame * 0.0045f);
    skyBrightness = 1.0f - 0.70f * sunsetCycle;
    sunY = 470.0f - 220.0f * sunsetCycle;
    sunSize = 56.0f - 10.0f * sunsetCycle;

    planeVerticalSpeed -= 0.09f;
    planeVerticalSpeed = clampValue(planeVerticalSpeed, -6.0f, 6.5f);
    planeY += planeVerticalSpeed;
    planeTilt = clampValue(planeVerticalSpeed * 4.5f, -18.0f, 18.0f);

    if (planeY < 180.0f) {
        planeY = 180.0f;
        planeVerticalSpeed = 0.0f;
    }
    if (planeY > 540.0f) {
        planeY = 540.0f;
        planeVerticalSpeed = 0.0f;
    }

    spawnCooldown--;
    if (spawnCooldown <= 0) {
        spawnObstacle();
        spawnCooldown = std::max(38, 95 - score / 45) + (std::rand() % 28);
    }

    Rect planeBounds = makeRect(PLAYER_X + 8.0f, planeY + 4.0f, PLAYER_WIDTH - 14.0f, PLAYER_HEIGHT - 8.0f);
    std::vector<Obstacle> activeObstacles;
    activeObstacles.reserve(obstacles.size());

    for (Obstacle obstacle : obstacles) {
        obstacle.x -= obstacle.speed;

        if (!obstacle.scored && obstacle.x + obstacle.w < PLAYER_X) {
            obstacle.scored = true;
            passedObstacles++;
        }

        Rect obstacleBounds = makeRect(obstacle.x, obstacle.y, obstacle.w, obstacle.h);
        bool hit = intersects(planeBounds, obstacleBounds);

        if (hit) {
            hits++;
            collisionFlashFrames = 14;
            if (hits >= MAX_HITS) {
                gameOver = true;
                score = static_cast<int>(elapsedFrames * 0.20f) + passedObstacles * 50;
                if (score > highScore) {
                    highScore = score;
                    saveHighScore();
                }
                break;
            }
            continue;
        }

        if (obstacle.x + obstacle.w > -50.0f) {
            activeObstacles.push_back(obstacle);
        }
    }

    obstacles.swap(activeObstacles);

    score = static_cast<int>(elapsedFrames * 0.20f) + passedObstacles * 50;
    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }

    if (collisionFlashFrames > 0) {
        collisionFlashFrames--;
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawSky();
    drawStars();
    drawSun();
    drawBackgroundCity();
    drawRunway();
    drawGround();
    drawControlTower();

    for (const Obstacle& obstacle : obstacles) {
        drawObstacle(obstacle);
    }

    drawPlane();
    drawHUD();
    drawBottomBorder();

    glutSwapBuffers();
}

void timer(int value) {
    updateGame();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(1.0f);
}

void boostPlaneUp() {
    if (!gameOver) {
        planeVerticalSpeed = std::min(planeVerticalSpeed + 3.2f, 6.5f);
    }
}

void pushPlaneDown() {
    if (!gameOver) {
        planeVerticalSpeed = std::max(planeVerticalSpeed - 3.2f, -6.0f);
    }
}

void keyboard(unsigned char key, int, int) {
    if (key == 'f' || key == 'F' || key == 'w' || key == 'W' || key == ' ') {
        if (gameOver && key == ' ') {
            resetGame();
        }
        else {
            boostPlaneUp();
        }
    }
    else if (key == 'd' || key == 'D' || key == 's' || key == 'S') {
        pushPlaneDown();
    }
    else if (key == 'r' || key == 'R') {
        resetGame();
    }
    else if (key == 27) {
        if (score > highScore) {
            highScore = score;
            saveHighScore();
        }
        std::exit(0);
    }
}

void specialKeyboard(int key, int, int) {
    if (gameOver) {
        return;
    }

    if (key == GLUT_KEY_UP) {
        boostPlaneUp();
    }
    else if (key == GLUT_KEY_DOWN) {
        pushPlaneDown();
    }
}

int main(int argc, char** argv) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Plane Escape - Airport Sunset Game");

    init();
    loadHighScore();
    resetGame();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    glutTimerFunc(0, timer, 0);

    std::cout << "=== Plane Escape - Airport Sunset Game ===" << std::endl;
    std::cout << "Goal: survive as long as possible, avoid buildings, clouds, and birds." << std::endl;
    std::cout << "Three hits end the run. Highest score is saved in airport_high_score.txt." << std::endl;
    std::cout << "Controls: W/S or Up/Down to move, R to restart, ESC to quit." << std::endl;

    glutMainLoop();
    return 0;
}