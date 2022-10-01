
// todo get rid of this
#include <vector>

#include "raylib.h"

#include "Maze.h"

struct RowCol
{
    size_t row;
    size_t col;
};

struct MazeWall
{
    Vector2 position;
    Texture2D* texture;
};

// init
const size_t SCREEN_WIDTH = 720;
const size_t SCREEN_HEIGHT = 480;

// player 
const size_t PLAYER_START_COL = 6;
const size_t PLAYER_START_ROW = 4;
const size_t PLAYER_START_LIVES = 5;

// goal
const size_t GOAL_START_COL = MAZE_COLS - 6;
const size_t GOAL_START_ROW = MAZE_ROWS - 4;

// game
const float MAZE_SECONDS = 10;

Vector2 getPosition(RowCol rowCol)
{
    Vector2 position;
    position.x = (float)(rowCol.col * MAZE_TILE_SIZE);
    position.y = (float)(rowCol.row * MAZE_TILE_SIZE);
    return position;
}

RowCol getRandomRowCol()
{
    return { (size_t)GetRandomValue(0, MAZE_ROWS - 1), (size_t)GetRandomValue(0, MAZE_COLS - 1)};
}

bool checkIfBlocked(RowCol target, const std::vector<RowCol>& blockers)
{
    for (size_t i = 0; i < blockers.size(); i++)
    {
        if (blockers[i].row == target.row &&
            blockers[i].col == target.col)
        {
           return true;
        }
    }
    return false;
}

void resetPlayer(Vector2& playerPosition, std::vector<RowCol>& blockers)
{
    RowCol targetRowCol{};
    do
    {
       targetRowCol = getRandomRowCol(); 
    } while (checkIfBlocked(targetRowCol, blockers));
    playerPosition = getPosition(targetRowCol);
}

void resetGoal(RowCol& goalRowCol, Vector2& goalPosition, std::vector<RowCol>& blockers)
{
    // todo make sure to now overlap player
    RowCol targetRowCol{};
    do
    {
       targetRowCol = getRandomRowCol(); 
    } while (checkIfBlocked(targetRowCol, blockers));
    goalRowCol = targetRowCol;
    goalPosition = getPosition(targetRowCol);
    blockers.push_back(goalRowCol);
}

void resetGame(float& mazeTimer)
{
    mazeTimer = MAZE_SECONDS;
}


void resetMaze(size_t& currentMazeIndex, std::vector<MazeWall>& mazeWalls, std::vector<RowCol>& blockers, Texture2D* mazeWallTextures)
{
    mazeWalls.clear();
    blockers.clear();

    size_t mazeIndex;
    do
    {
        mazeIndex = (size_t)GetRandomValue(0, MAZE_LAYOUTS_SIZE - 1);
    } while (mazeIndex == currentMazeIndex);
    currentMazeIndex = mazeIndex;
    
    const char** currentMaze = MAZE_LAYOUTS[mazeIndex];
    for (size_t row = 0; row < MAZE_ROWS; row++)
    {
        const char* mazeRow = currentMaze[row];
        for (size_t col = 0; col < MAZE_COLS; col++)
        {
           char type = mazeRow[col];
            if (type == MAZE_EMPTY)
            {
                
            }
            else if (type == MAZE_WALL)
            {
                MazeWall newWall;
                newWall.position = getPosition({row, col});
                newWall.texture = &mazeWallTextures[(size_t)GetRandomValue(0, MAZE_WALL_TEXTURES - 1)];
                mazeWalls.push_back(newWall);
                blockers.push_back({row, col});
            }
        }
    }
}

