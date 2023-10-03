#include <complex.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define WINDOW_WIDTH 800
#define WINDOW_HIGHT 450
#define WINDOW_TITLE "mvisual"
#define MAX_FPS 70
#define FONT_SIZE 20

#ifdef _MSC_VER
#define FloatComplex _Fcomplex
#define cfromreal(re) _FCbuild(re, 0)
#define cfromimag(im) _FCbuild(0, im)
#define mulcc _FCmulcc
#define addcc(a, b) _FCbuild(crealf(a) + crealf(b), cimagf(a) + cimagf(b))
#define subcc(a, b) _FCbuild(crealf(a) - crealf(b), cimagf(a) - cimagf(b))
#else
#define FloatComplex float complex
#define cfromreal(re) (re)
#define cfromimag(im) ((im)*I)
#define mulcc(a, b) ((a) * (b))
#define addcc(a, b) ((a) + (b))
#define subcc(a, b) ((a) - (b))
#endif

#define N ((1 << 14) / 2)
#define N_FFT (1 << 14)

typedef enum
{
    MUSIC_UNLOADED,
    ERROR_LOADING_MUSIC,
    SIMPLE,
    VISUALATION,
} MODE;

float inRaw[N_FFT];
float inRawWindowed[N_FFT];
FloatComplex outRaw[N_FFT];

Music musicStream;
bool isFPSShown = false;
MODE currentMod = MUSIC_UNLOADED;

/* Audio Processing */
// -------------------------------------------------------------------------------------------------------------------------

// Ported From https://github.com/tsoding/musializer/blob/master/src/plug.c#L110
void fft(float in[], size_t stride, FloatComplex out[], size_t n)
{

    if (n == 1)
    {
        out[0] = cfromreal(in[0]);
        return;
    }

    fft(in, stride * 2, out, n / 2);
    fft(in + stride, stride * 2, out + n / 2, n / 2);

    for (size_t k = 0; k < n / 2; ++k)
    {
        float t = (float)k / n;
        FloatComplex v = mulcc(cexpf(cfromimag(-2 * PI * t)), out[k + n / 2]);
        FloatComplex e = out[k];
        out[k] = addcc(e, v);
        out[k + n / 2] = subcc(e, v);
    }
}

#include <string.h>

void fftPush(float f)
{
    memmove(inRaw, inRaw + 1, (N - 1) * sizeof(inRaw[0]));
    inRaw[N - 1] = f;
}

void audioCallback(void* bufferData, unsigned int frames)
{

    float(*fs)[2] = bufferData;
    for (size_t i = 0; i < frames; ++i)
    {
        fftPush(fs[i][0]);
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
        if (currentMod == MUSIC_UNLOADED || currentMod == ERROR_LOADING_MUSIC) currentMod = SIMPLE;
    }
    else
    {
        currentMod = ERROR_LOADING_MUSIC;
    }
}

static inline float amp(FloatComplex z)
{   
    float a = cfromreal(z);
    float b = cfromimag(z);
    return logf(a*a + b*b);
}

// -------------------------------------------------------------------------------------------------------------------------

/*  UI  */
// -------------------------------------------------------------------------------------------------------------------------

void drawHorizentalyCenteredText(const char* text, int y)
{
    DrawText(text, ((WINDOW_WIDTH - MeasureText(text, FONT_SIZE)) / 2), y, FONT_SIZE, LIGHTGRAY);
}

void drawTitle(const char* text)
{
    ClearBackground(RAYWHITE);
    drawHorizentalyCenteredText(text, 20);
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

    drawHorizentalyCenteredText("PRESS SPACE TO RESTART MUSIC", 200);
    drawHorizentalyCenteredText("PRESS P TO PAUSE/RESUME MUSIC", 230);
    drawHorizentalyCenteredText("PRESS V TO ENTER VISUALATION MODE", 260);
    drawHorizentalyCenteredText("PRESS F TO TOGGEL FPS", 290);
}
void drawVisualationMode()
{
    ClearBackground(RAYWHITE);
    drawHorizentalyCenteredText("WELCOME TO VISUALATION MODE STILL IN DEVELEPMENT", 20);

    for (int i = 0; i < N_FFT; ++i)
    {
        float t = (float)i / N_FFT;
        float hann = 0.5 - 2*cosf(2*PI*t);
        inRawWindowed[i] = inRaw[i] * hann;
    }

    fft(inRawWindowed, 1, outRaw, N_FFT);

    float ampMax = 0.00f;
    for (size_t i = 0; i < N; ++i)
    {
        float a = amp(outRaw[i]);
        if (ampMax < a) ampMax = a;
    }

    uint m = 0;
    float fn = logf((N / 17.0f)) / logf(1.06f) + 1.0f;
    float w = (float)WINDOW_WIDTH / fn;

    for (float f = 17.0f; f < N; f = f*1.06f)
    {
        float f1 = f*1.06f;
        float ft = 0;

        for (size_t q = (size_t)f; q < N && q < (size_t)f1; q++)
        {
            float _f = amp(outRaw[q]);
            if (ft < _f) ft = _f;
        }

        float t = (ft) / ampMax;
        float h = ((float)(WINDOW_HIGHT * t)) / 1.5;
        int x = (int)(w * m);
        DrawRectangle(x, WINDOW_HIGHT - (int)h, (int)w, (int)h, BLUE);

        m++;
    }
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

    if (IsKeyPressed(KEY_P) && currentMod != MUSIC_UNLOADED && currentMod != ERROR_LOADING_MUSIC)
    {
        (IsMusicStreamPlaying(musicStream)) ? PauseMusicStream(musicStream) : ResumeMusicStream(musicStream);
    }

    if (IsKeyPressed(KEY_SPACE) && currentMod != MUSIC_UNLOADED && currentMod != ERROR_LOADING_MUSIC)
    {
        StopMusicStream(musicStream);
        PlayMusicStream(musicStream);
    }

    if (IsKeyPressed(KEY_V) && currentMod != MUSIC_UNLOADED && currentMod != ERROR_LOADING_MUSIC)
    {
        if (currentMod == VISUALATION)
            currentMod = SIMPLE;
        else if (currentMod == SIMPLE)
            currentMod = VISUALATION;
    }
}
// -------------------------------------------------------------------------------------------------------------------------

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, WINDOW_TITLE);
    InitAudioDevice();
    SetTargetFPS(MAX_FPS);

    // FOR DEVELEPMENT ------------------------------------------------
    loadAndPlayMusic("song.mp3");
    AttachAudioStreamProcessor(musicStream.stream, audioCallback);
    // SetMusicVolume(musicStream, 0.0f);
    // ----------------------------------------------------------------

    while (!WindowShouldClose())
    {
        UpdateMusicStream(musicStream);

        handleInput();

        BeginDrawing();

        if (isFPSShown) DrawFPS(0, 0);
        if (currentMod == SIMPLE) drawSimpleMode();
        if (currentMod == VISUALATION) drawVisualationMode();
        if (currentMod == MUSIC_UNLOADED) drawTitle("DRAG & DROP TO LOAD MUSIC");
        if (currentMod == ERROR_LOADING_MUSIC) drawTitle("ERROR Loading Music");

        EndDrawing();
    }

    UnloadMusicStream(musicStream);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
