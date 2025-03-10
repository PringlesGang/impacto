#include "librarymenu.h"

#include "clearlistmenu.h"

#include "../../profile/games/cclcc/librarymenu.h"
#include "../../profile/ui/backlogmenu.h"
#include "../../profile/dialogue.h"
#include "../../renderer/renderer.h"
#include "../../mem.h"
#include "../../profile/scriptvars.h"
#include "../../vm/interface/input.h"
#include "../../profile/game.h"

namespace Impacto {
namespace UI {
namespace CCLCC {

using namespace Impacto::Profile::CCLCC::LibraryMenu;
using namespace Impacto::Profile::ScriptVars;

using namespace Impacto::Vm::Interface;

using namespace Impacto::UI::Widgets;

void LibraryMenu::LibraryMenuButtonOnClick(Widgets::Button* target) {
  CurrentLibraryMenu = target->Id;
}

LibraryMenu::LibraryMenu() {
  MainItems = new Widgets::Group(this);

  auto libraryMenuOnClick = [&](Widgets::Button* target) {
    Audio::Channels[Audio::AC_SSE]->Play("sysse", 2, false, 0);
    LibraryMenuButtonOnClick(target);
  };

  FadeAnimation.Direction = AnimationDirection::In;
  FadeAnimation.LoopMode = AnimationLoopMode::Stop;
  FadeAnimation.DurationIn = FadeInDuration;
  FadeAnimation.DurationOut = FadeOutDuration;
}

void LibraryMenu::Show() {
  if (State != Shown) {
    State = Showing;
    FadeAnimation.StartIn();
    if (UI::FocusedMenu != 0) {
      LastFocusedMenu = UI::FocusedMenu;
      LastFocusedMenu->IsFocused = false;
    }
    IsFocused = true;
    UI::FocusedMenu = this;
  }
}

void LibraryMenu::Hide() {
  if (State != Hidden) {
    State = Hiding;
    FadeAnimation.StartOut();
    if (LastFocusedMenu != 0) {
      UI::FocusedMenu = LastFocusedMenu;
      LastFocusedMenu->IsFocused = true;
    } else {
      UI::FocusedMenu = 0;
    }
    IsFocused = false;
  }
}

void LibraryMenu::Update(float dt) {
  if (ScrWork[SW_SYSSUBMENUCT] < 32 && State == Shown &&
      (ScrWork[SW_SYSSUBMENUNO] == 8)) {
    Hide();
  } else if (ScrWork[SW_SYSSUBMENUCT] >= 32 && State == Hidden &&
             (ScrWork[SW_SYSSUBMENUNO] == 8)) {
    Show();
  }
  if (State == Shown && ScrWork[SW_SYSSUBMENUNO] == 8) {
    UpdateInput();
  }
  FadeAnimation.Update(dt);
  if (State == Showing && FadeAnimation.Progress == 1.0f) {
    State = Shown;
  } else if (State == Hiding && FadeAnimation.Progress == 0.0f) {
    State = Hidden;
  }
}

void LibraryMenu::Render() {
  if (State != Hidden && ScrWork[SW_SYSSUBMENUCT] >= 32 &&
      ScrWork[SW_SYSSUBMENUNO] == 8) {
    glm::vec4 col(1.0f, 1.0f, 1.0f, FadeAnimation.Progress);
    glm::vec4 maskTint = glm::vec4(1.0f);
    if (CurrentLibraryMenu != LibraryMenuPageType::Sound) {
      Renderer->DrawSprite(LibraryBackgroundSprite, LibraryBackgroundPosition,
                           col);
    }
    Renderer->DrawSprite(LibraryIndexSprite, LibraryIndexPosition, col);
    Renderer->DrawSprite(
        LibraryMaskSprite,
        RectF(0.0f, 0.0f, Profile::DesignWidth, Profile::DesignHeight),
        maskTint);
    Renderer->DrawSprite(LibraryButtonGuideSprite, LibraryButtonGuidePosition,
                         col);
  }
}

}  // namespace CCLCC
}  // namespace UI
}  // namespace Impacto