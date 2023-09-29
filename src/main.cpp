#include <cstddef>
#include <iostream>
#include <raylib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HIGHT 450
#define WINDOW_TITLE "mvisual"
#define MAX_FPS 60

bool isPlaying = true;
bool isFPSShown = false;

enum class MODE
{
    SIMPLE_MODE,
    VISUALATION_MODE,
};

// Ported from https://rosettacode.org/wiki/Fast_Fourier_transform#Python

int main(void)
{
    auto currentMod = MODE::SIMPLE_MODE;
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, WINDOW_TITLE);
    InitAudioDevice();

    auto music = LoadMusicStream("song.mp3");
    PlayMusicStream(music);
    SetTargetFPS(MAX_FPS);

    while (!WindowShouldClose())
    {
        UpdateMusicStream(music);

        auto timePlayed = GetMusicTimePlayed(music) / GetMusicTimeLength(music);
        if (timePlayed > 1.0f) timePlayed = 1.0;

        if (IsKeyPressed(KEY_P))
        {
            (isPlaying) ? PauseMusicStream(music) : ResumeMusicStream(music);
            isPlaying = !isPlaying;
        }
        if (IsKeyPressed(KEY_SPACE))
        {
            StopMusicStream(music);
            PauseMusicStream(music);
        }
        if (IsKeyPressed(KEY_F))
        {
            isFPSShown = !isFPSShown;
        }
        if (IsKeyPressed(KEY_V))
        {
            switch (currentMod)
            {
            case MODE::SIMPLE_MODE:
                currentMod = MODE::VISUALATION_MODE;
                break;
            case MODE::VISUALATION_MODE:
                currentMod = MODE::SIMPLE_MODE;
                break;
            }
        }

        auto calculateTextCenterPosX = [](const char* text) { return (WINDOW_WIDTH - MeasureText(text, 20)) / 2; };

        BeginDrawing();

        if(isFPSShown) {
            DrawFPS(0, 0);
        }

        if (currentMod == MODE::SIMPLE_MODE)
        {
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

            EndDrawing();
            continue;
        }

        if (currentMod == MODE::VISUALATION_MODE)
        {
            ClearBackground(RAYWHITE);
            auto text = "WELCOME TO VISUALATION MODE STILL IN DEVELEPMENT";
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
