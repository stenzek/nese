#define SDL_MAIN_HANDLED 1
#include "YBaseLib/ByteStream.h"
#include "YBaseLib/Error.h"
#include "YBaseLib/Log.h"
#include "audio.h"
#include "nese-sdl/display_d3d.h"
#include "nese-sdl/display_gl.h"
#include "nese/cartridge.h"
#include "nese/controller.h"
#include "nese/system.h"
#include <SDL/SDL.h>
#include <cstdio>
#include <cstdlib>
Log_SetChannel(Main);

bool g_running = true;

static std::unique_ptr<Cartridge> LoadCartridge(const char* filename)
{
  ByteStream* stream;
  if (!ByteStream_OpenFileStream(filename, BYTESTREAM_OPEN_READ | BYTESTREAM_OPEN_STREAMED, &stream))
  {
    Log_ErrorPrintf("Failed to open file '%s'.", filename);
    return nullptr;
  }

  Error load_error;
  std::unique_ptr<Cartridge> cart = Cartridge::Load(stream, &load_error);
  stream->Release();

  if (!cart)
  {
    Log_ErrorPrintf("Cartridge load error: %s", load_error.GetErrorDescription().GetCharArray());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Cartridge load error",
                             load_error.GetErrorDescription().GetCharArray(), nullptr);
  }

  return cart;
}

static void HandleControllerAxisEvent(const SDL_Event* ev, StandardController* nes_controller)
{
  printf("axis %d %d\n", ev->caxis.axis, ev->caxis.value);

  // use both analog sticks
  if (ev->caxis.axis >= 4)
    return;

  u8 negative_button, positive_button;
  if (ev->caxis.axis & 1)
  {
    negative_button = StandardController::Button_Up;
    positive_button = StandardController::Button_Down;
  }
  else
  {
    negative_button = StandardController::Button_Left;
    positive_button = StandardController::Button_Right;
  }

  static constexpr int deadzone = 5000;
  const bool negative = (ev->caxis.value < 0);
  const bool active = (std::abs(ev->caxis.value) >= deadzone);

  nes_controller->SetButtonState(negative_button, negative && active);
  nes_controller->SetButtonState(positive_button, !negative && active);
}

static void HandleControllerButtonEvent(const SDL_Event* ev, StandardController* nes_controller)
{
  // printf("button %d %s\n", ev->cbutton.button, ev->cbutton.state == SDL_PRESSED ? "pressed" : "released");

  // For xbox one controller..
  static constexpr std::pair<u8, u8> button_mapping[] = {
    {0, StandardController::Button_A},      {1, StandardController::Button_B},
    {4, StandardController::Button_Select}, {6, StandardController::Button_Start},
    {11, StandardController::Button_Up},    {12, StandardController::Button_Down},
    {13, StandardController::Button_Left},  {14, StandardController::Button_Right}};

  for (const auto& bm : button_mapping)
  {
    if (bm.first == ev->cbutton.button)
    {
      nes_controller->SetButtonState(bm.second, ev->cbutton.state == SDL_PRESSED);
      break;
    }
  }
}

int main(int argc, char* argv[])
{
  // set log flags
  g_pLog->SetConsoleOutputParams(true);
  // g_pLog->SetDebugOutputParams(true);

  if (argc < 2)
  {
    std::fprintf(stderr, "usage: %s <path to .nes>\n", argv[0]);
    return EXIT_FAILURE;
  }

  std::unique_ptr<Cartridge> cart = LoadCartridge(argv[1]);
  if (!cart)
    return EXIT_FAILURE;

  // init sdl
  if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
  {
    Panic("SDL initialization failed");
    return EXIT_FAILURE;
  }

  std::vector<SDL_GameController*> controllers;
  for (int i = 0; i < SDL_NumJoysticks(); i++)
  {
    SDL_GameController* gcontroller = SDL_GameControllerOpen(i);
    if (gcontroller)
    {
      Log_InfoPrintf("Opened controller %d: %s", i, SDL_GameControllerName(gcontroller));
      controllers.push_back(gcontroller);
    }
    else
    {
      Log_WarningPrintf("Failed to open controller %d", i);
    }
  }

  std::unique_ptr<SDLFrontend::Audio> audio = std::make_unique<SDLFrontend::Audio>();
  std::unique_ptr<SDLFrontend::Display> display = SDLFrontend::Display::Create();
  if (!display)
    return EXIT_FAILURE;

  std::unique_ptr<StandardController> controller = std::make_unique<StandardController>();

  std::unique_ptr<System> system = std::make_unique<System>();
  system->Initialize(display.get(), audio.get(), cart.get());
  system->SetController(0, controller.get());
  system->Reset();

  display->SetDisplayScale(2);
  display->ResizeDisplay();

  while (g_running)
  {
    system->FrameStep();

    // SDL event loop...
    for (;;)
    {
      SDL_Event ev;
      if (!SDL_PollEvent(&ev))
        break;

      if (display->HandleSDLEvent(&ev))
        continue;

      switch (ev.type)
      {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
          const bool down = (ev.type == SDL_KEYDOWN);
          switch (ev.key.keysym.sym)
          {
            case SDLK_w:
            case SDLK_UP:
              controller->SetButtonState(StandardController::Button_Up, down);
              break;
            case SDLK_s:
            case SDLK_DOWN:
              controller->SetButtonState(StandardController::Button_Down, down);
              break;
            case SDLK_a:
            case SDLK_LEFT:
              controller->SetButtonState(StandardController::Button_Left, down);
              break;
            case SDLK_d:
            case SDLK_RIGHT:
              controller->SetButtonState(StandardController::Button_Right, down);
              break;

            case SDLK_z:
              controller->SetButtonState(StandardController::Button_B, down);
              break;
            case SDLK_x:
              controller->SetButtonState(StandardController::Button_A, down);
              break;

            case SDLK_RETURN:
              controller->SetButtonState(StandardController::Button_Start, down);
              break;
            case SDLK_LCTRL:
            case SDLK_RCTRL:
              controller->SetButtonState(StandardController::Button_Select, down);
              break;
          }

          if (ev.type == SDL_KEYUP)
          {
            switch (ev.key.keysym.sym)
            {
              case SDLK_PAUSE:
                system->Reset();
                break;
            }
          }
        }
        break;

        case SDL_CONTROLLERAXISMOTION:
          HandleControllerAxisEvent(&ev, controller.get());
          break;

        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
          HandleControllerButtonEvent(&ev, controller.get());
          break;

        case SDL_QUIT:
          g_running = false;
          break;
      }
    }
  }

  for (SDL_GameController* controller : controllers)
    SDL_GameControllerClose(controller);

  SDL_Quit();
  return EXIT_SUCCESS;
}