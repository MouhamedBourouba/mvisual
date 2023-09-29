#include <iostream>
#include <raylib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HIGHT 450
#define WINDOW_TITLE "Mvisual"
#define MAX_FPS 60

bool isPlaying = true;

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, WINDOW_TITLE);
  InitAudioDevice();

  auto music = LoadMusicStream("song.mp3");
  PlayMusicStream(music);
  SetTargetFPS(MAX_FPS);

  while (!WindowShouldClose()) {
    UpdateMusicStream(music);

    auto timePlayed = GetMusicTimePlayed(music) / GetMusicTimeLength(music);
    if (timePlayed > 1.0f) {
      timePlayed = 1.0;
    }

    if (IsKeyPressed(KEY_P)) {
      isPlaying = !isPlaying;
      if (isPlaying) {
        PauseMusicStream(music);
      } else {
        ResumeMusicStream(music);
      }
    }

    if (IsKeyPressed(KEY_SPACE)) {
      StopMusicStream(music);
      PauseMusicStream(music);
    }

    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("MUSIC SHOULD BE PLAYING!", 255, 150, 20, LIGHTGRAY);

    DrawRectangle(200, 200, 400, 12, LIGHTGRAY);
    DrawRectangle(200, 200, (int)(timePlayed * 400.0f), 12, MAROON);
    DrawRectangleLines(200, 200, 400, 12, GRAY);

    auto restartText = "PRESS SPACE TO RESTART MUSIC";
    auto pauseText = "PRESS P TO PAUSE/RESUME MUSIC";
    auto visualationText = "PRESS V TO ENTER VISUALATION MODE";
    auto fpsText = "PRESS F TO TOGGEL FPS";

    auto calculateTextPosX = [](const char *text) {
      return (WINDOW_WIDTH - MeasureText(text, 20)) / 2;
    };

    DrawText(restartText, calculateTextPosX(restartText), 250, 20, LIGHTGRAY);
    DrawText(pauseText, calculateTextPosX(pauseText), 280, 20, LIGHTGRAY);
    DrawText(visualationText, calculateTextPosX(visualationText), 310, 20,
             LIGHTGRAY);

    DrawFPS(0, 0);

    EndDrawing();
  }

  CloseAudioDevice();
  CloseWindow();

  return 0;
}
