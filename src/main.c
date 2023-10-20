#include <complex.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define WINDOW_WIDTH 60 * 16
#define WINDOW_HIGHT 60 * 9
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

#define N_FFT (1 << 14)
#define N (size_t)((float)N_FFT / 2)

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
float outLog[N_FFT];
float outSmooth[N_FFT];

Music musicStream;
bool isFPSShown = false;
MODE currentMod = MUSIC_UNLOADED;

/* Audio Processing */
// -------------------------------------------------------------------------------------------------------------------------

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

    if (IsMusicReady(musicStream))
    {
        AttachAudioStreamProcessor(musicStream.stream, audioCallback);
        PlayMusicStream(musicStream);
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
    return logf(a * a + b * b);
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

    drawHorizentalyCenteredText("MUSIC SHOULD BE PLAYING!", 100);

    DrawRectangle((WINDOW_WIDTH - 400) / 2, 150, 400, 12, LIGHTGRAY);
    DrawRectangle((WINDOW_WIDTH - 400) / 2, 150, (int)(timePlayed * 400.0f), 12, MAROON);
    DrawRectangleLines((WINDOW_WIDTH - 400) / 2, 150, 400, 12, GRAY);

    drawHorizentalyCenteredText("PRESS SPACE TO RESTART MUSIC", 200);
    drawHorizentalyCenteredText("PRESS P TO PAUSE/RESUME MUSIC", 230);
    drawHorizentalyCenteredText("PRESS V TO ENTER VISUALATION MODE", 260);
    drawHorizentalyCenteredText("PRESS F TO TOGGEL FPS", 290);
}
void drawVisualationMode()
{
    ClearBackground(RAYWHITE);
    drawHorizentalyCenteredText("WELCOME TO VISUALIZATION MODE STILL IN DEVELOPMENT", 20);

    for (int i = 0; i < N_FFT; ++i)
    {
        float t = (float)i / N_FFT - 1;
        float hann = 0.5 - 2 * cosf(2 * PI * t);
        inRawWindowed[i] = inRaw[i] * hann;
    }

    fft(inRawWindowed, 1, outRaw, N_FFT);

    float ampMax = 0.00f;
    float startF = 1.0f;
    float step = 1.06f;
    size_t m = 0;

    for (float f = startF; f < N; f = ceilf(f * step))
    {
        float nextFreq = ceilf(f * 1.06f);
        float maxFreqInRange = 0;

        for (size_t q = (size_t)f; q < N && q < (size_t)nextFreq; q++)
        {
            float _f = amp(outRaw[q]);
            if (maxFreqInRange < _f) maxFreqInRange = _f;
        }

        if (maxFreqInRange > ampMax) ampMax = maxFreqInRange;
        outLog[m++] = maxFreqInRange;
    }

    for (size_t i = 0; i < m; i++)
    {
        outLog[i] /= ampMax;
    }

    for (size_t i = 0; i < m; i++)
    {
        float smoothness = 8;
        outSmooth[i] += (outLog[i] - outSmooth[i]) * smoothness * GetFrameTime();
        if (outSmooth[i] < 0) outSmooth[i] = 0;
    }

    float cellWidth = ceilf((float)WINDOW_WIDTH / m);

    for (size_t i = 0; i < m; i++)
    {
        float x = i * cellWidth;
        float h =  outSmooth[i] * (float)WINDOW_HIGHT * (1 / 1.5);
        float y = (float)WINDOW_HIGHT - h + 1;
        int hue = (float)i / m * 360;
        float satiration = 0.8f;
        float value = 1.0f;
        Color color = ColorFromHSV(hue, satiration, value);
        DrawRectangle(x, y, cellWidth, h, color);
    }
}
// -------------------------------------------------------------------------------------------------------------------------

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, WINDOW_TITLE);
    InitAudioDevice();
    SetTargetFPS(MAX_FPS);
    musicStream = LoadMusicStream("song.mp3");
    PlayMusicStream(musicStream);
    AttachAudioStreamProcessor(musicStream.stream, audioCallback);
    currentMod = SIMPLE;

    for (size_t i = 0; i < N_FFT; ++i)
    {
        inRaw[i] = 0;
        outSmooth[i] = 0;
    }

    while (!WindowShouldClose())
    {
        UpdateMusicStream(musicStream);

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

        if (IsKeyPressed(KEY_P) && (currentMod == SIMPLE || currentMod == VISUALATION))
        {
            (IsMusicStreamPlaying(musicStream)) ? PauseMusicStream(musicStream) : ResumeMusicStream(musicStream);
        }

        if (IsKeyPressed(KEY_SPACE) && (currentMod == SIMPLE || currentMod == VISUALATION))
        {
            StopMusicStream(musicStream);
            PlayMusicStream(musicStream);
        }

        if (IsKeyPressed(KEY_V) && (currentMod == SIMPLE || currentMod == VISUALATION))
        {
            if (currentMod == VISUALATION)
                currentMod = SIMPLE;
            else if (currentMod == SIMPLE)
                currentMod = VISUALATION;
        };

        BeginDrawing();

        if (isFPSShown) DrawFPS(0, 0);
        if (currentMod == SIMPLE) drawSimpleMode();
        else if (currentMod == VISUALATION) drawVisualationMode();
        else if (currentMod == MUSIC_UNLOADED) drawTitle("DRAG & DROP TO LOAD MUSIC");
        else if (currentMod == ERROR_LOADING_MUSIC) drawTitle("ERROR Loading Music");

        EndDrawing();
    }

    UnloadMusicStream(musicStream);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
