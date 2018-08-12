#include "display.h"
#include "YBaseLib/Assert.h"
#include "display_d3d.h"
#include "display_gl.h"
#include <SDL.h>

namespace SDLFrontend {
Display::Display() {}

Display::~Display()
{
  if (m_window)
    SDL_DestroyWindow(m_window);
}

std::unique_ptr<Display> Display::Create()
{
  std::unique_ptr<Display> display;

  // Use D3D on windows by default, otherwise GL.
#if defined(Y_PLATFORM_WINDOWS) && true
  display = std::make_unique<DisplayD3D>();
#else
  display = std::make_unique<DisplayGL>();
#endif

  if (!display->Initialize())
    display.reset();

  return display;
}

bool Display::Initialize()
{
  const uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
  m_window = SDL_CreateWindow("SDL Display", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_display_width,
                              m_display_height, flags | GetAdditionalWindowCreateFlags());
  if (!m_window)
    return false;

  return true;
}

void Display::ResizeDisplay(uint32 width /*= 0*/, uint32 height /*= 0*/)
{
  ::Display::ResizeDisplay(width, height);
  SDL_SetWindowSize(m_window, static_cast<int>(m_display_width), static_cast<int>(m_display_height));
  // Don't do anything when it's maximized or fullscreen
  // if (SDL_GetWindowFlags(m_window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN))
  // return;
}

bool Display::IsFullscreen() const
{
  return ((SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN) != 0);
}

void Display::SetFullscreen(bool enable)
{
  SDL_SetWindowFullscreen(m_window, enable ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

bool Display::HandleSDLEvent(const SDL_Event* ev)
{
  if (ev->type == SDL_WINDOWEVENT &&
      (ev->window.event == SDL_WINDOWEVENT_SIZE_CHANGED || ev->window.event == SDL_WINDOWEVENT_RESIZED))
  {
    OnWindowResized();
  }

  return false;
}

void Display::OnWindowResized()
{
  int width, height;
  SDL_GetWindowSize(m_window, &width, &height);
  m_display_width = width;
  m_display_height = height;
}

void Display::CalculateDrawRectangle(s32* x, s32* y, u32* width, u32* height)
{
  // Calculate render rectangle based on aspect ratio.
  int window_width = int(m_display_width);
  int window_height = std::max(1, int(m_display_height));
  float display_ratio = float(m_display_aspect_numerator) / float(m_display_aspect_denominator);
  float window_ratio = float(window_width) / float(window_height);
  if (window_ratio >= display_ratio)
  {
    *width = u32(float(window_height) * display_ratio);
    *height = u32(window_height);
  }
  else
  {
    *width = u32(window_width);
    *height = u32(float(window_width) / display_ratio);
  }

  *x = (window_width - s32(*width)) / 2;
  *y = (window_height - s32(*height)) / 2;
}

} // namespace SDLFrontend