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
// Enemy Class
//
class Enemy {
public:
    Vector2 position;
    float speed;
    float scale = 0.3f;
    Enemy(Texture2D& texture) {
        position.x = static_cast<float>(GetRandomValue(100, GetScreenWidth() - 100));
        position.y = static_cast<float>(GetRandomValue(100, GetScreenHeight() - 100));
        speed = static_cast<float>(GetRandomValue(50, 150));
    }

    void Draw(Texture2D& texture, Vector2 playerPos) {
        // Determine if the player is to the right of the enemy
        bool facingRight = playerPos.x > position.x;

        // Source rectangle: defines which part of the texture to draw
        Rectangle sourceRec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };

        // Flip horizontally if facing right (since texture faces left by default)
        if (facingRight) {
            sourceRec.width *= -1;
        }

        // Destination rectangle: where to draw on the screen and its size
        Rectangle destRec = {
            position.x,
            position.y,
            texture.width * scale,
            texture.height * scale
        };

        // Origin: the center of the sprite (so flipping looks natural)
        Vector2 origin = { texture.width * scale / 2, texture.height * scale / 2 };

        // Draw the texture with flipping
        DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, WHITE);
    }


    void MoveTowards(Vector2 target, float deltaTime, float moveSpeed) {
        Vector2 direction = Vector2Normalize(Vector2Subtract(target, position));
        position = Vector2Add(position, Vector2Scale(direction, moveSpeed * deltaTime));
    }


};


int main() {
    InitWindow(1200, 800, "Skibiddi Shooter");

    Guy guy;
    float speed = 200.0f;

    float enemySpeed = 60.0f;


    int currentWave = 1;
    float waveDelay = 2.0f;
    float waveTimer = 0.0f;
    bool waveInProgress = true;


    // Load textures
    Texture2D chillGuyRight = LoadTexture("assets/shooting-right.png");
    Texture2D background = LoadTexture("assets/bg.png");
    Texture2D enemyTexture = LoadTexture("assets/enemy.png");

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

   
    // ------------------------
 // Spawn Wave Function
 // ------------------------
    vector<Enemy> enemies;
    auto spawnWave = [&](int wave) {
        int enemyCount = 3 + wave * 2; // Increase number of enemies per wave
        float minSpawnDistance = 200.0f; // Minimum distance from player

        for (int i = 0; i < enemyCount; ++i) {
            Enemy enemy(enemyTexture);

            // Keep generating a position until it's far enough from the player
            while (Vector2Distance(enemy.position, guy.position) < minSpawnDistance) {
                enemy.position.x = static_cast<float>(GetRandomValue(100, GetScreenWidth() - 100));
                enemy.position.y = static_cast<float>(GetRandomValue(100, GetScreenHeight() - 100));
            }

            enemies.push_back(enemy);
        }
        };


    // Initial wave
    spawnWave(currentWave);

	// Load sound
    InitAudioDevice();
    Sound enemyDieSFX = LoadSound("assets/enemy-die.mp3");
	Sound shootSFX = LoadSound("assets/player-shoot.mp3");
    Sound playerDieSFX = LoadSound("assets/player-die.mp3");
    Sound bgSound = LoadSound("assets/game-bg.mp3");

    SetSoundVolume(bgSound, 0.5f);  // optional
    PlaySound(bgSound);




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
            PlaySound(shootSFX);
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

        // ------------------------
// Check Bullet-Enemy Collision & Remove
// ------------------------
        for (int i = bulletPositions.size() - 1; i >= 0; --i) {
            bool hit = false;
            for (int j = enemies.size() - 1; j >= 0; --j) {
                float enemyRadius = (enemyTexture.width * enemies[j].scale) / 2.5f;

                if (CheckCollisionCircles(bulletPositions[i], 5.0f, enemies[j].position, enemyRadius)) {
                    // ✅ Play sound
                    PlaySound(enemyDieSFX);

                    // ✅ Add particle burst (simple visual effect)
                    for (int k = 0; k < 10; ++k) {
                        Vector2 particlePos = enemies[j].position;
                        Vector2 velocity = {
                            (float)GetRandomValue(-100, 100) / 100.0f,
                            (float)GetRandomValue(-100, 100) / 100.0f
                        };
                        DrawCircleV(Vector2Add(particlePos, Vector2Scale(velocity, 20)), 3, ORANGE);
                    }

                    enemies.erase(enemies.begin() + j);  // Remove enemy
                    hit = true;
                    break;
                }
            }
            if (hit) {
                bulletPositions.erase(bulletPositions.begin() + i);
                bulletVelocities.erase(bulletVelocities.begin() + i);
            }
        }


        // ------------------------
// Wave Check Logic
// ------------------------
        if (enemies.empty() && waveTimer <= 0.0f && waveInProgress) {
            waveInProgress = false;
            waveTimer = waveDelay;
        }

        if (!waveInProgress) {
            waveTimer -= deltaTime;
            if (waveTimer <= 0.0f) {
                currentWave++;
                spawnWave(currentWave);
                waveInProgress = true;
            }
        }



        // Draw everything
        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle src = { 0, 0, (float)background.width, (float)background.height };
        Rectangle dest = { 0, 0, 1200, 800 };
        Vector2 bgOrigin = { 0, 0 };
        DrawTexturePro(background, src, dest, bgOrigin, 0.0f, WHITE);
        DrawText(TextFormat("Wave: %d", currentWave), 20, 20, 30, YELLOW);


        // Determine facing direction
        bool facingLeft = mousePos.x < guy.position.x;
        Texture2D currentTexture = facingLeft ? chillGuyLeft : chillGuyRight;
        float adjustedAngle = facingLeft ? angleDegrees + 180.0f : angleDegrees;

        Rectangle srcRect = { 0, 0, (float)currentTexture.width, (float)currentTexture.height };
        Vector2 origin = { currentTexture.width / 2.0f, currentTexture.height / 2.0f };

        DrawTexturePro(currentTexture,
            srcRect,
            { guy.position.x, guy.position.y, (float)currentTexture.width, (float)currentTexture.height },
            origin,
            adjustedAngle,
            WHITE);

        // Draw bullets
        for (auto& bullet : bulletPositions) {
            DrawCircleV(bullet, 5, RED);
        }

        // ------------------------
        // Draw Enemies
        // ------------------------
        for (auto& enemy : enemies) {
            enemy.Draw(enemyTexture, guy.position);
            enemy.MoveTowards(guy.position, deltaTime, enemySpeed);

            // Optional: debug circle
            DrawCircleLines((int)enemy.position.x, (int)enemy.position.y, 32.0f, GREEN);

            // ✅ Player collision check
            if (CheckCollisionCircles(enemy.position, 25.0f, guy.position, 30.0f)) {
                PlaySound(playerDieSFX);
                // Optionally handle player respawn or game over
                //reset here
            }
        }




        EndDrawing();
    }

    // Unload resources
    UnloadTexture(chillGuyRight);
    UnloadTexture(chillGuyLeft);
    UnloadTexture(background);
    UnloadTexture(enemyTexture);
    UnloadSound(enemyDieSFX);
    UnloadSound(shootSFX);
    UnloadSound(playerDieSFX);
    UnloadSound(bgSound);
    CloseAudioDevice();


    CloseWindow();
    return 0;
}
