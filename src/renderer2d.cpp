#include "renderer2d.h"

#include "log.h"
#include "shader.h"
#include "texture/texture.h"

namespace Impacto {

static bool IsInit = false;

static GLuint ShaderProgramSprite;

void Renderer2D::Init() {
  if (IsInit) return;
  ImpLog(LL_Info, LC_Render, "Initializing Renderer2D system\n");
  IsInit = true;

  ShaderProgramSprite = ShaderCompile("Sprite");

  glUniform1i(glGetUniformLocation(ShaderProgramSprite, "ColorMap"), 0);
}

Renderer2D::Renderer2D() {
  // Fill index buffer with quads
  int index = 0;
  int vertex = 0;
  while (index + 6 <= IndexBufferCount) {
    // bottom-left -> top-left -> top-right
    IndexBuffer[index] = vertex + 0;
    IndexBuffer[index + 1] = vertex + 1;
    IndexBuffer[index + 2] = vertex + 2;
    // bottom-left -> top-right -> bottom-right
    IndexBuffer[index + 3] = vertex + 0;
    IndexBuffer[index + 4] = vertex + 2;
    IndexBuffer[index + 5] = vertex + 3;
    index += 6;
    vertex += 4;
  }

  // Generate buffers
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &IBO);
  glGenVertexArrays(1, &VAOSprites);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, VertexBufferSize, NULL, GL_STREAM_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               IndexBufferCount * sizeof(IndexBuffer[0]), IndexBuffer,
               GL_STATIC_DRAW);

  // Specify vertex layouts
  glBindVertexArray(VAOSprites);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBufferSprites),
                        (void*)offsetof(VertexBufferSprites, Position));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBufferSprites),
                        (void*)offsetof(VertexBufferSprites, UV));
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBufferSprites),
                        (void*)offsetof(VertexBufferSprites, Tint));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  // Make 1x1 white pixel for colored rectangles
  Texture rectTexture;
  rectTexture.Load1x1(0xFF, 0xFF, 0xFF, 0xFF);
  SpriteSheet rectSheet(1.0f, 1.0f);
  rectSheet.Texture = rectTexture.Submit();
  RectSprite = Sprite(rectSheet, 0.0f, 0.0f, 1.0f, 1.0f);
}

Renderer2D::~Renderer2D() {
  if (VBO) glDeleteBuffers(1, &VBO);
  if (IBO) glDeleteBuffers(1, &IBO);
  if (VAOSprites) glDeleteVertexArrays(1, &VAOSprites);
  if (RectSprite.Sheet.Texture) glDeleteTextures(1, &RectSprite.Sheet.Texture);
}

void Renderer2D::Begin() {
  if (Drawing) {
    ImpLog(LL_Error, LC_Render, "Renderer2D::Begin() called before Finish()\n");
    return;
  }

  Drawing = true;
  CurrentTexture = 0;
  CurrentMode = R2D_None;
  VertexBufferFill = 0;
  IndexBufferFill = 0;

  glDisable(GL_CULL_FACE);
}

void Renderer2D::DrawSprite(Sprite const& sprite, glm::vec2 topLeft,
                            glm::vec4 tint, glm::vec2 scale, float angle) {
  RectF scaledDest(topLeft.x, topLeft.y, scale.x * sprite.Bounds.Width,
                   scale.y * sprite.Bounds.Height);
  DrawSprite(sprite, scaledDest, tint, angle);
}

void Renderer2D::DrawRect(RectF const& dest, glm::vec4 color, float angle) {
  DrawSprite(RectSprite, dest, color, angle);
}

void Renderer2D::DrawSprite(Sprite const& sprite, RectF const& dest,
                            glm::vec4 tint, float angle) {
  if (!Drawing) {
    ImpLog(LL_Error, LC_Render,
           "Renderer2D::DrawSprite() called before Begin()\n");
    return;
  }

  // Do we have space for one more sprite quad?
  EnsureSpaceAvailable(4, sizeof(VertexBufferSprites), 6);

  // Are we in sprite mode?
  if (CurrentMode != R2D_Sprite) {
    ImpLogSlow(
        LL_Trace, LC_Render,
        "Renderer2D::DrawSprite flushing because mode %d is not R2D_Sprite\n",
        CurrentMode);
    Flush();
    glBindVertexArray(VAOSprites);
    glUseProgram(ShaderProgramSprite);
    CurrentMode = R2D_Sprite;
  }

  // Do we have the texture assigned?
  EnsureTextureBound(sprite.Sheet.Texture);

  // OK, all good, make quad

  RectF scaledDest = dest;
  scaledDest.Width *= sprite.BaseScale.x;
  scaledDest.Height *= sprite.BaseScale.y;

  VertexBufferSprites* vertices =
      (VertexBufferSprites*)(VertexBuffer + VertexBufferFill);
  VertexBufferFill += 4 * sizeof(VertexBufferSprites);

  IndexBufferFill += 6;

  QuadSetUV(sprite.Bounds, sprite.Sheet.DesignWidth, sprite.Sheet.DesignHeight,
            (uintptr_t)&vertices[0].UV, sizeof(VertexBufferSprites));
  QuadSetPosition(scaledDest, angle, (uintptr_t)&vertices[0].Position,
                  sizeof(VertexBufferSprites));

  for (int i = 0; i < 4; i++) vertices[i].Tint = tint;
}

