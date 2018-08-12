#include "displaywidget.h"
#include "YBaseLib/Assert.h"
#include "YBaseLib/Memory.h"
#include "YBaseLib/String.h"
#include <QtOpenGL/QGL>
#include <QtOpenGL/QGLContext>
#include <QtOpenGL/QGLFunctions>
#include <algorithm>

namespace QtFrontend {

DisplayWidget::DisplayWidget(QWidget* parent) : QGLWidget(parent), Display()
{
  setFocusPolicy(Qt::FocusPolicy::StrongFocus);

  QGLFormat format;
  format.setProfile(QGLFormat::NoProfile);
  format.setVersion(2, 0);
  format.setRedBufferSize(8);
  format.setGreenBufferSize(8);
  format.setBlueBufferSize(8);
  format.setSwapInterval(0);
  setFormat(format);
  setAutoBufferSwap(false);
}

DisplayWidget::~DisplayWidget()
{
  makeCurrent();

#if 0
  if (m_framebuffer_texture != 0)
    glDeleteTextures(1, &m_framebuffer_texture);
#endif

  doneCurrent();
}

void DisplayWidget::ResizeDisplay(uint32 width /*= 0*/, uint32 height /*= 0*/)
{
  Display::ResizeDisplay(width, height);

  // Cause the parent window to resize to the content
  // FIXME for thread
  // updateGeometry();
  // parentWidget()->adjustSize();
}

void DisplayWidget::ResizeFramebuffer(uint32 width, uint32 height)
{
  if (m_framebuffer_width == width && m_framebuffer_height == height)
    return;

  makeCurrent();

#if 0
  if (m_framebuffer_texture == 0)
    glGenTextures(1, &m_framebuffer_texture);

  glBindTexture(GL_TEXTURE_2D, m_framebuffer_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  uint32 pitch = sizeof(byte) * 4 * width;
  std::unique_ptr<byte[]> buffer = std::make_unique<byte[]>(pitch * height);

  // Copy as much as possible in
  if (m_framebuffer_pointer)
  {
    uint32 copy_width = std::min(m_framebuffer_width, width);
    uint32 copy_height = std::min(m_framebuffer_height, height);
    Y_memzero(buffer.get(), pitch * height);
    Y_memcpy_stride(buffer.get(), pitch, m_framebuffer_pointer, m_framebuffer_pitch, copy_width * 4, copy_height);
  }

  m_framebuffer_width = width;
  m_framebuffer_height = height;
  m_framebuffer_pitch = pitch;
  m_framebuffer_texture_buffer = std::move(buffer);
  m_framebuffer_pointer = m_framebuffer_texture_buffer.get();
#endif

  doneCurrent();
  update();
}

void DisplayWidget::DisplayFramebuffer()
{
  AddFrameRendered();

  auto gl = QGLContext::currentContext()->functions();

  makeCurrent();

#if 0
  gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  gl->glClear(GL_COLOR_BUFFER_BIT);

  // glViewport already done
  gl->glMatrixMode(GL_MODELVIEW);
  gl->glLoadIdentity();
  gl->glMatrixMode(GL_PROJECTION);
  gl->glLoadIdentity();

  gl->glEnable(GL_TEXTURE_2D);
  gl->glBindTexture(GL_TEXTURE_2D, m_framebuffer_texture);
  gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_framebuffer_width, m_framebuffer_height, GL_RGBA, GL_UNSIGNED_BYTE,
                  m_framebuffer_pointer);

  gl->glViewport(0, 0, width(), height());

  gl->glBegin(GL_QUADS);
  gl->glTexCoord2f(0.0f, 0.0f);
  gl->glVertex4f(-1.0f, 1.0f, 0.0f, 1.0f);
  gl->glTexCoord2f(0.0f, 1.0f);
  gl->glVertex4f(-1.0f, -1.0f, 0.0f, 1.0f);
  gl->glTexCoord2f(1.0f, 1.0f);
  gl->glVertex4f(1.0f, -1.0f, 0.0f, 1.0f);
  gl->glTexCoord2f(1.0f, 0.0f);
  gl->glVertex4f(1.0f, 1.0f, 0.0f, 1.0f);
  gl->glEnd();
#endif

  swapBuffers();
  doneCurrent();
}

QSize DisplayWidget::sizeHint() const
{
  return QSize(static_cast<int>(std::max(m_display_width, 1u)), static_cast<int>(std::max(m_display_height, 1u)));
}

void DisplayWidget::moveGLContextToThread(QThread* thread)
{
  context()->moveToThread(thread);
}

void DisplayWidget::keyPressEvent(QKeyEvent* event)
{
  emit onKeyPressed(event);
}

void DisplayWidget::keyReleaseEvent(QKeyEvent* event)
{
  emit onKeyReleased(event);
}

void DisplayWidget::paintEvent(QPaintEvent* e) {}

void DisplayWidget::resizeEvent(QResizeEvent* e) {}

void DisplayWidget::initializeGL() {}

void DisplayWidget::paintGL() {}

void DisplayWidget::resizeGL(int w, int h) {}

} // namespace QtFrontend