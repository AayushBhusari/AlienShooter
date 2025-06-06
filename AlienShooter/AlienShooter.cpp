#include <iostream>
#include <vector>
#include <raylib.h>
#include "raymath.h"
#include <cmath>
using namespace std;

class Guy {
public:
    Vector2 position = { 600, 400 };

    void Shoot(float angle, vector<Vector2>& bulletPositions, vector<Vector2>& bulletVelocities) {
        Vector2 direction = { cosf(angle), sinf(angle) };
        float muzzleDistance = 50.0f;
        Vector2 offset = { cosf(angle) * muzzleDistance, sinf(angle) * muzzleDistance };

        Vector2 bulletStart = Vector2Add(position, offset);
        bulletPositions.push_back(bulletStart);
        bulletVelocities.push_back(Vector2Scale(direction, 500.0f));
    }
};

int main() {
    InitWindow(1200, 800, "Skibiddi Shooter");

    Guy guy;
    float speed = 200.0f;

    // Load textures
    Texture2D chillGuyRight = LoadTexture("assets/shooting-right.png");
    Texture2D background = LoadTexture("assets/bg.png");

    // Flip the right-facing image to create the left-facing one
    Image img = LoadImage("assets/shooting-right.png");
    ImageFlipHorizontal(&img);
    Texture2D chillGuyLeft = LoadTextureFromImage(img);
    UnloadImage(img);

    chillGuyRight.height = 120;
    chillGuyRight.width = 120;
    chillGuyLeft.height = 120;
    chillGuyLeft.width = 120;

    vector<Vector2> bulletPositions;
    vector<Vector2> bulletVelocities;

    float shootCooldown = 0.3f;
    float shootTimer = 0.0f;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Movement
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) guy.position.y -= speed * deltaTime;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) guy.position.y += speed * deltaTime;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) guy.position.x += speed * deltaTime;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) guy.position.x -= speed * deltaTime;

        Vector2 mousePos = GetMousePosition();
        float angle = atan2(mousePos.y - guy.position.y, mousePos.x - guy.position.x);
        float angleDegrees = angle * RAD2DEG;

        // Update cooldown timer
        if (shootTimer > 0.0f) {
            shootTimer -= deltaTime;
        }

        // Shooting
        if (shootTimer <= 0.0f && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            guy.Shoot(angle, bulletPositions, bulletVelocities);
            shootTimer = shootCooldown;
        }

        // Wrap-around
        if (guy.position.x < 0) guy.position.x = GetScreenWidth();
        if (guy.position.x > GetScreenWidth()) guy.position.x = 0;
        if (guy.position.y < 0) guy.position.y = GetScreenHeight();
        if (guy.position.y > GetScreenHeight()) guy.position.y = 0;

        // Update bullets
        for (size_t i = 0; i < bulletPositions.size(); ++i) {
            bulletPositions[i].x += bulletVelocities[i].x * deltaTime;
            bulletPositions[i].y += bulletVelocities[i].y * deltaTime;
        }

        // Remove offscreen bullets
        for (int i = bulletPositions.size() - 1; i >= 0; --i) {
            if (bulletPositions[i].x < 0 || bulletPositions[i].x > GetScreenWidth() ||
                bulletPositions[i].y < 0 || bulletPositions[i].y > GetScreenHeight()) {
                bulletPositions.erase(bulletPositions.begin() + i);
                bulletVelocities.erase(bulletVelocities.begin() + i);
            }
        }

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle src = { 0, 0, (float)background.width, (float)background.height };
        Rectangle dest = { 0, 0, 1200, 800 };  // Match your window size
        Vector2 bgOrigin = { 0, 0 };

        DrawTexturePro(background, src, dest, bgOrigin, 0.0f, WHITE);

        // Determine facing direction
        bool facingLeft = mousePos.x < guy.position.x;
        Texture2D currentTexture = facingLeft ? chillGuyLeft : chillGuyRight;

        // Adjust angle to prevent upside-down when facing left
        float adjustedAngle = facingLeft ? angleDegrees + 180.0f : angleDegrees;

        Rectangle srcRect = { 0, 0, (float)currentTexture.width, (float)currentTexture.height };
        Vector2 origin = { currentTexture.width / 2.0f, currentTexture.height / 2.0f };

        DrawTexturePro(currentTexture,
            srcRect,
            { guy.position.x, guy.position.y, (float)currentTexture.width, (float)currentTexture.height },
            origin,
            adjustedAngle,
            WHITE);

        for (auto& bullet : bulletPositions) {
            DrawCircleV(bullet, 5, RED);
        }

        EndDrawing();
    }

    UnloadTexture(chillGuyRight);
    UnloadTexture(chillGuyLeft);
    UnloadTexture(background);

    CloseWindow();
    return 0;
}
