#pragma once
#include "common/display.h"
#include "common/types.h"
#include <memory>
#include <vector>

union SDL_Event;
struct SDL_Window;

namespace SDLFrontend {
class DisplaySDL : public Display
{
public:
  DisplaySDL();
  ~DisplaySDL();

  virtual void ResizeDisplay(u32 width = 0, u32 height = 0) override;
  virtual void ResizeFramebuffer(u32 width, u32 height) override;

  SDL_Window* GetSDLWindow() const { return m_window; }

  bool IsFullscreen() const;
  void SetFullscreen(bool enable);

  virtual bool HandleSDLEvent(const SDL_Event* ev);

protected:
  virtual u32 GetAdditionalWindowCreateFlags() { return 0; }
  virtual bool Initialize();
  virtual void OnWindowResized();

  SDL_Window* m_window = nullptr;

  std::vector<u32> m_framebuffer_data;
};

} // namespace SDLFrontend