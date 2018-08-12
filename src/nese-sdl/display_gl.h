#pragma once

#include "display_sdl.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <vector>

namespace SDLFrontend {
class DisplayGL : public DisplaySDL
{
public:
  DisplayGL();
  ~DisplayGL();

  static std::unique_ptr<DisplayGL> Create();

  void ResizeFramebuffer(uint32 width, uint32 height) override;
  void DisplayFramebuffer() override;

protected:
  virtual u32 GetAdditionalWindowCreateFlags() override;

private:
  bool Initialize() override;

  SDL_GLContext m_gl_context = nullptr;
  GLuint m_framebuffer_texture = 0;
  std::vector<u32> m_framebuffer_data;
};
} // namespace SDLFrontend