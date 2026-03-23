#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <ctime>

#define M_PI 3.14159265358979323846

// ============= STRUCTURES =============
struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    float length() const { return sqrt(x * x + y * y + z * z); }
    Vec3 normalize() const { float l = length(); return l > 0 ? Vec3(x / l, y / l, z / l) : Vec3(0, 0, 0); }
    Vec3 cross(const Vec3& v) const { return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
};

struct Particle {
    Vec3 position;
    Vec3 velocity;
    Vec3 color;
    float life;
    float fade;
    float size;
};

struct Person {
    Vec3 position;
    Vec3 direction;
    float walkCycle;
    float speed;
    float targetX, targetZ;
    Vec3 color;
};

struct GrassPatch {
    Vec3 position;
    float rotation;
    float scale;
};

struct Tree {
    Vec3 position;
    float height;
    float trunkRadius;
    float canopyRadius;
};

struct Shop {
    Vec3 position;
    Vec3 size;
    Vec3 color;
    const char* name;
};

// ============= GLOBAL VARIABLES =============
float animationSpeed = 1.0f;
bool isAnimating = true;
bool showParticles = true;
bool showPeople = true;
bool showShops = true;
bool showGrass = true;
bool showTrees = true;

// Enhanced Camera
float cameraDistance = 55.0f;
float cameraAngleX = 15.0f;  // Eye level
float cameraAngleY = 45.0f;
float cameraHeight = 8.0f;  // Lower for eye level
int cameraMode = 0;
Vec3 cameraTarget(0, 5, 0);

// Roller Coaster - More dramatic design
std::vector<Vec3> controlPoints;
std::vector<Vec3> splinePoints;
std::vector<Vec3> splineTangents;
std::vector<Vec3> splineNormals;
std::vector<Vec3> splineBinormals;
float coasterPosition = 0.0f;
int coasterCarCount = 5;
std::vector<float> coasterCarOffsets;

// Ferris Wheel
float ferrisWheelRotation = 0.0f;
const int cabinCount = 16;
const float ferrisWheelRadius = 18.0f;

// Carousel
float carouselRotation = 0.0f;
const int horseCount = 12;
const float carouselRadius = 10.0f;

// Environment
std::vector<Particle> fountainParticles;
std::vector<Particle> fireworksParticles;
std::vector<Person> people;
std::vector<GrassPatch> grassPatches;
std::vector<Tree> trees;
std::vector<Shop> shops;
float fireworkTimer = 0.0f;
float timeOfDay = 0.5f;  // 0 = night, 1 = day

// ============= BEZIER FUNCTIONS =============
Vec3 bezierPoint(float t, const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3) {
    float u = 1 - t;
    float tt = t * t, uu = u * u;
    float uuu = uu * u, ttt = tt * t;

    return p0 * uuu + p1 * (3 * uu * t) + p2 * (3 * u * tt) + p3 * ttt;
}

Vec3 bezierTangent(float t, const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3) {
    float u = 1 - t;
    float uu = u * u, tt = t * t;

    Vec3 tangent = p0 * (-3 * uu) + p1 * (3 * uu - 6 * u * t) +
                   p2 * (6 * u * t - 3 * tt) + p3 * (3 * tt);
    return tangent.normalize();
}

void initRollerCoasterTrack() {
    controlPoints.clear();
    // More dramatic and realistic roller coaster path
    controlPoints.push_back(Vec3(0, 5, 0));
    controlPoints.push_back(Vec3(15, 35, 10));  // Steep lift hill
    controlPoints.push_back(Vec3(30, 40, 8));   // Peak
    controlPoints.push_back(Vec3(42, 25, -5));  // First drop
    controlPoints.push_back(Vec3(48, 8, -18));  // Bottom of drop
    controlPoints.push_back(Vec3(50, 18, -30)); // Rise
    controlPoints.push_back(Vec3(42, 22, -38)); // Curve
    controlPoints.push_back(Vec3(28, 15, -35)); // Banking turn
    controlPoints.push_back(Vec3(15, 12, -25)); // Return
    controlPoints.push_back(Vec3(5, 10, -12));  // Final turn
    controlPoints.push_back(Vec3(0, 5, 0));     // Station

    splinePoints.clear();
    splineTangents.clear();
    splineNormals.clear();
    splineBinormals.clear();

    int segments = 150;
    for (size_t i = 0; i < controlPoints.size() - 3; i++) {
        for (int j = 0; j < segments; j++) {
            float t = j / (float)segments;
            Vec3 point = bezierPoint(t, controlPoints[i], controlPoints[i + 1],
                                     controlPoints[i + 2], controlPoints[i + 3]);
            Vec3 tangent = bezierTangent(t, controlPoints[i], controlPoints[i + 1],
                                         controlPoints[i + 2], controlPoints[i + 3]);

            splinePoints.push_back(point);
            splineTangents.push_back(tangent);

            Vec3 up(0, 1, 0);
            Vec3 binormal = tangent.cross(up).normalize();
            Vec3 normal = binormal.cross(tangent).normalize();

            splineNormals.push_back(normal);
            splineBinormals.push_back(binormal);
        }
    }

    coasterCarOffsets.clear();
    for (int i = 0; i < coasterCarCount; i++) {
        coasterCarOffsets.push_back(i * 80.0f);
    }
}

// ============= INITIALIZATION =============
void initGrass() {
    grassPatches.clear();
    srand(12345);  // Fixed seed for consistent grass

    for (int i = 0; i < 8000; i++) {
        GrassPatch grass;
        grass.position = Vec3((rand() % 2400 - 1200) / 10.0f, 0.1f, (rand() % 2400 - 1200) / 10.0f);
        grass.rotation = rand() % 360;
        grass.scale = 0.8f + (rand() % 60) / 100.0f;
        grassPatches.push_back(grass);
    }
}

