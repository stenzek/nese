#include "display_sdl.h"
#include "YBaseLib/Assert.h"
#include <SDL.h>

namespace SDLFrontend {
DisplaySDL::DisplaySDL() {}

DisplaySDL::~DisplaySDL()
{
  if (m_window)
    SDL_DestroyWindow(m_window);
}

bool DisplaySDL::Initialize()
{
  const uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
  m_window = SDL_CreateWindow("SDL Display", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_display_width,
                              m_display_height, flags | GetAdditionalWindowCreateFlags());
  if (!m_window)
    return false;

  return true;
}

void DisplaySDL::ResizeDisplay(uint32 width /*= 0*/, uint32 height /*= 0*/)
{
  Display::ResizeDisplay(width, height);
  SDL_SetWindowSize(m_window, static_cast<int>(m_display_width), static_cast<int>(m_display_height));
  // Don't do anything when it's maximized or fullscreen
  // if (SDL_GetWindowFlags(m_window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN))
  // return;
}

bool DisplaySDL::IsFullscreen() const
{
  return ((SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN) != 0);
}

void DisplaySDL::SetFullscreen(bool enable)
{
  SDL_SetWindowFullscreen(m_window, enable ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

bool DisplaySDL::HandleSDLEvent(const SDL_Event* ev)
{
  if (ev->type == SDL_WINDOWEVENT &&
      (ev->window.event == SDL_WINDOWEVENT_SIZE_CHANGED || ev->window.event == SDL_WINDOWEVENT_RESIZED))
  {
    OnWindowResized();
  }

  return false;
}

void DisplaySDL::OnWindowResized()
{
  int width, height;
  SDL_GetWindowSize(m_window, &width, &height);
  m_display_width = width;
  m_display_height = height;
}
} // namespace SDLFrontend