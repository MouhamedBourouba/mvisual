#include <cstddef>
#include <cstdio>
#include <iostream>
#include <raylib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HIGHT 450
#define WINDOW_TITLE "mvisual"
#define MAX_FPS 60

enum class MODE
{
    MUSIC_UNLOADED,
    SIMPLE,
    VISUALATION,
};

auto calculateTextCenterPosX = [](const char* text) { return (WINDOW_WIDTH - MeasureText(text, 20)) / 2; };

Music music;
bool isFPSShown = false;
auto currentMod = MODE::MUSIC_UNLOADED;

void drawSimpleMode();
void drawVisualationMode();
void handleInput();

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, WINDOW_TITLE);
    InitAudioDevice();
    SetTargetFPS(MAX_FPS);

    while (!WindowShouldClose())
    {
        UpdateMusicStream(music);

        handleInput();

        BeginDrawing();

        if (isFPSShown)
        {
            DrawFPS(0, 0);
        }

        if (currentMod == MODE::SIMPLE)
        {
            drawSimpleMode();
            EndDrawing();
            continue;
        }
        if (currentMod == MODE::VISUALATION)
        {
            drawVisualationMode();
            EndDrawing();
            continue;
        }
        if (currentMod == MODE::MUSIC_UNLOADED)
        {
            ClearBackground(RAYWHITE);
            auto text = "WELCOME TO MVISAL PLEASE DRAG AND DROP YOUR MUSIC";
            DrawText(text, calculateTextCenterPosX(text), 20, 20, LIGHTGRAY);
            EndDrawing();
            continue;
        }
    }

    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

void drawSimpleMode()
{

    float timePlayed = GetMusicTimePlayed(music) / GetMusicTimeLength(music);
    if (timePlayed > 1.0f) timePlayed = 1.0;

    ClearBackground(RAYWHITE);

    DrawText("MUSIC SHOULD BE PLAYING!", 255, 100, 20, LIGHTGRAY);

    DrawRectangle(200, 150, 400, 12, LIGHTGRAY);
    DrawRectangle(200, 150, (int)(timePlayed * 400.0f), 12, MAROON);
    DrawRectangleLines(200, 150, 400, 12, GRAY);

    auto restartText = "PRESS SPACE TO RESTART MUSIC";
    auto pauseText = "PRESS P TO PAUSE/RESUME MUSIC";
    auto visualationText = "PRESS V TO ENTER VISUALATION MODE";
    auto fpsText = "PRESS F TO TOGGEL FPS";

    DrawText(restartText, calculateTextCenterPosX(restartText), 200, 20, LIGHTGRAY);
    DrawText(pauseText, calculateTextCenterPosX(pauseText), 230, 20, LIGHTGRAY);
    DrawText(visualationText, calculateTextCenterPosX(visualationText), 260, 20, LIGHTGRAY);
    DrawText(fpsText, calculateTextCenterPosX(fpsText), 290, 20, LIGHTGRAY);
}

void drawVisualationMode()
{
    ClearBackground(RAYWHITE);
    auto text = "WELCOME TO VISUALATION MODE STILL IN DEVELEPMENT";
    DrawText(text, calculateTextCenterPosX(text), 20, 20, LIGHTGRAY);
}

void handleInput()
{

    if (IsFileDropped())
    {
        UnloadMusicStream(music);

        FilePathList files = LoadDroppedFiles();

        music = LoadMusicStream(files.paths[0]);
        PlayMusicStream(music);

        if (IsMusicStreamPlaying(music) && currentMod == MODE::MUSIC_UNLOADED) currentMod = MODE::SIMPLE;

        UnloadDroppedFiles(files);
    }

    if (IsKeyPressed(KEY_P) && currentMod != MODE::MUSIC_UNLOADED)
    {
        (IsMusicStreamPlaying(music)) ? PauseMusicStream(music) : ResumeMusicStream(music);
    }
    
    if (IsKeyPressed(KEY_SPACE) && currentMod != MODE::MUSIC_UNLOADED)
    {
        StopMusicStream(music);
        PlayMusicStream(music);
    }
    
    if (IsKeyPressed(KEY_F))
    {
        isFPSShown = !isFPSShown;
    }

    if (IsKeyPressed(KEY_V) && currentMod != MODE::MUSIC_UNLOADED)
    {
        switch (currentMod)
        {
        case MODE::SIMPLE:
            currentMod = MODE::VISUALATION;
            break;
        case MODE::VISUALATION:
            currentMod = MODE::SIMPLE;
            break;
        case MODE::MUSIC_UNLOADED:
            break;
        }
    }
}