void initTrees() {
    trees.clear();
    srand(54321);

    for (int i = 0; i < 40; i++) {
        Tree tree;
        tree.position = Vec3((rand() % 2000 - 1000) / 10.0f, 0, (rand() % 2000 - 1000) / 10.0f);

        // Don't place trees in center area
        if (abs(tree.position.x) < 50 && abs(tree.position.z) < 50) continue;

        tree.height = 8.0f + (rand() % 60) / 10.0f;
        tree.trunkRadius = 0.4f + (rand() % 30) / 100.0f;
        tree.canopyRadius = 3.0f + (rand() % 40) / 10.0f;
        trees.push_back(tree);
    }
}

void initShops() {
    shops.clear();
    shops.push_back({Vec3(-50, 0, -20), Vec3(10, 8, 10), Vec3(0.95f, 0.75f, 0.55f), "TICKETS"});
    shops.push_back({Vec3(-55, 0, 30), Vec3(12, 9, 12), Vec3(0.85f, 0.55f, 0.65f), "FOOD COURT"});
    shops.push_back({Vec3(60, 0, 25), Vec3(11, 8, 10), Vec3(0.65f, 0.75f, 0.95f), "ARCADE"});
    shops.push_back({Vec3(58, 0, -40), Vec3(10, 8, 9), Vec3(0.95f, 0.85f, 0.55f), "SOUVENIRS"});
    shops.push_back({Vec3(-25, 0, 50), Vec3(14, 10, 14), Vec3(0.75f, 0.65f, 0.85f), "RESTAURANT"});
}

void initPeople() {
    people.clear();
    srand(time(NULL));

    for (int i = 0; i < 40; i++) {
        Person p;
        p.position = Vec3((rand() % 180) - 90, 0, (rand() % 180) - 90);
        p.direction = Vec3((rand() % 200 - 100) / 100.0f, 0, (rand() % 200 - 100) / 100.0f).normalize();
        p.walkCycle = rand() % 360;
        p.speed = 2.5f + (rand() % 100) / 80.0f;
        p.targetX = (rand() % 180) - 90;
        p.targetZ = (rand() % 180) - 90;
        p.color = Vec3(0.2f + (rand() % 80) / 100.0f, 0.2f + (rand() % 70) / 100.0f, 0.3f + (rand() % 70) / 100.0f);
        people.push_back(p);
    }
}

void initParticles() {
    fountainParticles.clear();

    for (int i = 0; i < 400; i++) {
        Particle p;
        float angle = (rand() % 360) * M_PI / 180.0f;
        float speed = 7.0f + (rand() % 100) / 30.0f;
        p.position = Vec3(-45, 0, 45);
        p.velocity = Vec3(cos(angle) * speed * 0.5f, speed, sin(angle) * speed * 0.5f);
        p.color = Vec3(0.5f + (rand() % 50) / 100.0f, 0.7f + (rand() % 30) / 100.0f, 1.0f);
        p.life = 1.0f;
        p.fade = 0.006f;
        p.size = 0.5f;
        fountainParticles.push_back(p);
    }
}

void createFirework(Vec3 position) {
    for (int i = 0; i < 200; i++) {
        Particle p;
        float theta = (rand() % 360) * M_PI / 180.0f;
        float phi = (rand() % 360) * M_PI / 180.0f;
        float speed = 5.0f + (rand() % 100) / 30.0f;

        p.position = position;
        p.velocity = Vec3(sin(phi) * cos(theta) * speed,
                          sin(phi) * sin(theta) * speed,
                          cos(phi) * speed);
        p.color = Vec3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
        p.life = 1.0f;
        p.fade = 0.01f;
        p.size = 0.6f;
        fireworksParticles.push_back(p);
    }
}

void init() {
    srand(time(NULL));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    // Enhanced fog for depth
    glEnable(GL_FOG);
    GLfloat fogColor[4] = {0.75f, 0.85f, 0.95f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogf(GL_FOG_DENSITY, 0.008f);

    glClearColor(0.75f, 0.85f, 0.95f, 1.0f);
    glShadeModel(GL_SMOOTH);

    initRollerCoasterTrack();
    initShops();
    initPeople();
    initGrass();
    initTrees();
    initParticles();
}

// ============= DRAWING FUNCTIONS =============
void drawSphere(float radius, int slices, int stacks) {
    GLUquadric* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_FILL);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, radius, slices, stacks);
    gluDeleteQuadric(quad);
}

void drawCylinder(float radius, float height, int slices) {
    GLUquadric* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_FILL);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluCylinder(quad, radius, radius, height, slices, 5);
    gluDeleteQuadric(quad);
}

void drawCube(float size) {
    glutSolidCube(size);
}

