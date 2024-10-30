/*******************************************************************************************
*
*   raylib gamejam template
*
*   Template originally created with raylib 4.5-dev, last time updated with raylib 5.0
*
*   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 
#include <string.h>                         // Required for: 

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

#define LOGO_DISPLAY_FRAMES 120
#define MAX_DRONES 10
#define MAX_ENEMIES 100


//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { 
    SCREEN_LOGO = 0, 
    SCREEN_TITLE, 
    SCREEN_GAMEPLAY, 
    SCREEN_ENDING
} GameScreen;

typedef struct Shot {
    Vector2 position;
    Vector2 velocity;
}Shot;

typedef enum {
    MODULE_BEHAVIOUR,
    MODULE_SHOT_MODIFIER,
    MODULE_BOOSTER,
}ModuleType;

typedef struct Module {
    ModuleType type;

};

typedef struct Drone {
    bool alive;

    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    float dampening;
    float maxAccel;
    float maxVelocity;
    float jerk;

    int size;
    Color color;
    float playerMinDistance;
    float playerMaxDistance;

    int framesSinceShotFired;
    int shotCooldownFrames;
    float detectRange;
    float facingAngle;
    bool canShoot;

}Drone;

typedef struct Enemy {
    bool alive;
    Vector2 position;
    int size; 
}Enemy;

typedef struct Player {
    Vector2 position;
    Vector2 velocity;
    float acceleration;
    float dampening;
    float maxVelocity;
    int size;
    Rectangle hitbox;
    Drone drones[MAX_DRONES];
}Player;



// TODO: Define your custom data types here

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

Camera2D camera = {0};
GameScreen currentGameScreen = SCREEN_LOGO;
int globalFrameCounter = 0;
Player player = { 0 };
Enemy enemyArray[MAX_ENEMIES];

static RenderTexture2D target = { 0 };  // Render texture to render our game

// TODO: Define global variables here, recommended to make them static

void UpdateDrawDrone(Drone* drone);

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame
void InitPlayer();
void InitDrones();
void InitEnemies();

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "TBD Game Name");

    InitPlayer();
    InitDrones();
    InitEnemies();
    
    // Render texture to draw full screen, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);     // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);
    
    // TODO: Unload all loaded resources at this point

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------

void InitPlayer() {
    // Init Player 
    player.position = (Vector2){ 384.0f , 400.0f };
    player.size = 32;
    player.acceleration = 2500.0f;
    player.maxVelocity = 650.0f;
    player.dampening = 0.90f;
    player.hitbox = (Rectangle){
        player.position.x,
        player.position.y,
        player.position.x + player.size,
        player.position.y + player.size
    };
}

void InitDrones(){
    // Init Drones

    //Init dead drones
    for (size_t i = 0; i < MAX_DRONES; i++)
    {
        player.drones[i] = (Drone){
            .alive = false,
        };
    }

    // Test Drones
    player.drones[0] = (Drone){
        .alive = true,
        .position = player.position,
        .jerk = 5000.0f,
        .maxAccel = 2000.0f,
        .maxVelocity = 700.0f,
        .dampening = 0.90f,
        .framesSinceShotFired = 0,
        .shotCooldownFrames = 60,
        .size = 20,
        .color = BLUE,
        .playerMaxDistance = 80.0f,
        .playerMinDistance = 40.0f,
        .detectRange = 100.0f,
        .facingAngle = 0.0f,
    };

    player.drones[1] = (Drone){
        .alive = true,
        .position = Vector2Add(player.position, (Vector2) { 20.0f, 20.f }),
        .jerk = 5000.0f,
        .maxAccel = 2000.0f,
        .maxVelocity = 700.0f,
        .dampening = 0.90f,
        .framesSinceShotFired = 0,
        .shotCooldownFrames = 60,
        .size = 20,
        .color = PINK,
        .playerMaxDistance = 80.0f,
        .playerMinDistance = 40.0f,
        .detectRange = 100.0f,
        .facingAngle = 0.0f,
    };
}

void InitEnemies() {
    for (size_t i = 0; i < MAX_ENEMIES; i++)
    {
        enemyArray[i].alive = false;
    }
    for (size_t i = 0; i < 10; i++)
    {
        enemyArray[i].alive = true;
        enemyArray[i].position = (Vector2){ 200 + i * 40, 200 };
        enemyArray[i].size = 16;
    }
}

void DrawLogoScreen() {
    BeginTextureMode(target);
        ClearBackground(RAYWHITE);
        // TODO: Draw your game screen here
        DrawText("Welcome to raylib NEXT gamejam!", 150, 140, 30, BLACK);
        DrawRectangleLinesEx((Rectangle) { 0, 0, screenWidth, screenHeight }, 16, BLACK);
    EndTextureMode();

    BeginDrawing();
        ClearBackground(RAYWHITE);
        // Draw render texture to screen, scaled if required
        DrawTexturePro(target.texture, (Rectangle) { 0, 0, (float)target.texture.width, -(float)target.texture.height }, (Rectangle) { 0, 0, (float)target.texture.width, (float)target.texture.height }, (Vector2) { 0, 0 }, 0.0f, WHITE);
        // TODO: Draw everything that requires to be drawn at this point, maybe UI?
    EndDrawing();
}

void DrawMenuScreen() {
    
}

Vector2 CalcCenter(Vector2 position, int size) {
    return Vector2Add(position, (Vector2) { size / 2, size / 2 });
}

void MoveShip() {
    float delta = GetFrameTime();
    Vector2 direction = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_A))
        direction.x = direction.x - 1;
    if (IsKeyDown(KEY_S))
        direction.y = direction.y + 1;
    if (IsKeyDown(KEY_D))
        direction.x = direction.x + 1;
    if (IsKeyDown(KEY_W))
        direction.y = direction.y - 1;
        
    if (Vector2LengthSqr(direction) < EPSILON){
        player.velocity = Vector2Scale(player.velocity, player.dampening);
    } else {
        direction = Vector2Normalize(direction);
        player.velocity = Vector2Add(player.velocity, Vector2Scale(direction, player.acceleration * delta));
    }
    player.velocity = Vector2ClampValue(player.velocity, 0.0f, player.maxVelocity);
    player.position = Vector2Add(player.position, Vector2Scale(player.velocity, delta));
}

void UpdatePlayerShip() {
    //UpdatePlayerHitbox
    player.hitbox.x = player.position.x;
    player.hitbox.y = player.position.y;
    player.hitbox.width  = player.size;
    player.hitbox.height = player.size;

    MoveShip();
}

void DrawShipHitbox() {
    DrawRectangleRec(player.hitbox, RED);
}

void DrawDebug() {
    Vector2 playerCenter = CalcCenter(player.position, player.size);
    DrawLineV(playerCenter, Vector2Add(playerCenter, Vector2Scale(player.velocity, 1)), GREEN);

    //DrawText(TextFormat("%f", Vector2Length(player.drones[0].acceleration)), player.position.x - 10, player.position.y - 10, 14, WHITE);
}

void MoveDrone(Drone* drone) {
    float delta = GetFrameTime();

    Vector2 playerCenter = CalcCenter(player.position, player.size);
    Vector2 droneCenter = drone->position;

    float playerDroneDistance = Vector2Distance(playerCenter, droneCenter);
    Vector2 droneToPlayer = Vector2Normalize(Vector2Subtract(playerCenter, droneCenter));
    Vector2 playerToDrone = Vector2Normalize(Vector2Subtract(droneCenter, playerCenter));
    Vector2 normalizedVelocity = Vector2Normalize(drone->velocity);

    if (playerDroneDistance > drone->playerMaxDistance) {
        drone->acceleration = Vector2Add(drone->acceleration, Vector2Scale(droneToPlayer, drone->jerk/100 * playerDroneDistance * delta));
        drone->velocity = Vector2Scale(drone->velocity, Vector2DotProduct(droneToPlayer, normalizedVelocity)*0.2f + 0.8f);
    }
    else if (playerDroneDistance < drone->playerMinDistance) {
        drone->acceleration = Vector2Add(drone->acceleration, Vector2Scale(playerToDrone, drone->jerk * delta));
    }
    else {
        drone->acceleration = Vector2Scale(drone->acceleration, drone->dampening);
        drone->velocity = Vector2Scale(drone->velocity, drone->dampening);
    }
    drone->acceleration = Vector2ClampValue(drone->acceleration, 0.0f, drone->maxAccel);
    drone->velocity = Vector2Add(drone->velocity, Vector2Scale(drone->acceleration, delta));
    drone->velocity = Vector2ClampValue(drone->velocity, 0.0f, drone->maxVelocity);
    drone->position = Vector2Add(drone->position, Vector2Scale(drone->velocity, delta));
}

void DrawDrone(Drone* drone) {
    DrawCircleV(drone->position, drone->size, drone->color);
}

void DrawDroneConnection(Drone* drone) {
    Vector2 playerCenter = CalcCenter(player.position, player.size);
    Vector2 droneCenter = drone->position;
    DrawLineV(playerCenter, droneCenter, YELLOW);
}

Vector2 FindEnemyDirection(Drone* drone, Enemy* enemy) {
    return Vector2Normalize(Vector2Subtract(enemy->position, drone->position));
}

void Shoot(Drone* drone, Vector2 direction) {
    
}

Enemy* FindClosestEnemyInRange(Drone* drone, float range) {
    Enemy* closest = NULL;
    float distanceToClosest = INT_MAX;
    for (size_t i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemyArray[i].alive) continue;

        float distanceToEnemy = Vector2Distance(enemyArray[i].position, drone->position);
        if (distanceToEnemy > range) continue;
        if (distanceToEnemy > distanceToClosest) continue;

        closest = &enemyArray[i];
        distanceToClosest = distanceToEnemy;
    }
    return closest;
}

void DrawDroneFacingDirection(Drone* drone, float arcsize) {
    if (arcsize < 20) arcsize = 20;
    DrawCircleSector(drone->position, drone->size + 3.0f, drone->facingAngle*180/PI - arcsize / 2, drone->facingAngle*180 / PI + arcsize / 2, 1, GREEN);
}

void TurnTowardsDirection(Drone* drone, Vector2 direction) {
    float delta = GetFrameTime();
    float directionAngle = Vector2Angle((Vector2) { 1.0f, 0.0f }, direction);
    float turningFactor = 0.1f; // [0.0f - 1.0f] Higher means faster turning towards enemy.
    drone->facingAngle = Lerp(drone->facingAngle, directionAngle, turningFactor);
}

void UpdateDrawDrone(Drone* drone) {
    MoveDrone(drone);

    DrawDroneFacingDirection(drone, 20);
    DrawDrone(drone);
    DrawDroneConnection(drone);

    drone->framesSinceShotFired++;
    if (drone->framesSinceShotFired > drone->shotCooldownFrames) drone->canShoot = true;

    if (!drone->canShoot) return;
    Enemy* closestEnemy = FindClosestEnemyInRange(drone, drone->detectRange);
    if (closestEnemy == NULL) return;
    //DrawCircleV(closestEnemy->position, 10.0f, drone->color); // Debug Closest Enemy in Range
    DrawLineV(drone->position, closestEnemy->position, WHITE); // Debug Closest Enemy in Range
    
    Vector2 enemyDirection = FindEnemyDirection(drone, closestEnemy);
    TurnTowardsDirection(drone, enemyDirection);
    Shoot(drone, enemyDirection);
    drone->framesSinceShotFired = 0;
}

void DrawEnemy(Enemy *enemy) {
    DrawPolyLines(enemy->position, 6, enemy->size, 0.0f, WHITE);
}

// Update and draw frame
void UpdateDrawFrame(void)
{

    switch (currentGameScreen) {
        case SCREEN_LOGO:
            if (globalFrameCounter > LOGO_DISPLAY_FRAMES) {
                globalFrameCounter = 0;
                currentGameScreen = SCREEN_TITLE;
            }
            DrawLogoScreen();

            globalFrameCounter++;
            break;
        case SCREEN_TITLE:
            // TODO: Make main menu logic

            BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawMenuScreen();
            EndDrawing();
            currentGameScreen = SCREEN_GAMEPLAY;
            break;

        case SCREEN_GAMEPLAY:
            UpdatePlayerShip();
            
            BeginDrawing();
                ClearBackground(BLACK);
                DrawShipHitbox();

                // Drone Logic and Draw
                for (size_t i = 0; i < MAX_DRONES; i++)
                {
                    if (!player.drones[i].alive) continue;

                    UpdateDrawDrone(&player.drones[i]);
                    // Nudge Drones away from each other
                    for (size_t j = 0; j < MAX_DRONES; j++)
                    {
                        if (i == j) continue;
                        float nudgeRadius = (float)player.drones[i].size*2 + 4;
                        float distJtoI = Vector2Distance(player.drones[i].position, player.drones[j].position);
                        if (distJtoI > nudgeRadius) continue;
                        Vector2 dirJtoI = Vector2Normalize(Vector2Subtract(player.drones[i].position, player.drones[j].position));
                        player.drones[i].acceleration = Vector2Add(player.drones[i].acceleration, Vector2Scale(dirJtoI, 30.0f));
                    }
                }

                // Enemy Logic and Draw
                for (size_t i = 0; i < MAX_ENEMIES; i++)
                {   
                    if (!enemyArray[i].alive) continue;
                    DrawEnemy(&enemyArray[i]);
                }
                DrawDebug();
            EndDrawing();

            break;
    }
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables / Implement example logic at this point
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    // Render game screen to a texture, 
    // it could be useful for scaling or further shader postprocessing
    

    // Render to screen (main framebuffer)
    
    //----------------------------------------------------------------------------------  
}