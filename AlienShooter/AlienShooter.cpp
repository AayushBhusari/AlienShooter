#include <iostream>
#include <vector>
#include <raylib.h>
#include "raymath.h"
#include <cmath>
const Color LIGHTGREEN = { 144, 238, 144, 255 }; // RGBA
const Color DARKRED = { 139, 0, 0, 255 };
using namespace std;

// --- Classes ---
class Guy
{
public:
    Vector2 position = { 600, 400 };

    void Shoot(float angle, vector<Vector2>& bulletPositions, vector<Vector2>& bulletVelocities)
    {
        Vector2 direction = { cosf(angle), sinf(angle) };
        float muzzleDistance = 50.0f;
        Vector2 offset = { cosf(angle) * muzzleDistance, sinf(angle) * muzzleDistance };

        Vector2 bulletStart = Vector2Add(position, offset);
        bulletPositions.push_back(bulletStart);
        bulletVelocities.push_back(Vector2Scale(direction, 500.0f));
    }
};
// Enemy Class
class Enemy
{
public:
    Vector2 position;
    float speed;
    float scale = 0.3f;
    float hitboxWidthFactor = 0.5f;  // Adjust width
    float hitboxHeightFactor = 0.7f; // Adjust height
    Rectangle hitbox;

    Enemy(Texture2D& texture)
    {
        position.x = static_cast<float>(GetRandomValue(100, GetScreenWidth() - 100));
        position.y = static_cast<float>(GetRandomValue(100, GetScreenHeight() - 100));
        speed = static_cast<float>(GetRandomValue(50, 150));

        UpdateHitbox(texture);
    }

    void UpdateHitbox(Texture2D& texture)
    {
        float width = texture.width * scale * hitboxWidthFactor;
        float height = texture.height * scale * hitboxHeightFactor;
        hitbox = { position.x - width / 2, position.y - height / 2, width, height };
    }

    void Draw(Texture2D& texture, Vector2 playerPos)
    {
        UpdateHitbox(texture);

        bool facingRight = playerPos.x > position.x;

        Rectangle sourceRec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
        if (facingRight) sourceRec.width *= -1;

        Rectangle destRec = { position.x, position.y, texture.width * scale, texture.height * scale };
        Vector2 origin = { texture.width * scale / 2, texture.height * scale / 2 };

        DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, WHITE);

        // Debug: Draw hitbox
        DrawRectangleLines((int)hitbox.x, (int)hitbox.y, (int)hitbox.width, (int)hitbox.height, RED);
    }

    void MoveTowards(Vector2 target, float deltaTime, float moveSpeed)
    {
        Vector2 direction = Vector2Normalize(Vector2Subtract(target, position));
        position = Vector2Add(position, Vector2Scale(direction, moveSpeed * deltaTime));
    }
};


enum GameState { MENU, GAMEPLAY, GAMEOVER, EXIT };