void drawSky() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);

    glPushMatrix();
    glLoadIdentity();
    glRotatef(-cameraAngleX, 1, 0, 0);
    glRotatef(-cameraAngleY, 0, 1, 0);

    float skySize = 300.0f;

    // Sky gradient - realistic colors
    glBegin(GL_QUADS);

    // Top (zenith) - deep sky blue
    glColor3f(0.25f, 0.5f, 0.85f);
    glVertex3f(-skySize, skySize, -skySize);
    glVertex3f(skySize, skySize, -skySize);
    glVertex3f(skySize, skySize, skySize);
    glVertex3f(-skySize, skySize, skySize);

    // All four sides with gradient to horizon
    for (int side = 0; side < 4; side++) {
        float x1, z1, x2, z2;
        switch(side) {
            case 0: x1 = -skySize; z1 = -skySize; x2 = skySize; z2 = -skySize; break;
            case 1: x1 = skySize; z1 = -skySize; x2 = skySize; z2 = skySize; break;
            case 2: x1 = skySize; z1 = skySize; x2 = -skySize; z2 = skySize; break;
            case 3: x1 = -skySize; z1 = skySize; x2 = -skySize; z2 = -skySize; break;
        }

        glColor3f(0.25f, 0.5f, 0.85f);
        glVertex3f(x1, skySize, z1);
        glVertex3f(x2, skySize, z2);
        glColor3f(0.7f, 0.85f, 0.98f);
        glVertex3f(x2, 0, z2);
        glVertex3f(x1, 0, z1);
    }
    glEnd();

    // Draw sun
    glEnable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 0.92f, 1.0f);
    glPushMatrix();
    glTranslatef(80, 90, 50);
    drawSphere(12.0f, 32, 32);

    // Sun glow
    glColor4f(1.0f, 1.0f, 0.85f, 0.3f);
    drawSphere(18.0f, 32, 32);
    glPopMatrix();
    glDisable(GL_BLEND);

    // Realistic clouds - fluffy cumulus style
    glEnable(GL_BLEND);
    for (int c = 0; c < 12; c++) {
        float cloudX = -150 + c * 30 + (c % 3) * 10;
        float cloudY = 70 + (c % 4) * 8;
        float cloudZ = -100 + (c / 3) * 50;

        glColor4f(1.0f, 1.0f, 1.0f, 0.85f);

        // Multiple spheres for realistic cloud shape
        for (int puff = 0; puff < 7; puff++) {
            glPushMatrix();
            float offsetX = (puff - 3) * 4.0f + sin(puff * 1.2f) * 3.0f;
            float offsetY = sin(puff * 0.8f) * 2.5f;
            float offsetZ = cos(puff * 0.9f) * 2.0f;
            float puffSize = 5.0f + sin(puff * 1.5f) * 2.0f;

            glTranslatef(cloudX + offsetX, cloudY + offsetY, cloudZ + offsetZ);
            drawSphere(puffSize, 16, 16);
            glPopMatrix();
        }
    }
    glDisable(GL_BLEND);

    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_FOG);
}

