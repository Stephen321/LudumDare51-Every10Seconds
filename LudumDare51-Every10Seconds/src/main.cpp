
#include "raylib.h"

// init
const size_t TILE_SIZE = 720 / 30; // 24
const size_t SCREEN_WIDTH = 720;
const size_t SCREEN_HEIGHT = 480;
const size_t ROWS = SCREEN_HEIGHT / TILE_SIZE; // 30
const size_t COLS = SCREEN_WIDTH / TILE_SIZE; // 20
const bool USE_SHAPES = false;

// player 
const size_t PLAYER_START_COL = 6;
const size_t PLAYER_START_ROW = 4;
const size_t PLAYER_START_LIVES = 5;

// goal
const size_t GOAL_START_COL = COLS - 6;
const size_t GOAL_START_ROW = ROWS - 4;

// game
const float MAZE_SECONDS = 10;

Vector2 getPosition(size_t row, size_t col)
{
    Vector2 position;
    position.x = (float)(col * TILE_SIZE);
    position.y = (float)(row * TILE_SIZE);
    return position;
}

void resetPlayer(Vector2& playerPosition)
{
    size_t row = GetRandomValue(0, ROWS - 1);
    size_t col = GetRandomValue(0, COLS - 1);
    playerPosition = getPosition(row, col);
}

void resetGoal(size_t& goalRow, size_t& goalCol, Vector2& goalPosition)
{
    // todo make sure to now overlap player
    goalRow = GetRandomValue(0, ROWS - 1);
    goalCol = GetRandomValue(0, COLS - 1);
    goalPosition = getPosition(goalRow, goalCol);
}

void resetGame(float& mazeTimer)
{
    mazeTimer = MAZE_SECONDS;
}

int main()
{

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Every 10 Seconds");
    SetTargetFPS(60);

    InitAudioDevice();

    // player
    bool playerMoving = false;
    float playerTilesPerSecond = 8;
    float playerMoveTimer = 0;
    size_t playerLives = PLAYER_START_LIVES;
    // todo replace with resetPlayer
    Vector2 playerPosition = getPosition(PLAYER_START_COL, PLAYER_START_ROW);
    Texture2D playerTexture = LoadTexture("resources/player.png");
    Sound playerLifeLostSound = LoadSound("resources/playerLiveLost.wav"); 
    Sound playerFootStepsSound = LoadSound("resources/playerFootSteps.wav"); 
    Sound playerObstacleSound = LoadSound("resources/playerObstacle.wav"); 
    SetSoundVolume(playerObstacleSound, 0.3f);
    SetSoundVolume(playerFootStepsSound, 0.5f);
    
    // goal
    // todo replace with resetGoal
    size_t goalCol = GOAL_START_COL;
    size_t goalRow = GOAL_START_ROW;
    Vector2 goalPosition = getPosition(goalRow, goalCol);
    Texture2D goalTexture = LoadTexture("resources/goal.png");
    Sound goalSound = LoadSound("resources/goal.wav"); 
    
    // game
    float mazeTimer = MAZE_SECONDS;
    Music music = LoadMusicStream("resources/music.mp3");
    PlayMusicStream(music);
    SetMusicVolume(music, 0.4f);
    
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

        // music 
        UpdateMusicStream(music);
        
        // input
        Vector2 movement{0,0};
        playerMoving = false;
        if (IsKeyDown(KEY_RIGHT))
        {
            movement.x = TILE_SIZE;
        }
        if (IsKeyDown(KEY_LEFT))
        {
            movement.x -= TILE_SIZE;
        }
        if (IsKeyDown(KEY_UP))
        {
            movement.y -= TILE_SIZE;
        }
        if (IsKeyDown(KEY_DOWN))
        {
            movement.y = TILE_SIZE;
        }
        if (movement.x != 0 || movement.y != 0)
        {
            playerMoving = true;
        }

        // timers
        float deltaTime = GetFrameTime();
        playerMoveTimer += deltaTime;
        mazeTimer -= deltaTime;

        // player logic
        if (playerMoveTimer > 1.f / playerTilesPerSecond)
        {
            if (playerMoving)
            {
                playerMoveTimer = 0;

                // move player
                playerPosition.x += movement.x;
                playerPosition.y += movement.y;

                int playerCol = playerPosition.x / TILE_SIZE; 
                int playerRow = playerPosition.y / TILE_SIZE; 


                // check if in screen bounds
                bool playFootsteps = true;
                if (playerCol < 0 || playerCol > COLS - 1)
                {
                    playerPosition.x -= movement.x;
                    playFootsteps = false;
                }
                if (playerRow < 0 || playerRow > ROWS - 1)
                {
                    playerPosition.y -= movement.y;
                    playFootsteps = false;
                }

                if (playFootsteps)
                {
                    PlaySound(playerFootStepsSound);
                }
                else
                {
                    PlaySound(playerObstacleSound);
                }

                // check if reached goal
                if (playerCol == goalCol &&
                    playerRow == goalRow)
                {
                    // win
                    playerLives++;
                    
                    // todo increase obstacles/difficulty
                    resetPlayer(playerPosition);
                    resetGoal(goalRow, goalCol, goalPosition);
                    resetGame(mazeTimer);
                    PlaySound(goalSound);
                }
            }
        }

        // game logic
        if (mazeTimer < 0.f)
        {
            // lose live, next maze
            playerLives--;
            resetPlayer(playerPosition);
            resetGoal(goalRow, goalCol, goalPosition);
            resetGame(mazeTimer);
            
            // lost game
            if (playerLives <= 0)
            {
               playerLives = PLAYER_START_LIVES;

                // todo: lose 
            }
            else
            {
                PlaySound(playerLifeLostSound);
            }
        }


        // draw
        BeginDrawing();

            ClearBackground(DARKGRAY);

            // draw grid
            for (int col = 0; col < COLS; col++)
            {
                DrawLine(col * TILE_SIZE, 0, col * TILE_SIZE, SCREEN_HEIGHT, GRAY);
            }
            for (int row = 0; row < ROWS; row++)
            {
                DrawLine(0, row * TILE_SIZE, SCREEN_WIDTH, row * TILE_SIZE, GRAY);
            }

            // draw goal
            if (USE_SHAPES)
            {
                DrawCircleV(goalPosition, TILE_SIZE / 2.f, GREEN);
            }
            else
            {
                DrawTextureV(goalTexture, goalPosition, WHITE);
            }
        
            // draw player
            if (USE_SHAPES)
            {
                DrawCircleV(playerPosition, TILE_SIZE / 2.f, MAROON);
            }
            else
            {
                DrawTextureV(playerTexture, playerPosition, WHITE);
            }
            // draw game
            DrawText(TextFormat("Lives: %d", playerLives), (1 * TILE_SIZE) + (TILE_SIZE / 4), 3, 15, LIGHTGRAY);
            DrawText("Reach the end of the maze!", 11 * TILE_SIZE + (TILE_SIZE / 4), 3, 15, LIGHTGRAY);
            DrawText(TextFormat("Time left: %.2f", mazeTimer), SCREEN_WIDTH - (5 * TILE_SIZE) + (TILE_SIZE / 4), 3, 15, LIGHTGRAY);
        
        EndDrawing();
    }
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(playerTexture);
    UnloadTexture(goalTexture);

    UnloadMusicStream(music);   // Unload music stream buffers from RAM
    UnloadSound(playerLifeLostSound);
    UnloadSound(playerFootStepsSound);
    UnloadSound(playerObstacleSound);
    UnloadSound(goalSound);

    CloseAudioDevice();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
