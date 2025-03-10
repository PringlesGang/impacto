#include "background2d.h"

#include "mask2d.h"
#include "mem.h"
#include "io/memorystream.h"
#include "io/vfs.h"
#include "util.h"
#include "profile/game.h"
#include "profile/scriptvars.h"
#include "profile/vm.h"
// #include "window.h"
#include "renderer/renderer.h"

namespace Impacto {

using namespace Impacto::Profile::ScriptVars;
using namespace Impacto::Profile::Vm;

void Background2D::Init() {
  for (int i = 0; i < MaxBackgrounds2D; i++) {
    Backgrounds2D[i] = &Backgrounds[i];
  }

  for (int i = 0; i < MaxScreencaptures; i++) {
    Screencaptures[i].BgSprite.Sheet.IsScreenCap = true;
    Screencaptures[i].LoadSolidColor(0xFF000000, Window->WindowWidth,
                                     Window->WindowHeight);
    Screencaptures[i].Status = LS_Loaded;
  }

  ShaderScreencapture.BgSprite.Sheet.IsScreenCap = true;
  ShaderScreencapture.LoadSolidColor(0xFF000000, Window->WindowWidth,
                                     Window->WindowHeight);
  ShaderScreencapture.Status = LS_Loaded;
}

bool Background2D::LoadSync(uint32_t bgId) {
  if (bgId & 0xFF000000) {
    BgTexture.Load1x1((bgId >> 16) & 0xFF, (bgId >> 8) & 0xFF, bgId & 0xFF,
                      0xFF);
  } else {
    Io::Stream* stream;
    int64_t err = Io::VfsOpen("bg", bgId, &stream);
    if (err != IoError_OK) return false;
    BgTexture.Load(stream);
    delete stream;
  }
  return true;
}

void Background2D::LoadSolidColor(uint32_t color, int width, int height) {
  BgTexture.LoadSolidColor(width, height, color);
  MainThreadOnLoad();
}

void Background2D::UnloadSync() {
  Renderer->FreeTexture(BgSprite.Sheet.Texture);
  BgSprite.Sheet.DesignHeight = 0.0f;
  BgSprite.Sheet.DesignWidth = 0.0f;
  BgSprite.Sheet.Texture = 0;
  BgSprite.Sheet.IsScreenCap = false;
  Show = false;
  Layer = -1;
}

void Background2D::MainThreadOnLoad() {
  BgSprite.Sheet.Texture = BgTexture.Submit();
  if ((BgTexture.Width == 1) && (BgTexture.Height == 1)) {
    BgSprite.Sheet.DesignWidth = Profile::DesignWidth;
    BgSprite.Sheet.DesignHeight = Profile::DesignHeight;
  } else {
    BgSprite.Sheet.DesignWidth = (float)BgTexture.Width;
    BgSprite.Sheet.DesignHeight = (float)BgTexture.Height;
  }
  BgSprite.BaseScale = glm::vec2(1.0f);
  BgSprite.Bounds = RectF(0.0f, 0.0f, BgSprite.Sheet.DesignWidth,
                          BgSprite.Sheet.DesignHeight);
  Show = false;
  Layer = -1;
}

void Background2D::Render(int bgId, int layer) {
  if (Status == LS_Loaded && Layer == layer && Show) {
    glm::vec4 col = glm::vec4(1.0f);
    if (GameInstructionSet == +Vm::InstructionSet::Dash) {
      col.a = ScrWork[SW_BG1ALPHA + ScrWorkBgStructSize * bgId] / 256.0f;
    } else if (GameInstructionSet == +Vm::InstructionSet::CC) {
      if (ScrWork[SW_BGEFF_MODE + 30 * bgId] == 1)
        col.a = ScrWork[SW_BGEFF_ALPHA + 30 * bgId] / 256.0f;
    }
    int renderType = ScrWork[SW_BG1FADETYPE + ScrWorkBgStructSize * bgId];
    BackgroundRenderTable[renderType](this, bgId, col);
  }
}

BackgroundRenderer(RenderRegular) {
  Renderer->DrawSprite(
      bg->BgSprite,
      RectF(bg->DisplayCoords.x, bg->DisplayCoords.y,
            bg->BgSprite.ScaledWidth(), bg->BgSprite.ScaledHeight()),
      col, 0.0f, false);
  for (int i = 0; i < MaxLinkedBgBuffers; i++) {
    if (bg->Links[i].Direction != LD_Off && bg->Links[i].LinkedBuffer != NULL) {
      Renderer->DrawSprite(
          bg->Links[i].LinkedBuffer->BgSprite,
          RectF(bg->Links[i].DisplayCoords.x, bg->Links[i].DisplayCoords.y,
                bg->Links[i].LinkedBuffer->BgSprite.ScaledWidth(),
                bg->Links[i].LinkedBuffer->BgSprite.ScaledHeight()),
          col, 0.0f, false);
    }
  }
}

BackgroundRenderer(RenderMasked) {
  Renderer->DrawMaskedSprite(
      bg->BgSprite,
      Masks2D[ScrWork[SW_BG1MASKNO + ScrWorkBgStructSize * bgId]].MaskSprite,
      RectF(bg->DisplayCoords.x, bg->DisplayCoords.y,
            bg->BgSprite.ScaledWidth(), bg->BgSprite.ScaledHeight()),
      col, ScrWork[SW_BG1FADECT + ScrWorkBgStructSize * bgId],
      ScrWork[SW_BG1MASKFADERANGE + ScrWorkBgStructSize * bgId]);
}

BackgroundRenderer(RenderMaskedInverted) {
  Renderer->DrawMaskedSprite(
      bg->BgSprite,
      Masks2D[ScrWork[SW_BG1MASKNO + ScrWorkBgStructSize * bgId]].MaskSprite,
      RectF(bg->DisplayCoords.x, bg->DisplayCoords.y,
            bg->BgSprite.ScaledWidth(), bg->BgSprite.ScaledHeight()),
      col, ScrWork[SW_BG1FADECT + ScrWorkBgStructSize * bgId],
      ScrWork[SW_BG1MASKFADERANGE + ScrWorkBgStructSize * bgId], true);
}

BackgroundRenderer(RenderFade) {
  col.a = ((ScrWork[SW_BG1FADECT + ScrWorkBgStructSize * bgId] *
            (ScrWork[SW_BG1ALPHA + ScrWorkBgStructSize * bgId] +
             ScrWork[SW_BG1ALPHA_OFS + 10 * bgId])) >>
           8) /
          256.0f;
  Renderer->DrawSprite(
      bg->BgSprite,
      RectF(bg->DisplayCoords.x, bg->DisplayCoords.y,
            bg->BgSprite.ScaledWidth(), bg->BgSprite.ScaledHeight()),
      col, 0.0f, false);
  for (int i = 0; i < MaxLinkedBgBuffers; i++) {
    if (bg->Links[i].Direction != LD_Off && bg->Links[i].LinkedBuffer != NULL) {
      Renderer->DrawSprite(
          bg->Links[i].LinkedBuffer->BgSprite,
          RectF(bg->Links[i].DisplayCoords.x, bg->Links[i].DisplayCoords.y,
                bg->Links[i].LinkedBuffer->BgSprite.ScaledWidth(),
                bg->Links[i].LinkedBuffer->BgSprite.ScaledHeight()),
          col, 0.0f, false);
    }
  }
}

}  // namespace Impacto