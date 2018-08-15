#pragma once

#include "common/display.h"
#include <QtGui/QWindow>
#include <memory>

class QOpenGLContext;

namespace QtFrontend {
class DisplayWindow : public QWindow, public Display
{
  Q_OBJECT

public:
  DisplayWindow(QWindow* parent = nullptr);
  ~DisplayWindow();

  void ClearFramebuffer();
  void ResizeDisplay(u32 width = 0, u32 height = 0) override;
  void ResizeFramebuffer(u32 width, u32 height) override;
  void DisplayFramebuffer() override;

  bool createOpenGLObjects();
  void moveOpenGLContextToThread(QThread* thread);
  void makeOpenGLContextCurrent();

Q_SIGNALS:
  void keyPressed(QKeyEvent* event);
  void keyReleased(QKeyEvent* event);

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  void destroyOpenGLObjects();

  QOpenGLContext* m_gl_context = nullptr;
  std::vector<u32> m_framebuffer_data;

  unsigned int m_vbo = 0;
  unsigned int m_vao = 0;
  unsigned int m_program = 0;
  unsigned int m_framebuffer_texture = 0;
};
} // namespace QtFrontend