#include <complex>
#include <iostream>
#include <raylib.h>
#include <valarray>
#include <vector>

#define WINDOW_WIDTH 800
#define WINDOW_HIGHT 450
#define WINDOW_TITLE "mvisual"
#define MAX_FPS 70
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
void loadAndPlayMusic(const char* path);
void audioCallback(void* bufferData, unsigned int frames);

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, WINDOW_TITLE);
    InitAudioDevice();
    SetTargetFPS(MAX_FPS);

    // FOR DEVELEPMENT ------------------------------------------------
    loadAndPlayMusic("song.mp3");
    AttachAudioStreamProcessor(musicStream.stream, audioCallback);
    // ----------------------------------------------------------------

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

/*  UI  */
// -------------------------------------------------------------------------------------------------------------------------
void drawWelcomeText()
{
    ClearBackground(RAYWHITE);
    auto text = "WELCOME TO MVISAL PLEASE DRAG AND DROP YOUR MUSIC";
    DrawText(text, calculateTextCenterPosX(text), 20, FONT_SIZE, LIGHTGRAY);
}
void drawSimpleMode()
{
    float timePlayed = GetMusicTimePlayed(musicStream) / GetMusicTimeLength(musicStream);
    if (timePlayed > 1.0f) timePlayed = 1.0f;

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
// -------------------------------------------------------------------------------------------------------------------------

/*  USER INPUT */
// -------------------------------------------------------------------------------------------------------------------------
void handleInput()
{
    if (IsFileDropped())
    {
        FilePathList files = LoadDroppedFiles();
        loadAndPlayMusic(files.paths[0]);
        UnloadDroppedFiles(files);
    }

    if (IsKeyPressed(KEY_F))
    {
        isFPSShown = !isFPSShown;
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
// -------------------------------------------------------------------------------------------------------------------------

/*  UTILS */
// -------------------------------------------------------------------------------------------------------------------------
void loadAndPlayMusic(const char* path)
{
    musicStream = LoadMusicStream(path);
    PlayMusicStream(musicStream);

    if (IsMusicReady(musicStream))
    {
        AttachAudioStreamProcessor(musicStream.stream, audioCallback);
        if (currentMod == MODE::MUSIC_UNLOADED) currentMod = MODE::SIMPLE;
    }
    else
    {
        currentMod = MODE::MUSIC_UNLOADED;
    }
}
// -------------------------------------------------------------------------------------------------------------------------

/* Audio Processing */
// -------------------------------------------------------------------------------------------------------------------------

typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;
const double pi = 3.141592653589793238460;

// Cooley–Tukey FFT (in-place, divide-and-conquer)
// Higher memory requirements and redundancy although more intuitive
// Ported From https://www.rosettacode.org/wiki/Fast_Fourier_transform#C++
void fft(CArray& x)
{
    const size_t N = x.size();
    if (N <= 1) return;

    // divide
    CArray even = x[std::slice(0, N / 2, 2)];
    CArray odd = x[std::slice(1, N / 2, 2)];

    // conquer
    fft(even);
    fft(odd);

    // combine
    for (size_t k = 0; k < N / 2; ++k)
    {
        Complex t = std::polar(1.0, -2 * pi * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N / 2] = even[k] - t;
    }
}

void audioCallback(void* bufferData, unsigned int frames)
{
    float(*samplesAs2DArray)[2] = reinterpret_cast<float(*)[2]>(bufferData);

    for (int i = 0; i < frames; ++i)
    {
    }
}

// -------------------------------------------------------------------------------------------------------------------------
