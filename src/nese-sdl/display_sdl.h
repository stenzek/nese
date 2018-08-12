#pragma once
#include "common/display.h"
#include "common/types.h"
#include <memory>
#include <vector>

struct SDL_Window;

class DisplaySDL : public Display
{
public:
  DisplaySDL();
  ~DisplaySDL();

  void ResizeDisplay(uint32 width = 0, uint32 height = 0) override;
  void ResizeFramebuffer(uint32 width, uint32 height) override;
  void DisplayFramebuffer() override;

  SDL_Window* GetSDLWindow() const { return m_window; }

  bool IsFullscreen() const;
  void SetFullscreen(bool enable);

protected:
  virtual uint32 GetAdditionalWindowCreateFlags() { return 0; }
  virtual bool Initialize();
  virtual void OnWindowResized();
  virtual void RenderImpl() = 0;

  SDL_Window* m_window = nullptr;
  uint32 m_window_width = 0;
  uint32 m_window_height = 0;

  std::vector<u32> m_framebuffer_data;
};