int main()
{
    // --- Window & Audio ---
    InitWindow(1200, 800, "Skibiddi Shooter");
    InitAudioDevice();

    GameState currentState = MENU;
    GameState previousState = EXIT;

    // --- Load Textures ---
    Texture2D chillGuyRight = LoadTexture("assets/shooting-right.png");
    Texture2D background = LoadTexture("assets/bg.png");
    Texture2D enemyTexture = LoadTexture("assets/enemy.png");

    // Flip right texture to create left
    Image img = LoadImage("assets/shooting-right.png");
    ImageFlipHorizontal(&img);
    Texture2D chillGuyLeft = LoadTextureFromImage(img);
    UnloadImage(img);

    chillGuyRight.width = chillGuyRight.height = 120;
    chillGuyLeft.width = chillGuyLeft.height = 120;

    // --- Load Music & Sounds ---
    Music menuMusic = LoadMusicStream("assets/menu-music.mp3");
    Music bgMusic = LoadMusicStream("assets/game-bg.mp3");
    Music gameOverMusic = LoadMusicStream("assets/game-over-music.mp3");
    Sound shootSFX = LoadSound("assets/player-shoot.mp3");
    Sound enemyDieSFX = LoadSound("assets/enemy-die.mp3");
    Sound playerDieSFX = LoadSound("assets/player-die.mp3");

    SetMusicVolume(menuMusic, 0.6f);
    SetMusicVolume(bgMusic, 0.5f);
    SetMusicVolume(gameOverMusic, 0.6f);

    Guy guy;
    float speed = 200.0f;
    float enemySpeed = 60.0f;
    int currentWave = 1;
    float waveDelay = 2.0f;
    float waveTimer = 0.0f;
    bool waveInProgress = true;

    vector<Vector2> bulletPositions;
    vector<Vector2> bulletVelocities;
    float shootCooldown = 0.3f;
    float shootTimer = 0.0f;

    vector<Enemy> enemies;

    // --- Spawn Wave ---
    auto spawnWave = [&](int wave)
        {
            int enemyCount = 3 + wave * 2;
            float minDist = 200.0f;
            for (int i = 0; i < enemyCount; ++i)
            {
                Enemy e(enemyTexture);
                while (Vector2Distance(e.position, guy.position) < minDist)
                {
                    e.position.x = static_cast<float>(GetRandomValue(100, GetScreenWidth() - 100));
                    e.position.y = static_cast<float>(GetRandomValue(100, GetScreenHeight() - 100));
                }
                enemies.push_back(e);
            }
        };
    spawnWave(currentWave);

    int score = 0;

    // --- Game Loop ---
    while (!WindowShouldClose() && currentState != EXIT)
    {
        float deltaTime = GetFrameTime();

        // Update music streams for smooth playback
        UpdateMusicStream(menuMusic);
        UpdateMusicStream(bgMusic);
        UpdateMusicStream(gameOverMusic);

        // Handle music on state change
        if (currentState != previousState)
        {
            switch (currentState)
            {
            case MENU:
                StopMusicStream(bgMusic);
                StopMusicStream(gameOverMusic);
                if (!IsMusicStreamPlaying(menuMusic)) PlayMusicStream(menuMusic);
                break;
            case GAMEPLAY:
                StopMusicStream(menuMusic);
                StopMusicStream(gameOverMusic);
                if (!IsMusicStreamPlaying(bgMusic)) PlayMusicStream(bgMusic);
                break;
            case GAMEOVER:
                StopMusicStream(menuMusic);
                StopMusicStream(bgMusic);
                if (!IsMusicStreamPlaying(gameOverMusic)) PlayMusicStream(gameOverMusic);
                break;
            case EXIT:
                StopMusicStream(menuMusic);
                StopMusicStream(bgMusic);
                StopMusicStream(gameOverMusic);
                break;
            }
            previousState = currentState;
        }

        // --- State Handling ---
        switch (currentState)
        {
        case MENU:
        {
            BeginDrawing();
            ClearBackground(DARKBLUE);
            DrawText("SKIBIDDI SHOOTER", 400, 200, 50, YELLOW);
            Rectangle playButton = { 500, 350, 200, 60 };
            Rectangle exitButton = { 500, 450, 200, 60 };
            DrawRectangleRec(playButton, LIGHTGRAY);
            DrawRectangleRec(exitButton, LIGHTGRAY);
            DrawText("PLAY", playButton.x + 60, playButton.y + 15, 30, BLACK);
            DrawText("EXIT", exitButton.x + 70, exitButton.y + 15, 30, BLACK);

            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, playButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                currentState = GAMEPLAY;
            if (CheckCollisionPointRec(mouse, exitButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                currentState = EXIT;

            EndDrawing();
        }
        break;

        case GAMEPLAY:
        {
            // Movement
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    guy.position.y -= speed * deltaTime;
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  guy.position.y += speed * deltaTime;
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) guy.position.x += speed * deltaTime;
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  guy.position.x -= speed * deltaTime;

            Vector2 mousePos = GetMousePosition();
            float angle = atan2(mousePos.y - guy.position.y, mousePos.x - guy.position.x);
            float angleDegrees = angle * RAD2DEG;

            if (shootTimer > 0.0f) shootTimer -= deltaTime;
            if (shootTimer <= 0.0f && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
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
            for (size_t i = 0; i < bulletPositions.size(); ++i)
            {
                bulletPositions[i].x += bulletVelocities[i].x * deltaTime;
                bulletPositions[i].y += bulletVelocities[i].y * deltaTime;
            }

            // Remove offscreen bullets
            for (int i = bulletPositions.size() - 1; i >= 0; --i)
            {
                if (bulletPositions[i].x < 0 || bulletPositions[i].x > GetScreenWidth() ||
                    bulletPositions[i].y < 0 || bulletPositions[i].y > GetScreenHeight())
                {
                    bulletPositions.erase(bulletPositions.begin() + i);
                    bulletVelocities.erase(bulletVelocities.begin() + i);
                }
            }

            // Bullet-enemy collision
           // Bullet-enemy collision (rectangle version)
            for (int i = bulletPositions.size() - 1; i >= 0; --i)
            {
                bool hit = false;
                for (int j = enemies.size() - 1; j >= 0; --j)
                {
                    Rectangle bulletRect = { bulletPositions[i].x - 2.5f, bulletPositions[i].y - 2.5f, 5, 5 };
                    if (CheckCollisionRecs(bulletRect, enemies[j].hitbox))
                    {
                        PlaySound(enemyDieSFX);
                        score++;
                        enemies.erase(enemies.begin() + j);
                        hit = true;
                        break;
                    }
                }
                if (hit)
                {
                    bulletPositions.erase(bulletPositions.begin() + i);
                    bulletVelocities.erase(bulletVelocities.begin() + i);
                }
            }


            // Enemies move & check collision
            for (auto& e : enemies)
            {
                e.MoveTowards(guy.position, deltaTime, enemySpeed);
                if (CheckCollisionCircles(e.position, 25.0f, guy.position, 30.0f))
                {
                    PlaySound(playerDieSFX);
                    currentState = GAMEOVER;
                }
            }

            // Wave logic
            if (enemies.empty() && waveTimer <= 0.0f && waveInProgress)
            {
                waveInProgress = false;
                waveTimer = waveDelay;
            }
            if (!waveInProgress)
            {
                waveTimer -= deltaTime;
                if (waveTimer <= 0.0f)
                {
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
            for (auto& bullet : bulletPositions)
            {
                DrawCircleV(bullet, 5, RED);
            }

            // Draw Enemies
            for (auto& enemy : enemies)
            {
                enemy.Draw(enemyTexture, guy.position);
                enemy.MoveTowards(guy.position, deltaTime, enemySpeed);

                //// Debug: show hitbox
                //DrawRectangleLines(
                //    (int)enemy.hitbox.x,
                //    (int)enemy.hitbox.y,
                //    (int)enemy.hitbox.width,
                //    (int)enemy.hitbox.height,
                //    RED
                //);

                // Player collision check
                Rectangle playerRect = { guy.position.x - 30, guy.position.y - 30, 60, 60 };
                if (CheckCollisionRecs(playerRect, enemy.hitbox))
                {
                    PlaySound(playerDieSFX);
                    currentState = GAMEOVER;
                }
            }

            EndDrawing();
        }
        break;

        case GAMEOVER:
        {
            BeginDrawing();
            ClearBackground(DARKRED);

            DrawText("GAME OVER", 450, 200, 60, YELLOW);
            DrawText(TextFormat("Final Score: %d", score), 500, 300, 40, WHITE);

            Rectangle restartButton = { 450, 400, 300, 60 };
            Rectangle exitButton = { 450, 500, 300, 60 };

            DrawRectangleRec(restartButton, LIGHTGRAY);
            DrawRectangleRec(exitButton, LIGHTGRAY);
            DrawText("RESTART", restartButton.x + 60, restartButton.y + 15, 30, BLACK);
            DrawText("EXIT", exitButton.x + 100, exitButton.y + 15, 30, BLACK);

            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, restartButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                guy.position = { 600, 400 };
                bulletPositions.clear();
                bulletVelocities.clear();
                enemies.clear();
                currentWave = 1;
                score = 0;
                waveInProgress = true;
                waveTimer = 0.0f;
                spawnWave(currentWave);
                currentState = GAMEPLAY;
            }
            if (CheckCollisionPointRec(mouse, exitButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                currentState = EXIT;

            EndDrawing();
        }
        break;

        case EXIT:
            break;
        }
    }

    // --- Unload everything ---
    UnloadTexture(chillGuyRight);
    UnloadTexture(chillGuyLeft);
    UnloadTexture(background);
    UnloadTexture(enemyTexture);

    UnloadSound(shootSFX);
    UnloadSound(enemyDieSFX);
    UnloadSound(playerDieSFX);

    StopMusicStream(menuMusic);
    StopMusicStream(bgMusic);
    StopMusicStream(gameOverMusic);

    UnloadMusicStream(menuMusic);
    UnloadMusicStream(bgMusic);
    UnloadMusicStream(gameOverMusic);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}