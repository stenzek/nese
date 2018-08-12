#include "display_sdl.h"
#include "YBaseLib/Assert.h"
#include <SDL.h>
//#include "imgui.h"

DisplaySDL::DisplaySDL()
{
  m_window_width = 512;
  m_window_height = 512;
}

DisplaySDL::~DisplaySDL()
{
  if (m_window)
    SDL_DestroyWindow(m_window);
}

bool DisplaySDL::Initialize()
{
  const uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
  m_window = SDL_CreateWindow("PCE - Initializing...", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_window_width,
                              m_window_height, flags | GetAdditionalWindowCreateFlags());
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

void DisplaySDL::ResizeFramebuffer(uint32 width, uint32 height)
{
  if (m_framebuffer_width != width || m_framebuffer_height != height)
  {
    m_framebuffer_width = width;
    m_framebuffer_height = height;
    m_framebuffer_data.resize(m_framebuffer_width * m_framebuffer_height);
    m_framebuffer_pointer = reinterpret_cast<u8*>(m_framebuffer_data.data());
    m_framebuffer_pitch = width * sizeof(u32);
  }
}

void DisplaySDL::DisplayFramebuffer()
{
  RenderImpl();
  AddFrameRendered();
}

bool DisplaySDL::IsFullscreen() const
{
  return ((SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN) != 0);
}

void DisplaySDL::SetFullscreen(bool enable)
{
  SDL_SetWindowFullscreen(m_window, enable ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void DisplaySDL::OnWindowResized()
{
  int width, height;
  SDL_GetWindowSize(m_window, &width, &height);
  m_window_width = width;
  m_window_height = height;
}