int main()
{

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Every 10 Seconds");
    SetTargetFPS(60);

    InitAudioDevice();
    
    // maze
    std::vector<MazeWall> mazeWalls;
    Texture2D mazeWallTextures[MAZE_WALL_TEXTURES];
    mazeWallTextures[0] = LoadTexture("resources/mazeWall0.png");
    mazeWallTextures[1] = LoadTexture("resources/mazeWall1.png");
    std::vector<RowCol> blockers;
    size_t currentMazeIndex = (size_t)GetRandomValue(0, MAZE_LAYOUTS_SIZE - 1);
    resetMaze(currentMazeIndex, mazeWalls, blockers, mazeWallTextures);
    
    // goal
    // todo replace with resetGoal
    RowCol goalRowCol;
    Vector2 goalPosition;
    resetGoal(goalRowCol, goalPosition, blockers);
    Texture2D goalTexture = LoadTexture("resources/goal.png");
    Sound goalSound = LoadSound("resources/goal.wav");
    

    // player
    bool playerMoving = false;
    float playerTilesPerSecond = 8;
    float playerMoveTimer = 0;
    size_t playerLives = PLAYER_START_LIVES;
    Vector2 playerPosition;
    float playerDirection = 1.f;
    resetPlayer(playerPosition, blockers);
    
    Texture2D playerTexture = LoadTexture("resources/player.png");
    Sound playerLifeLostSound = LoadSound("resources/playerLiveLost.wav"); 
    Sound playerFootStepsSound = LoadSound("resources/playerFootSteps.wav"); 
    Sound playerObstacleSound = LoadSound("resources/playerObstacle.wav"); 
    SetSoundVolume(playerObstacleSound, 0.3f);
    SetSoundVolume(playerFootStepsSound, 0.5f);
    
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
            movement.x = MAZE_TILE_SIZE;
            playerDirection = 1.f;
        }
        if (IsKeyDown(KEY_LEFT))
        {
            movement.x -= MAZE_TILE_SIZE;
            playerDirection = -1.f;
        }
        if (IsKeyDown(KEY_UP))
        {
            movement.y -= MAZE_TILE_SIZE;
        }
        if (IsKeyDown(KEY_DOWN))
        {
            movement.y = MAZE_TILE_SIZE;
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

                Vector2 lastPosition = playerPosition;
                
                // move player
                playerPosition.x += movement.x;
                playerPosition.y += movement.y;

                int targetRow = playerPosition.y / MAZE_TILE_SIZE;
                int targetCol = playerPosition.x / MAZE_TILE_SIZE; 
                
                // wrap around 
                bool wrappedArouind = false;
                if (targetCol < 0)
                {
                    targetCol = MAZE_COLS - 1;
                    playerPosition.x = SCREEN_WIDTH + movement.x;
                    wrappedArouind = true;
                }
                else if (targetCol > MAZE_COLS - 1)
                {
                    targetCol = 0;
                    playerPosition.x = 0;
                    wrappedArouind = true;
                }
                if (targetRow < 0)
                {
                    targetRow = MAZE_ROWS - 1;
                    playerPosition.y = SCREEN_HEIGHT + movement.y;
                    wrappedArouind = true;
                }
                else if (targetRow > MAZE_ROWS - 1)
                {
                    targetRow = 0;
                    playerPosition.y = 0;
                    wrappedArouind = true;
                }

                // check if blocker
                bool playFootsteps = true;
                if (checkIfBlocked({(size_t)targetRow, (size_t)targetCol}, blockers))
                {
                    // revert position
                    playerPosition.x = lastPosition.x;
                    playerPosition.y = lastPosition.y;
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
                if (targetCol == goalRowCol.col &&
                    targetRow == goalRowCol.row)
                {
                    // win
                    playerLives++;
                    
                    // todo increase obstacles/difficulty
                    resetMaze(currentMazeIndex, mazeWalls, blockers, mazeWallTextures);
                    resetGoal(goalRowCol, goalPosition, blockers);
                    resetPlayer(playerPosition, blockers);
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
            resetMaze(currentMazeIndex, mazeWalls, blockers, mazeWallTextures);
            resetGoal(goalRowCol, goalPosition, blockers);
            resetPlayer(playerPosition, blockers);
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
            for (int col = 0; col < MAZE_COLS; col++)
            {
                DrawLine(col * MAZE_TILE_SIZE, 0, col * MAZE_TILE_SIZE, SCREEN_HEIGHT, GRAY);
            }
            for (int row = 0; row < MAZE_ROWS; row++)
            {
                DrawLine(0, row * MAZE_TILE_SIZE, SCREEN_WIDTH, row * MAZE_TILE_SIZE, GRAY);
            }

            // draw maze
            for (int i = 0; i < mazeWalls.size(); i++)
            {
                DrawTextureV(*mazeWalls[i].texture, mazeWalls[i].position, WHITE);
            }
        

            // draw goal
            // DrawCircleV(goalPosition, MAZE_TILE_SIZE / 2.f, GREEN);
            DrawTextureV(goalTexture, goalPosition, WHITE);
        
            // draw player
            // DrawCircleV(playerPosition, MAZE_TILE_SIZE / 2.f, MAROON);
            DrawTextureRec(playerTexture, {0, 0, playerDirection * MAZE_TILE_SIZE, MAZE_TILE_SIZE}, playerPosition, WHITE);
        
            // draw game
            DrawText(TextFormat("Lives: %d", playerLives), (1 * MAZE_TILE_SIZE) + (MAZE_TILE_SIZE / 4), 3, 15, LIGHTGRAY);
            DrawText("Reach the end of the maze!", 11 * MAZE_TILE_SIZE + (MAZE_TILE_SIZE / 4), 3, 15, LIGHTGRAY);
            DrawText(TextFormat("Time left: %.2f", mazeTimer), SCREEN_WIDTH - (5 * MAZE_TILE_SIZE) + (MAZE_TILE_SIZE / 4), 3, 15, LIGHTGRAY);
        
        EndDrawing();
    }
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(mazeWallTextures[0]);
    UnloadTexture(mazeWallTextures[1]);
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
