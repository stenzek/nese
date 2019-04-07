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
#include <map>
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

static void HandleKeyEvent(const SDL_Event* ev, StandardController* nes_controller)
{
  const bool down = (ev->type == SDL_KEYDOWN);
  switch (ev->key.keysym.sym)
  {
    case SDLK_w:
    case SDLK_UP:
      nes_controller->SetButtonState(StandardController::Button_Up, down);
      break;
    case SDLK_s:
    case SDLK_DOWN:
      nes_controller->SetButtonState(StandardController::Button_Down, down);
      break;
    case SDLK_a:
    case SDLK_LEFT:
      nes_controller->SetButtonState(StandardController::Button_Left, down);
      break;
    case SDLK_d:
    case SDLK_RIGHT:
      nes_controller->SetButtonState(StandardController::Button_Right, down);
      break;

    case SDLK_z:
      nes_controller->SetButtonState(StandardController::Button_B, down);
      break;
    case SDLK_x:
      nes_controller->SetButtonState(StandardController::Button_A, down);
      break;

    case SDLK_RETURN:
      nes_controller->SetButtonState(StandardController::Button_Start, down);
      break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
      nes_controller->SetButtonState(StandardController::Button_Select, down);
      break;
  }
}

static void HandleControllerAxisEvent(const SDL_Event* ev, StandardController* nes_controller)
{
  // Log_DevPrintf("axis %d %d", ev->caxis.axis, ev->caxis.value);

  // use both analog sticks
  if (ev->caxis.axis > SDL_CONTROLLER_AXIS_RIGHTY)
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
  // Log_DevPrintf("button %d %s", ev->cbutton.button, ev->cbutton.state == SDL_PRESSED ? "pressed" : "released");

  // For xbox one controller..
  static constexpr std::pair<SDL_GameControllerButton, u8> button_mapping[] = {
    {SDL_CONTROLLER_BUTTON_A, StandardController::Button_A},
    {SDL_CONTROLLER_BUTTON_B, StandardController::Button_B},
    {SDL_CONTROLLER_BUTTON_X, StandardController::Button_A},
    {SDL_CONTROLLER_BUTTON_Y, StandardController::Button_B},
    {SDL_CONTROLLER_BUTTON_BACK, StandardController::Button_Select},
    {SDL_CONTROLLER_BUTTON_START, StandardController::Button_Start},
    {SDL_CONTROLLER_BUTTON_GUIDE, StandardController::Button_Start},
    {SDL_CONTROLLER_BUTTON_DPAD_UP, StandardController::Button_Up},
    {SDL_CONTROLLER_BUTTON_DPAD_DOWN, StandardController::Button_Down},
    {SDL_CONTROLLER_BUTTON_DPAD_LEFT, StandardController::Button_Left},
    {SDL_CONTROLLER_BUTTON_DPAD_LEFT, StandardController::Button_Right}};

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

  std::map<int, SDL_GameController*> controllers;
  for (int i = 0; i < SDL_NumJoysticks(); i++)
  {
    SDL_GameController* gcontroller = SDL_GameControllerOpen(i);
    if (gcontroller)
    {
      Log_InfoPrintf("Opened controller %d: %s", i, SDL_GameControllerName(gcontroller));
      controllers.emplace(i, gcontroller);
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
          HandleKeyEvent(&ev, controller.get());
          if (ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_PAUSE)
            system->Reset();
        }
        break;

        case SDL_CONTROLLERDEVICEADDED:
        {
          auto iter = controllers.find(ev.cdevice.which);
          if (iter == controllers.end())
          {
            SDL_GameController* gcontroller = SDL_GameControllerOpen(ev.cdevice.which);
            if (gcontroller)
            {
              Log_InfoPrintf("Controller %s inserted", SDL_GameControllerName(gcontroller));
              controllers.emplace(ev.cdevice.which, gcontroller);
            }
          }
        }
        break;

        case SDL_CONTROLLERDEVICEREMOVED:
        {
          auto iter = controllers.find(ev.cdevice.which);
          if (iter != controllers.end())
          {
            Log_InfoPrintf("Controller %s removed", SDL_GameControllerName(iter->second));
            SDL_GameControllerClose(iter->second);
            controllers.erase(iter);
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

  for (auto& gc : controllers)
    SDL_GameControllerClose(gc.second);

  SDL_Quit();
  return EXIT_SUCCESS;
}