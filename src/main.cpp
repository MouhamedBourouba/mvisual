#include <cstddef>
#include <cstdio>
#include <iostream>
#include <raylib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HIGHT 450
#define WINDOW_TITLE "mvisual"
#define MAX_FPS 60
#define FONT_SIZE 20

enum class MODE
{
    MUSIC_UNLOADED,
    SIMPLE,
    VISUALATION,
};

auto calculateTextCenterPosX = [](const char* text) { return (WINDOW_WIDTH - MeasureText(text, FONT_SIZE)) / 2; };

Music musicStream;
bool isFPSShown = false;
auto currentMod = MODE::MUSIC_UNLOADED;

void drawSimpleMode();
void drawVisualationMode();
void handleInput();
void drawWelcomeText();

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, WINDOW_TITLE);
    InitAudioDevice();
    SetTargetFPS(MAX_FPS);

    while (!WindowShouldClose())
    {
        UpdateMusicStream(musicStream);

        handleInput();

        BeginDrawing();

        if (isFPSShown) DrawFPS(0, 0);
        if (currentMod == MODE::SIMPLE) drawSimpleMode();
        if (currentMod == MODE::VISUALATION) drawVisualationMode();
        if (currentMod == MODE::MUSIC_UNLOADED) drawWelcomeText();

        EndDrawing();
    }

    UnloadMusicStream(musicStream);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

void drawWelcomeText()
{
    ClearBackground(RAYWHITE);
    auto text = "WELCOME TO MVISAL PLEASE DRAG AND DROP YOUR MUSIC";
    DrawText(text, calculateTextCenterPosX(text), 20, FONT_SIZE, LIGHTGRAY);
}
void drawSimpleMode()
{

    float timePlayed = GetMusicTimePlayed(musicStream) / GetMusicTimeLength(musicStream);
    if (timePlayed > 1.0f) timePlayed = 1.0;

    ClearBackground(RAYWHITE);

    DrawText("MUSIC SHOULD BE PLAYING!", 255, 100, FONT_SIZE, LIGHTGRAY);

    DrawRectangle(200, 150, 400, 12, LIGHTGRAY);
    DrawRectangle(200, 150, (int)(timePlayed * 400.0f), 12, MAROON);
    DrawRectangleLines(200, 150, 400, 12, GRAY);

    auto restartText = "PRESS SPACE TO RESTART MUSIC";
    auto pauseText = "PRESS P TO PAUSE/RESUME MUSIC";
    auto visualationText = "PRESS V TO ENTER VISUALATION MODE";
    auto fpsText = "PRESS F TO TOGGEL FPS";

    DrawText(restartText, calculateTextCenterPosX(restartText), 200, FONT_SIZE, LIGHTGRAY);
    DrawText(pauseText, calculateTextCenterPosX(pauseText), 230, FONT_SIZE, LIGHTGRAY);
    DrawText(visualationText, calculateTextCenterPosX(visualationText), 260, FONT_SIZE, LIGHTGRAY);
    DrawText(fpsText, calculateTextCenterPosX(fpsText), 290, FONT_SIZE, LIGHTGRAY);
}
void drawVisualationMode()
{
    ClearBackground(RAYWHITE);
    auto text = "WELCOME TO VISUALATION MODE STILL IN DEVELEPMENT";
    DrawText(text, calculateTextCenterPosX(text), 20, FONT_SIZE, LIGHTGRAY);
}
void handleInput()
{

    if (IsFileDropped())
    {
        UnloadMusicStream(musicStream);

        FilePathList files = LoadDroppedFiles();

        musicStream = LoadMusicStream(files.paths[0]);
        PlayMusicStream(musicStream);

        if (IsMusicStreamPlaying(musicStream) && currentMod == MODE::MUSIC_UNLOADED) currentMod = MODE::SIMPLE;

        UnloadDroppedFiles(files);
    }

    if (IsKeyPressed(KEY_P) && currentMod != MODE::MUSIC_UNLOADED)
    {
        (IsMusicStreamPlaying(musicStream)) ? PauseMusicStream(musicStream) : ResumeMusicStream(musicStream);
    }

    if (IsKeyPressed(KEY_SPACE) && currentMod != MODE::MUSIC_UNLOADED)
    {
        StopMusicStream(musicStream);
        PlayMusicStream(musicStream);
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