inline void Renderer2D::QuadSetUV(RectF const& spriteBounds, float designWidth,
                                  float designHeight, uintptr_t uvs,
                                  int stride) {
  float topUV = (spriteBounds.Y / designHeight);
  float leftUV = (spriteBounds.X / designWidth);
  float bottomUV = ((spriteBounds.Y + spriteBounds.Height) / designHeight);
  float rightUV = ((spriteBounds.X + spriteBounds.Width) / designWidth);

  // bottom-left
  *(glm::vec2*)(uvs + 0 * stride) = glm::vec2(leftUV, bottomUV);
  // top-left
  *(glm::vec2*)(uvs + 1 * stride) = glm::vec2(leftUV, topUV);
  // top-right
  *(glm::vec2*)(uvs + 2 * stride) = glm::vec2(rightUV, topUV);
  // bottom-right
  *(glm::vec2*)(uvs + 3 * stride) = glm::vec2(rightUV, bottomUV);
}

inline void Renderer2D::QuadSetPosition(RectF const& transformedQuad,
                                        float angle, uintptr_t positions,
                                        int stride) {
  glm::vec2 bottomLeft =
      glm::vec2(transformedQuad.X, transformedQuad.Y + transformedQuad.Height);
  glm::vec2 topLeft = glm::vec2(transformedQuad.X, transformedQuad.Y);
  glm::vec2 topRight =
      glm::vec2(transformedQuad.X + transformedQuad.Width, transformedQuad.Y);
  glm::vec2 bottomRight = glm::vec2(transformedQuad.X + transformedQuad.Width,
                                    transformedQuad.Y + transformedQuad.Height);

  if (angle != 0.0f) {
    glm::vec2 center = transformedQuad.Center();
    glm::mat2 rot = Rotate2D(angle);

    bottomLeft = rot * (bottomLeft - center) + center;
    topLeft = rot * (topLeft - center) + center;
    topRight = rot * (topRight - center) + center;
    bottomRight = rot * (bottomRight - center) + center;
  }

  // bottom-left
  *(glm::vec2*)(positions + 0 * stride) = DesignToNDC(bottomLeft);
  // top-left
  *(glm::vec2*)(positions + 1 * stride) = DesignToNDC(topLeft);
  // top-right
  *(glm::vec2*)(positions + 2 * stride) = DesignToNDC(topRight);
  // bottom-right
  *(glm::vec2*)(positions + 3 * stride) = DesignToNDC(bottomRight);
}

void Renderer2D::Finish() {
  if (!Drawing) return;
  Flush();
  Drawing = false;
}

void Renderer2D::EnsureSpaceAvailable(int vertices, int vertexSize,
                                      int indices) {
  if (VertexBufferFill + vertices * vertexSize > VertexBufferSize ||
      IndexBufferFill + indices > IndexBufferCount) {
    ImpLogSlow(
        LL_Trace, LC_Render,
        "Renderer2D::EnsureSpaceAvailable flushing because buffers full at "
        "VertexBufferFill=%08X,IndexBufferFill=%08X\n",
        VertexBufferFill, IndexBufferFill);
    Flush();
  }
}

void Renderer2D::EnsureTextureBound(GLuint texture) {
  if (CurrentTexture != texture) {
    ImpLogSlow(LL_Trace, LC_Render,
               "Renderer2D::EnsureTextureBound flushing because texture %d is "
               "not %d\n",
               CurrentTexture, texture);
    Flush();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    CurrentTexture = texture;
  }
}

void Renderer2D::Flush() {
  if (!Drawing) {
    ImpLog(LL_Error, LC_Render, "Renderer2D::Flush() called before Begin()\n");
    return;
  }
  if (VertexBufferFill > 0 && IndexBufferFill > 0) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // TODO: better to specify the whole thing or just this?
    glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferFill, VertexBuffer);
    glDrawElements(GL_TRIANGLES, IndexBufferFill, GL_UNSIGNED_SHORT, 0);
  }
  IndexBufferFill = 0;
  VertexBufferFill = 0;
}

}  // namespace Impacto