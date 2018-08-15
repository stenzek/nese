#include "displaywindow.h"
#include "YBaseLib/Assert.h"
#include "YBaseLib/Memory.h"
#include "YBaseLib/String.h"
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLFunctions_3_0>
#include <QtGui/QResizeEvent>
#include <algorithm>
#include <sstream>

namespace QtFrontend {

DisplayWindow::DisplayWindow(QWindow* parent) : QWindow(parent), Display()
{
  QSurfaceFormat format;
  format.setVersion(3, 0);
  format.setRenderableType(QSurfaceFormat::OpenGL);
  setFormat(format);
}

DisplayWindow::~DisplayWindow()
{
  m_gl_context->makeCurrent(this);
  destroyOpenGLObjects();
  m_gl_context->doneCurrent();
  delete m_gl_context;
}

void DisplayWindow::moveOpenGLContextToThread(QThread* thread)
{
  m_gl_context->doneCurrent();
  m_gl_context->moveToThread(thread);
}

void DisplayWindow::makeOpenGLContextCurrent()
{
  if (!m_gl_context->makeCurrent(this))
    Panic("Failed to make OpenGL context current.");
}

void DisplayWindow::resizeEvent(QResizeEvent* event)
{
  m_display_width = event->size().width();
  m_display_height = event->size().height();
}

void DisplayWindow::keyPressEvent(QKeyEvent* event)
{
  emit keyPressed(event);
}

void DisplayWindow::keyReleaseEvent(QKeyEvent* event)
{
  emit keyReleased(event);
}

static std::string GenerateVertexShader(const bool old_glsl)
{
  std::stringstream ss;
  if (old_glsl)
  {
    ss << "#version 110\n";
    ss << "attribute vec4 a_position;\n";
    ss << "attribute vec2 a_texcoord;\n";
    ss << "varying vec2 v_texcoord;\n";
  }
  else
  {
    ss << "#version 130\n";
    ss << "in vec4 a_position;\n";
    ss << "in vec2 a_texcoord;\n";
    ss << "out vec2 v_texcoord;\n";
  }

  ss << "void main() {\n";
  ss << "  gl_Position = a_position;\n";
  ss << "  v_texcoord = a_texcoord;\n";
  ss << "}\n";

  return ss.str();
}

static std::string GenerateFragmentShader(const bool old_glsl)
{
  std::stringstream ss;
  if (old_glsl)
  {
    ss << "#version 110\n";
    ss << "varying vec2 v_texcoord;\n";
    ss << "uniform sampler2D samp0;\n";
  }
  else
  {
    ss << "#version 130\n";
    ss << "in vec2 v_texcoord;\n";
    ss << "uniform sampler2D samp0;\n";
    ss << "out vec4 ocol0;\n";
  }

  ss << "void main() {\n";
  if (old_glsl)
    ss << "  gl_FragColor = texture2D(samp0, v_texcoord);\n";
  else
    ss << "  ocol0 = texture(samp0, v_texcoord);\n";
  ss << "}\n";

  return ss.str();
}

bool DisplayWindow::createOpenGLObjects()
{
  // Create the OpenGL context.
  m_gl_context = new QOpenGLContext();
  m_gl_context->setFormat(format());
  if (!m_gl_context->create())
    Panic("Failed to create GL context.");

  m_gl_context->makeCurrent(this);

  auto gl = m_gl_context->versionFunctions<QOpenGLFunctions_3_0>();

  //////////////////////////////////////////////////////////////////////////
  // VAO/VAO
  //////////////////////////////////////////////////////////////////////////

  struct Vertex
  {
    float position[4];
    float texcoord[2];
  };
  static const Vertex vertices[4] = {{{1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
                                     {{-1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
                                     {{1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                                     {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}};

  gl->glGenVertexArrays(1, &m_vao);
  gl->glBindVertexArray(m_vao);
  gl->glGenBuffers(1, &m_vbo);
  gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  gl->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                            reinterpret_cast<void*>(offsetof(Vertex, position[0])));
  gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                            reinterpret_cast<void*>(offsetof(Vertex, texcoord[0])));
  gl->glEnableVertexAttribArray(0);
  gl->glEnableVertexAttribArray(1);

  //////////////////////////////////////////////////////////////////////////
  // Shaders
  //////////////////////////////////////////////////////////////////////////
  const auto format = m_gl_context->format();
  const bool old_glsl = (format.majorVersion() < 3 || (format.majorVersion() == 3 && format.minorVersion() < 2));
  const std::string vs_str = GenerateVertexShader(old_glsl);
  const std::string fs_str = GenerateFragmentShader(old_glsl);
  const char* vs_str_ptr = vs_str.c_str();
  const GLint vs_length = static_cast<GLint>(vs_str.length());
  const char* fs_str_ptr = fs_str.c_str();
  const GLint fs_length = static_cast<GLint>(fs_str.length());
  GLint param;

  GLuint vs = gl->glCreateShader(GL_VERTEX_SHADER);
  gl->glShaderSource(vs, 1, &vs_str_ptr, &vs_length);
  gl->glCompileShader(vs);
  gl->glGetShaderiv(vs, GL_COMPILE_STATUS, &param);
  if (param != GL_TRUE)
  {
    Panic("Failed to compile vertex shader.");
    return false;
  }

  GLuint fs = gl->glCreateShader(GL_FRAGMENT_SHADER);
  gl->glShaderSource(fs, 1, &fs_str_ptr, &fs_length);
  gl->glCompileShader(fs);
  gl->glGetShaderiv(fs, GL_COMPILE_STATUS, &param);
  if (param != GL_TRUE)
  {
    Panic("Failed to compile fragment shader.");
    return false;
  }

  m_program = gl->glCreateProgram();
  gl->glAttachShader(m_program, vs);
  gl->glAttachShader(m_program, fs);
  gl->glBindAttribLocation(m_program, 0, "a_position");
  gl->glBindAttribLocation(m_program, 1, "a_texcoord");
  if (!old_glsl)
    gl->glBindFragDataLocation(m_program, 0, "ocol0");
  gl->glLinkProgram(m_program);
  gl->glGetProgramiv(m_program, GL_LINK_STATUS, &param);
  if (param != GL_TRUE)
  {
    Panic("Failed to link program.");
    return false;
  }

  // Bind texture unit zero to the shader.
  gl->glUseProgram(m_program);
  GLint pos = gl->glGetUniformLocation(m_program, "samp0");
  if (pos >= 0)
    gl->glUniform1i(pos, 0);

  // Shaders are no longer needed after linking.
  gl->glDeleteShader(vs);
  gl->glDeleteShader(fs);
  return true;
}

void DisplayWindow::destroyOpenGLObjects()
{
  auto gl = m_gl_context->versionFunctions<QOpenGLFunctions_3_0>();
  if (m_framebuffer_texture != 0)
    gl->glDeleteTextures(1, &m_framebuffer_texture);
  if (m_program != 0)
    gl->glDeleteProgram(m_program);
  if (m_vao != 0)
    gl->glDeleteVertexArrays(1, &m_vao);
  if (m_vbo != 0)
    gl->glDeleteBuffers(1, &m_vbo);
}

void DisplayWindow::ClearFramebuffer()
{
  std::fill(m_framebuffer_data.begin(), m_framebuffer_data.end(), u32(0));
  DisplayFramebuffer();
}

void DisplayWindow::ResizeDisplay(uint32 width /*= 0*/, uint32 height /*= 0*/)
{
  Display::ResizeDisplay(width, height);

  // Cause the parent window to resize to the content
  // FIXME for thread
  // updateGeometry();
  // parentWidget()->adjustSize();
}

void DisplayWindow::ResizeFramebuffer(uint32 width, uint32 height)
{
  if (m_framebuffer_texture != 0 && m_framebuffer_width == width && m_framebuffer_height == height)
    return;

  // Update buffer.
  m_framebuffer_width = width;
  m_framebuffer_height = height;
  m_framebuffer_data = std::vector<u32>(width * height);
  m_framebuffer_pointer = reinterpret_cast<byte*>(m_framebuffer_data.data());
  m_framebuffer_pitch = sizeof(u32) * width;

  // Update GL texture.
  auto gl = m_gl_context->versionFunctions<QOpenGLFunctions_3_0>();
  if (m_framebuffer_texture == 0)
    gl->glGenTextures(1, &m_framebuffer_texture);

  gl->glBindTexture(GL_TEXTURE_2D, m_framebuffer_texture);
  gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_framebuffer_data.data());
  gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  // gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void DisplayWindow::DisplayFramebuffer()
{
  AddFrameRendered();

  // This shouldn't be needed, but it complains if we don't do it after swapping...
  if (!m_gl_context->makeCurrent(this))
    return;

  auto gl = m_gl_context->versionFunctions<QOpenGLFunctions_3_0>();

  // Upload new texture.
  gl->glBindTexture(GL_TEXTURE_2D, m_framebuffer_texture);
  gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_framebuffer_width, m_framebuffer_height, GL_RGBA, GL_UNSIGNED_BYTE,
                      m_framebuffer_data.data());

  // Clear area outside where we're rendering.
  gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  gl->glClear(GL_COLOR_BUFFER_BIT);

  s32 x, y;
  u32 viewport_width, viewport_height;
  CalculateDrawRectangle(&x, &y, &viewport_width, &viewport_height);
  gl->glViewport(x, y, viewport_width, viewport_height);

  // Draw. Texture is already bound.
  gl->glDisable(GL_SCISSOR_TEST);
  gl->glDisable(GL_DEPTH_TEST);
  gl->glDisable(GL_BLEND);
  gl->glUseProgram(m_program);
  gl->glBindVertexArray(m_vao);
  gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // And swap.
  m_gl_context->swapBuffers(this);
}

} // namespace QtFrontend