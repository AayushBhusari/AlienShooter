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
        // Direction the bullet travels
        Vector2 direction = { cosf(angle), sinf(angle) };

        // Offset to the muzzle from center (50 pixels to the right)
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
    Texture2D chillGuy = LoadTexture("assets/chill-guy-gun.png");
    chillGuy.height = 100 / 1.2;
    chillGuy.width = 100 / 1.2;


    vector<Vector2> bulletPositions;
    vector<Vector2> bulletVelocities;

    float shootCooldown = 0.3f; // Seconds between shots
    float shootTimer = 0.0f;    // Time left until the next shot

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

        // Handle shooting
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

        // Draw everything
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw rotated chillGuy centered on player position
        Rectangle srcRect = { 0, 0, (float)chillGuy.width, (float)chillGuy.height };
        Vector2 origin = { chillGuy.width / 2.0f, chillGuy.height / 2.0f };
        DrawTexturePro(chillGuy, srcRect, { guy.position.x, guy.position.y, (float)chillGuy.width, (float)chillGuy.height }, origin, angleDegrees, WHITE);

        // Draw bullets
        for (auto& bullet : bulletPositions) {
            DrawCircleV(bullet, 5, RED);
        }

        EndDrawing();
    }

    UnloadTexture(chillGuy);
    CloseWindow();
    return 0;
}