void drawGrass() {
    if (!showGrass) return;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);

    for (const auto& grass : grassPatches) {
        // Distance culling for performance
        float dist = sqrt(pow(grass.position.x - cameraTarget.x, 2) + pow(grass.position.z - cameraTarget.z, 2));
        if (dist > 100) continue;

        glPushMatrix();
        glTranslatef(grass.position.x, grass.position.y, grass.position.z);
        glRotatef(grass.rotation, 0, 1, 0);

        // Draw as crossed quads for grass blade
        float baseColor = 0.25f + (int)(grass.position.x + grass.position.z) % 10 / 50.0f;
        glColor4f(baseColor, baseColor + 0.35f, baseColor * 0.5f, 0.9f);

        float height = 0.8f * grass.scale;
        float width = 0.15f * grass.scale;

        // First quad
        glBegin(GL_QUADS);
        glVertex3f(-width, 0, 0);
        glVertex3f(width, 0, 0);
        glVertex3f(width * 0.3f, height, 0);
        glVertex3f(-width * 0.3f, height, 0);
        glEnd();

        // Crossed quad
        glRotatef(90, 0, 1, 0);
        glBegin(GL_QUADS);
        glVertex3f(-width, 0, 0);
        glVertex3f(width, 0, 0);
        glVertex3f(width * 0.3f, height, 0);
        glVertex3f(-width * 0.3f, height, 0);
        glEnd();

        glPopMatrix();
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void drawTree(const Tree& tree) {
    glPushMatrix();
    glTranslatef(tree.position.x, tree.position.y, tree.position.z);

    // Trunk
    glColor3f(0.4f, 0.3f, 0.2f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    drawCylinder(tree.trunkRadius, tree.height * 0.6f, 16);
    glPopMatrix();

    // Canopy - multiple spheres for realistic tree
    glColor3f(0.2f, 0.5f, 0.2f);
    for (int i = 0; i < 5; i++) {
        glPushMatrix();
        float yOffset = tree.height * 0.4f + i * tree.height * 0.15f;
        float radiusScale = 1.0f - i * 0.15f;
        glTranslatef(0, yOffset, 0);
        drawSphere(tree.canopyRadius * radiusScale, 16, 16);
        glPopMatrix();
    }

    // Top accent - lighter green
    glColor3f(0.3f, 0.6f, 0.3f);
    glPushMatrix();
    glTranslatef(0, tree.height * 0.95f, 0);
    drawSphere(tree.canopyRadius * 0.4f, 12, 12);
    glPopMatrix();

    glPopMatrix();
}

void drawShop(const Shop& shop) {
    glPushMatrix();
    glTranslatef(shop.position.x, shop.position.y + shop.size.y / 2, shop.position.z);

    // Main building with texture-like detailing
    glColor3f(shop.color.x, shop.color.y, shop.color.z);
    glPushMatrix();
    glScalef(shop.size.x, shop.size.y, shop.size.z);
    drawCube(1.0f);
    glPopMatrix();

    // Decorative trim
    glColor3f(shop.color.x * 0.8f, shop.color.y * 0.8f, shop.color.z * 0.8f);
    glPushMatrix();
    glTranslatef(0, shop.size.y / 2, 0);
    glScalef(shop.size.x * 1.05f, 0.3f, shop.size.z * 1.05f);
    drawCube(1.0f);
    glPopMatrix();

    // Roof
    glColor3f(0.6f, 0.35f, 0.25f);
    glPushMatrix();
    glTranslatef(0, shop.size.y / 2 + 1.5f, 0);
    glScalef(1.0f, 0.3f, 1.0f);
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(shop.size.x * 0.8f, 3.0f, 20, 12);
    glPopMatrix();

    // Door
    glColor3f(0.45f, 0.25f, 0.15f);
    glPushMatrix();
    glTranslatef(0, -shop.size.y / 2 + 2.0f, shop.size.z / 2 + 0.15f);
    glScalef(2.0f, 4.0f, 0.3f);
    drawCube(1.0f);
    glPopMatrix();

    // Windows - multiple
    glColor3f(0.75f, 0.92f, 1.0f);
    for (int i = -1; i <= 1; i++) {
        if (i == 0) continue;
        glPushMatrix();
        glTranslatef(i * 2.5f, shop.size.y / 3, shop.size.z / 2 + 0.15f);
        glScalef(1.5f, 2.0f, 0.25f);
        drawCube(1.0f);
        glPopMatrix();
    }

    // Awning
    glColor3f(0.95f, 0.6f, 0.4f);
    glPushMatrix();
    glTranslatef(0, shop.size.y / 2.5f, shop.size.z / 2 + 2.0f);
    glScalef(shop.size.x * 1.2f, 0.25f, 3.0f);
    drawCube(1.0f);
    glPopMatrix();

    // Signage posts
    glColor3f(0.5f, 0.5f, 0.5f);
    for (int i = -1; i <= 1; i += 2) {
        glPushMatrix();
        glTranslatef(i * shop.size.x * 0.5f, shop.size.y / 2.5f, shop.size.z / 2 + 1.5f);
        glRotatef(-90, 1, 0, 0);
        drawCylinder(0.1f, 1.5f, 8);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawPerson(const Person& person) {
    glPushMatrix();
    glTranslatef(person.position.x, person.position.y + 3.5f, person.position.z);

    float angle = atan2(person.direction.z, person.direction.x) * 180.0f / M_PI;
    glRotatef(angle - 90, 0, 1, 0);

    // Body
    glColor3f(person.color.x, person.color.y, person.color.z);
    glPushMatrix();
    glScalef(1.2f, 2.2f, 0.8f);
    drawCube(1.0f);
    glPopMatrix();

    // Head
    glColor3f(0.95f, 0.85f, 0.75f);
    glPushMatrix();
    glTranslatef(0, 1.7f, 0);
    drawSphere(0.7f, 16, 16);
    glPopMatrix();

    // Legs with walking animation
    float legSwing = sin(person.walkCycle * M_PI / 180.0f) * 35.0f;
    glColor3f(0.25f, 0.25f, 0.35f);

    for (int leg = 0; leg < 2; leg++) {
        float swing = (leg == 0) ? legSwing : -legSwing;
        glPushMatrix();
        glTranslatef((leg == 0 ? -0.35f : 0.35f), -1.1f, 0);
        glRotatef(swing, 1, 0, 0);
        glTranslatef(0, -0.8f, 0);
        glScalef(0.35f, 1.6f, 0.35f);
        drawCube(1.0f);
        glPopMatrix();
    }

    // Arms with walking animation
    glColor3f(person.color.x * 0.85f, person.color.y * 0.85f, person.color.z * 0.85f);
    for (int arm = 0; arm < 2; arm++) {
        float swing = (arm == 0) ? -legSwing * 0.6f : legSwing * 0.6f;
        glPushMatrix();
        glTranslatef((arm == 0 ? -0.8f : 0.8f), 0.7f, 0);
        glRotatef(swing, 1, 0, 0);
        glTranslatef(0, -0.7f, 0);
        glScalef(0.28f, 1.4f, 0.28f);
        drawCube(1.0f);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawRollerCoasterTrack() {
    glDisable(GL_LIGHTING);

    // Draw support structure with lattice framework
    glColor3f(0.55f, 0.55f, 0.65f);
    glLineWidth(3.0f);
    for (size_t i = 0; i < splinePoints.size(); i += 12) {
        // Main support
        glBegin(GL_LINES);
        glVertex3f(splinePoints[i].x, splinePoints[i].y, splinePoints[i].z);
        glVertex3f(splinePoints[i].x, 0, splinePoints[i].z);
        glEnd();

        // Cross bracing
        if (i > 12 && i < splinePoints.size() - 12) {
            glBegin(GL_LINES);
            glVertex3f(splinePoints[i].x, splinePoints[i].y / 2, splinePoints[i].z);
            glVertex3f(splinePoints[i - 12].x, splinePoints[i - 12].y / 2, splinePoints[i - 12].z);
            glEnd();
        }
    }

    glEnable(GL_LIGHTING);

    // Draw thick cylindrical rails
    glColor3f(0.75f, 0.75f, 0.85f);
    for (int rail = 0; rail < 2; rail++) {
        float offset = (rail == 0) ? -0.6f : 0.6f;

        for (size_t i = 0; i < splinePoints.size() - 1; i++) {
            Vec3 pos1 = splinePoints[i] + splineBinormals[i] * offset;
            Vec3 pos2 = splinePoints[i + 1] + splineBinormals[i + 1] * offset;

            Vec3 dir = pos2 - pos1;
            float length = dir.length();
            dir = dir.normalize();

            glPushMatrix();
            glTranslatef(pos1.x, pos1.y, pos1.z);

            // Orient cylinder along track
            float angle = acos(dir.y) * 180.0f / M_PI;
            Vec3 axis = Vec3(0, 1, 0).cross(dir).normalize();
            if (axis.length() > 0.01f) {
                glRotatef(angle, axis.x, axis.y, axis.z);
            }

            drawCylinder(0.15f, length, 12);
            glPopMatrix();
        }
    }

    // Cross ties between rails
    glColor3f(0.65f, 0.65f, 0.75f);
    for (size_t i = 0; i < splinePoints.size(); i += 8) {
        Vec3 left = splinePoints[i] + splineBinormals[i] * (-0.6f);
        Vec3 right = splinePoints[i] + splineBinormals[i] * 0.6f;

        glPushMatrix();
        glTranslatef(left.x, left.y, left.z);

        Vec3 dir = (right - left).normalize();
        float angle = acos(dir.x) * 180.0f / M_PI;
        if (dir.z < 0) angle = -angle;
        glRotatef(angle, 0, 1, 0);
        glRotatef(90, 0, 1, 0);
        drawCylinder(0.12f, 1.2f, 12);
        glPopMatrix();
    }

    // Draw coaster cars
    for (int carIdx = 0; carIdx < coasterCarCount; carIdx++) {
        int idx = ((int)(coasterPosition + coasterCarOffsets[carIdx])) % splinePoints.size();
        Vec3 pos = splinePoints[idx];
        Vec3 tangent = splineTangents[idx];
        Vec3 normal = splineNormals[idx];
        Vec3 binormal = splineBinormals[idx];

        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);

        GLfloat matrix[16] = {
            binormal.x, binormal.y, binormal.z, 0,
            normal.x, normal.y, normal.z, 0,
            tangent.x, tangent.y, tangent.z, 0,
            0, 0, 0, 1
        };
        glMultMatrixf(matrix);

        // Car body - more detailed
        glColor3f(0.98f, 0.22f, 0.22f);
        glPushMatrix();
        glScalef(2.0f, 1.4f, 2.8f);
        drawCube(1.0f);
        glPopMatrix();

        // Car sides
        glColor3f(0.85f, 0.15f, 0.15f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 1.05f, 0, 0);
            glScalef(0.1f, 1.2f, 2.6f);
            drawCube(1.0f);
            glPopMatrix();
        }

        // Seats (2 rows of 2)
        glColor3f(0.35f, 0.35f, 0.45f);
        for (int row = -1; row <= 1; row += 2) {
            for (int seat = -1; seat <= 1; seat += 2) {
                glPushMatrix();
                glTranslatef(seat * 0.5f, 0.4f, row * 0.6f);
                glScalef(0.7f, 0.7f, 0.8f);
                drawCube(1.0f);
                glPopMatrix();
            }
        }

        // Wheels with axles
        glColor3f(0.2f, 0.2f, 0.2f);
        for (int i = -1; i <= 1; i += 2) {
            for (int j = -1; j <= 1; j += 2) {
                glPushMatrix();
                glTranslatef(i * 1.15f, -0.9f, j * 1.2f);
                glRotatef(90, 0, 0, 1);

                // Wheel
                drawCylinder(0.3f, 0.2f, 16);

                // Hub cap
                glColor3f(0.5f, 0.5f, 0.5f);
                glTranslatef(0, 0, 0.1f);
                drawSphere(0.15f, 12, 12);
                glColor3f(0.2f, 0.2f, 0.2f);

                glPopMatrix();
            }
        }

        // Safety bar
        glColor3f(0.4f, 0.4f, 0.5f);
        glPushMatrix();
        glTranslatef(0, 0.8f, 0);
        glScalef(1.8f, 0.15f, 2.4f);
        drawCube(1.0f);
        glPopMatrix();

        glPopMatrix();
    }
}

void drawFerrisWheel() {
    glPushMatrix();
    glTranslatef(40, 0, 45);

    // Enhanced base platform
    glColor3f(0.5f, 0.5f, 0.6f);
    glPushMatrix();
    glTranslatef(0, 0.3f, 0);
    glScalef(14, 0.6f, 14);
    drawCube(1.0f);
    glPopMatrix();

    // Support towers
    glColor3f(0.6f, 0.6f, 0.7f);
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(side * 3.0f, 0, 0);
        glRotatef(90, 0, 1, 0);
        glRotatef(-90, 1, 0, 0);
        drawCylinder(0.7f, ferrisWheelRadius + 8, 24);
        glPopMatrix();
    }

    // Wheel mechanism
    glPushMatrix();
    glTranslatef(0, ferrisWheelRadius + 2, 0);
    glRotatef(ferrisWheelRotation, 0, 0, 1);

    // Central hub
    glColor3f(0.85f, 0.75f, 0.65f);
    drawSphere(2.2f, 28, 28);

    // Spokes
    glColor3f(0.75f, 0.75f, 0.85f);
    for (int i = 0; i < cabinCount; i++) {
        glPushMatrix();
        glRotatef(i * 360.0f / cabinCount, 0, 0, 1);
        glRotatef(90, 0, 1, 0);
        drawCylinder(0.28f, ferrisWheelRadius, 24);
        glPopMatrix();
    }

    // Outer rim segments
    glColor3f(0.8f, 0.8f, 0.9f);
    for (int i = 0; i < cabinCount; i++) {
        float angle1 = (i * 360.0f / cabinCount) * M_PI / 180.0f;
        float angle2 = ((i + 1) * 360.0f / cabinCount) * M_PI / 180.0f;

        Vec3 p1(ferrisWheelRadius * cos(angle1), ferrisWheelRadius * sin(angle1), 0);
        Vec3 p2(ferrisWheelRadius * cos(angle2), ferrisWheelRadius * sin(angle2), 0);
        Vec3 dir = p2 - p1;

        glPushMatrix();
        glTranslatef(p1.x, p1.y, p1.z);

        float segAngle = atan2(dir.y, dir.x) * 180.0f / M_PI;
        glRotatef(segAngle, 0, 0, 1);
        glRotatef(90, 0, 1, 0);

        drawCylinder(0.22f, dir.length(), 20);
        glPopMatrix();
    }

    // Cabins
    for (int i = 0; i < cabinCount; i++) {
        glPushMatrix();
        glRotatef(i * 360.0f / cabinCount, 0, 0, 1);
        glTranslatef(ferrisWheelRadius, 0, 0);
        glRotatef(-ferrisWheelRotation, 0, 0, 1);

        // Cabin structure
        glColor3f(0.92f, 0.75f, 0.48f);
        glPushMatrix();
        glTranslatef(0, -2.2f, 0);
        glScalef(2.2f, 2.8f, 2.2f);
        drawCube(1.0f);
        glPopMatrix();

        // Windows
        glColor3f(0.78f, 0.95f, 1.0f);
        for (int side = 0; side < 4; side++) {
            glPushMatrix();
            glRotatef(side * 90, 0, 1, 0);
            glTranslatef(0, -2.2f, 1.15f);
            glScalef(1.7f, 2.2f, 0.15f);
            drawCube(1.0f);
            glPopMatrix();
        }

        // Roof
        glColor3f(0.98f, 0.8f, 0.52f);
        glPushMatrix();
        glTranslatef(0, -0.6f, 0);
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(1.7f, 1.5f, 24, 12);
        glPopMatrix();

        glPopMatrix();
    }

    glPopMatrix();
    glPopMatrix();
}

void drawCarousel() {
    glPushMatrix();
    glTranslatef(-35, 0, -18);

    // Enhanced base platform
    glColor3f(0.92f, 0.75f, 0.58f);
    glPushMatrix();
    glTranslatef(0, 1.0f, 0);
    glScalef(carouselRadius * 3.0f, 1.0f, carouselRadius * 3.0f);
    drawCube(1.0f);
    glPopMatrix();

    // Decorative base trim
    glColor3f(0.95f, 0.85f, 0.65f);
    glPushMatrix();
    glTranslatef(0, 1.6f, 0);
    glScalef(carouselRadius * 3.2f, 0.3f, carouselRadius * 3.2f);
    drawCube(1.0f);
    glPopMatrix();

    // Center pole with lights
    glColor3f(0.98f, 0.98f, 0.48f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    drawCylinder(1.2f, 16.0f, 28);
    glPopMatrix();

    // Rotating top section
    glPushMatrix();
    glTranslatef(0, 16, 0);
    glRotatef(carouselRotation, 0, 1, 0);

    // Canopy with decorations
    glColor3f(0.98f, 0.45f, 0.45f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(carouselRadius + 4, 5.0f, 28, 16);
    glPopMatrix();

    // Canopy rim
    glColor3f(1.0f, 0.92f, 0.42f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glTranslatef(0, 0, 4.8f);
    glutSolidTorus(0.4f, carouselRadius + 4, 20, 32);
    glPopMatrix();

    // Top ornament
    glColor3f(1.0f, 0.95f, 0.45f);
    glPushMatrix();
    glTranslatef(0, 5, 0);
    drawSphere(1.8f, 24, 24);
    glPopMatrix();

    // Carousel horses
    for (int i = 0; i < horseCount; i++) {
        glPushMatrix();
        glRotatef(i * 360.0f / horseCount, 0, 1, 0);
        glTranslatef(carouselRadius, 0, 0);

        // Bobbing motion
        float bob = sin((carouselRotation + i * 30.0f) * M_PI / 180.0f) * 2.8f;
        glTranslatef(0, -11 + bob, 0);

        // Pole
        glColor3f(0.98f, 0.98f, 0.48f);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        drawCylinder(0.28f, 13.0f, 20);
        glPopMatrix();

        // Horse body - varied colors
        float hue = i / (float)horseCount;
        glColor3f(0.85f + 0.15f * sin(hue * M_PI * 2),
                  0.75f + 0.25f * cos(hue * M_PI * 3),
                  0.85f);

        glPushMatrix();
        glScalef(2.0f, 1.7f, 3.4f);
        drawSphere(1.1f, 24, 24);
        glPopMatrix();

        // Head
        glPushMatrix();
        glTranslatef(0, 1.5f, 2.3f);
        glRotatef(30, 1, 0, 0);
        glScalef(1.2f, 1.4f, 1.8f);
        drawSphere(0.9f, 20, 20);
        glPopMatrix();

        // Ears
        glColor3f(0.95f, 0.85f, 0.95f);
        for (int ear = -1; ear <= 1; ear += 2) {
            glPushMatrix();
            glTranslatef(ear * 0.6f, 2.5f, 2.8f);
            glScalef(0.3f, 0.6f, 0.2f);
            drawSphere(0.5f, 12, 12);
            glPopMatrix();
        }

        // Legs with more detail
        glColor3f(0.95f, 0.95f, 0.95f);
        for (int leg = 0; leg < 4; leg++) {
            glPushMatrix();
            float legX = (leg < 2) ? -0.7f : 0.7f;
            float legZ = (leg % 2 == 0) ? 1.4f : -1.4f;
            glTranslatef(legX, -2.3f, legZ);
            glRotatef(-90, 1, 0, 0);
            drawCylinder(0.25f, 2.3f, 16);

            // Hooves
            glColor3f(0.2f, 0.2f, 0.2f);
            glTranslatef(0, 0, 2.3f);
            drawSphere(0.35f, 12, 12);
            glColor3f(0.95f, 0.95f, 0.95f);
            glPopMatrix();
        }

        // Saddle
        glColor3f(0.65f, 0.35f, 0.25f);
        glPushMatrix();
        glTranslatef(0, 1.0f, 0);
        glScalef(1.4f, 0.4f, 2.0f);
        drawCube(1.0f);
        glPopMatrix();

        // Mane
        glColor3f(0.3f, 0.25f, 0.2f);
        for (int m = 0; m < 3; m++) {
            glPushMatrix();
            glTranslatef(0, 2.0f - m * 0.4f, 2.5f - m * 0.3f);
            glScalef(0.8f, 0.3f, 0.4f);
            drawSphere(0.4f, 12, 12);
            glPopMatrix();
        }

        glPopMatrix();
    }

    glPopMatrix();
    glPopMatrix();
}

void updateParticles(float deltaTime) {
    if (!showParticles) return;

    for (auto& p : fountainParticles) {
        p.velocity.y -= 14.0f * deltaTime;
        p.position = p.position + p.velocity * deltaTime;
        p.life -= p.fade;

        if (p.life <= 0 || p.position.y < 0) {
            float angle = (rand() % 360) * M_PI / 180.0f;
            float speed = 7.0f + (rand() % 100) / 30.0f;
            p.position = Vec3(-45, 0, 45);
            p.velocity = Vec3(cos(angle) * speed * 0.5f, speed, sin(angle) * speed * 0.5f);
            p.life = 1.0f;
        }
    }

    for (auto it = fireworksParticles.begin(); it != fireworksParticles.end();) {
        it->velocity.y -= 14.0f * deltaTime;
        it->position = it->position + it->velocity * deltaTime;
        it->life -= it->fade;

        if (it->life <= 0) {
            it = fireworksParticles.erase(it);
        } else {
            ++it;
        }
    }
}

void updatePeople(float deltaTime) {
    if (!showPeople) return;

    for (auto& person : people) {
        Vec3 toTarget(person.targetX - person.position.x, 0, person.targetZ - person.position.z);
        float distToTarget = toTarget.length();

        if (distToTarget < 3.0f) {
            person.targetX = (rand() % 160) - 80;
            person.targetZ = (rand() % 160) - 80;
        } else {
            person.direction = toTarget.normalize();
            person.position = person.position + person.direction * (person.speed * deltaTime * animationSpeed);
        }

        person.walkCycle += person.speed * 90.0f * deltaTime * animationSpeed;
        if (person.walkCycle >= 360.0f) person.walkCycle -= 360.0f;
    }
}

void drawParticles() {
    if (!showParticles) return;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    glPointSize(6.0f);
    glBegin(GL_POINTS);

    for (const auto& p : fountainParticles) {
        glColor4f(p.color.x, p.color.y, p.color.z, p.life * 0.85f);
        glVertex3f(p.position.x, p.position.y, p.position.z);
    }

    for (const auto& p : fireworksParticles) {
        glColor4f(p.color.x, p.color.y, p.color.z, p.life);
        glVertex3f(p.position.x, p.position.y, p.position.z);
    }

    glEnd();

    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}

void drawGround() {
    glDisable(GL_LIGHTING);

    // Main grass ground
    glColor3f(0.35f, 0.65f, 0.35f);
    glBegin(GL_QUADS);
    glVertex3f(-150, 0, -150);
    glVertex3f(150, 0, -150);
    glVertex3f(150, 0, 150);
    glVertex3f(-150, 0, 150);
    glEnd();

    // Paved pathways
    glColor3f(0.75f, 0.75f, 0.68f);

    // Main pathway
    glBegin(GL_QUADS);
    glVertex3f(-100, 0.08f, -8);
    glVertex3f(100, 0.08f, -8);
    glVertex3f(100, 0.08f, 8);
    glVertex3f(-100, 0.08f, 8);
    glEnd();

    // Cross pathway
    glBegin(GL_QUADS);
    glVertex3f(-8, 0.08f, -100);
    glVertex3f(8, 0.08f, -100);
    glVertex3f(8, 0.08f, 100);
    glVertex3f(-8, 0.08f, 100);
    glEnd();

    // Pathway borders
    glColor3f(0.65f, 0.65f, 0.58f);
    glLineWidth(2.0f);

    glBegin(GL_LINE_LOOP);
    glVertex3f(-100, 0.09f, -8);
    glVertex3f(100, 0.09f, -8);
    glVertex3f(100, 0.09f, 8);
    glVertex3f(-100, 0.09f, 8);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(-8, 0.09f, -100);
    glVertex3f(8, 0.09f, -100);
    glVertex3f(8, 0.09f, 100);
    glVertex3f(-8, 0.09f, 100);
    glEnd();

    glEnable(GL_LIGHTING);
}

void setupLighting() {
    // Main sun light
    GLfloat light0Ambient[] = {0.5f, 0.5f, 0.6f, 1.0f};
    GLfloat light0Diffuse[] = {1.0f, 0.98f, 0.88f, 1.0f};
    GLfloat light0Specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light0Position[] = {80.0f, 120.0f, 60.0f, 0.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0Position);

    // Ferris wheel accent light
    GLfloat light1Diffuse[] = {0.95f, 0.6f, 1.0f, 1.0f};
    GLfloat light1Position[] = {40.0f, ferrisWheelRadius + 5, 45.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Diffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, light1Position);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.02f);

    // Carousel accent light
    GLfloat light2Diffuse[] = {1.0f, 0.85f, 0.5f, 1.0f};
    GLfloat light2Position[] = {-35.0f, 14.0f, -18.0f, 1.0f};
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light2Diffuse);
    glLightfv(GL_LIGHT2, GL_POSITION, light2Position);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.018f);

    // Fountain light
    GLfloat light3Diffuse[] = {0.7f, 0.9f, 1.0f, 1.0f};
    GLfloat light3Position[] = {-45.0f, 8.0f, 45.0f, 1.0f};
    glLightfv(GL_LIGHT3, GL_DIFFUSE, light3Diffuse);
    glLightfv(GL_LIGHT3, GL_POSITION, light3Position);
    glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.025f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Eye-level camera
    if (cameraMode == 0) {
        gluLookAt(
            cameraTarget.x + cameraDistance * cos(cameraAngleY * M_PI / 180.0f) * cos(cameraAngleX * M_PI / 180.0f),
            cameraTarget.y + cameraDistance * sin(cameraAngleX * M_PI / 180.0f) + cameraHeight,
            cameraTarget.z + cameraDistance * sin(cameraAngleY * M_PI / 180.0f),
            cameraTarget.x, cameraTarget.y + 8, cameraTarget.z,
            0, 1, 0
        );
    } else if (cameraMode == 1) {
        int idx = (int)coasterPosition % splinePoints.size();
        Vec3 pos = splinePoints[idx];
        Vec3 tangent = splineTangents[idx];
        Vec3 lookAt = pos + tangent * 10.0f;

        gluLookAt(
            pos.x, pos.y + 2.5f, pos.z,
            lookAt.x, lookAt.y, lookAt.z,
            0, 1, 0
        );
    } else if (cameraMode == 2) {
        gluLookAt(0, 140, 1, 0, 0, 0, 0, 0, -1);
    }

    drawSky();
    setupLighting();

    drawGround();
    drawGrass();

    if (showTrees) {
        for (const auto& tree : trees) {
            drawTree(tree);
        }
    }

    drawRollerCoasterTrack();
    drawFerrisWheel();
    drawCarousel();

    if (showShops) {
        for (const auto& shop : shops) {
            drawShop(shop);
        }
    }

    if (showPeople) {
        for (const auto& person : people) {
            drawPerson(person);
        }
    }

    drawParticles();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0f, (float)w / (float)h, 1.0f, 600.0f);
    glMatrixMode(GL_MODELVIEW);
}

void update(int value) {
    if (isAnimating) {
        float deltaTime = 0.016f * animationSpeed;

        coasterPosition += 3.0f * animationSpeed;
        if (coasterPosition >= splinePoints.size()) coasterPosition = 0;

        ferrisWheelRotation += 0.22f * animationSpeed;
        if (ferrisWheelRotation >= 360.0f) ferrisWheelRotation -= 360.0f;

        carouselRotation += 1.1f * animationSpeed;
        if (carouselRotation >= 360.0f) carouselRotation -= 360.0f;

        updateParticles(deltaTime);
        updatePeople(deltaTime);

        fireworkTimer += deltaTime;
        if (fireworkTimer > 2.0f && (rand() % 100) < 10) {
            Vec3 pos((rand() % 100) - 50, 35 + rand() % 15, (rand() % 100) - 50);
            createFirework(pos);
            fireworkTimer = 0;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'p': case 'P': isAnimating = !isAnimating; break;
        case '+': case '=': animationSpeed += 0.2f; break;
        case '-': case '_': if (animationSpeed > 0.2f) animationSpeed -= 0.2f; break;
        case 'c': case 'C': cameraMode = (cameraMode + 1) % 3; break;
        case 'f': case 'F': showParticles = !showParticles; break;
        case 'h': case 'H': showPeople = !showPeople; break;
        case 'b': case 'B': showShops = !showShops; break;
        case 'g': case 'G': showGrass = !showGrass; break;
        case 't': case 'T': showTrees = !showTrees; break;
        case 'w': case 'W': cameraAngleX += 5.0f; break;
        case 's': case 'S': cameraAngleX -= 5.0f; break;
        case 'a': case 'A': cameraAngleY -= 5.0f; break;
        case 'd': case 'D': cameraAngleY += 5.0f; break;
        case 'z': case 'Z': if (cameraDistance > 30.0f) cameraDistance -= 5.0f; break;
        case 'x': case 'X': if (cameraDistance < 300.0f) cameraDistance += 5.0f; break;
        case 'r': case 'R': cameraDistance = 55.0f; cameraAngleX = 15.0f; cameraAngleY = 45.0f; break;
        case 27: exit(0); break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1600, 900);
    glutCreateWindow("Professional Theme Park - OpenGL");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, update, 0);

    printf("======== PROFESSIONAL THEME PARK ========\n");
    printf("P       - Pause/Resume\n");
    printf("+/-     - Speed\n");
    printf("C       - Camera (Free/Coaster/Aerial)\n");
    printf("WASD    - Rotate camera\n");
    printf("Z/X     - Zoom\n");
    printf("F       - Particles\n");
    printf("H       - People\n");
    printf("B       - Shops\n");
    printf("G       - Grass\n");
    printf("T       - Trees\n");
    printf("R       - Reset camera\n");
    printf("ESC     - Exit\n");
    printf("=========================================\n");

    glutMainLoop();
    return 0;
}
