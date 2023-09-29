#include <raylib.h>

#include "constants.h"

int main(int argc, char const *argv[]) {

  (void)argc;
  (void)argv;

  InitWindow(WINDOW_WIDTH, WINDOW_HIGHT, WINDOW_TITLE);
  InitAudioDevice();
  
  while (!WindowShouldClose()) {
  	
  }

  return 0;
}


