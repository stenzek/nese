#pragma once
#include "common/display.h"
#include "common/types.h"
#include <memory>
#include <vector>

union SDL_Event;
struct SDL_Window;

namespace SDLFrontend {
class Display : public ::Display
{
public:
  Display();
  ~Display();

  static std::unique_ptr<Display> Create();

  virtual void ResizeDisplay(u32 width = 0, u32 height = 0) override;

  virtual bool HandleSDLEvent(const SDL_Event* ev);

  SDL_Window* GetSDLWindow() const { return m_window; }

  bool IsFullscreen() const;
  void SetFullscreen(bool enable);

protected:
  virtual u32 GetAdditionalWindowCreateFlags() { return 0; }
  virtual bool Initialize();
  virtual void OnWindowResized();

  SDL_Window* m_window = nullptr;
};

} // namespace SDLFrontend