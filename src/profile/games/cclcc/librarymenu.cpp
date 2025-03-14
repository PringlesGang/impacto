#include "librarymenu.h"
#include "../../profile_internal.h"
#include "../../../ui/ui.h"
#include "../../../games/cclcc/librarymenu.h"
#include "../../../text.h"
#include "../../../game.h"

namespace Impacto {
namespace Profile {
namespace CCLCC {
namespace LibraryMenu {

void Configure() {
  FadeInDuration = EnsureGetMemberFloat("FadeInDuration");
  FadeOutDuration = EnsureGetMemberFloat("FadeOutDuration");

  LibraryBackgroundSprite = EnsureGetMemberSprite("LibraryBackgroundSprite");
  LibraryBackgroundPosition = EnsureGetMemberVec2("LibraryBackgroundPosition");
  LibraryIndexSprite = EnsureGetMemberSprite("LibraryIndexSprite");
  LibraryIndexPosition = EnsureGetMemberVec2("LibraryIndexPosition");
  LibraryButtonGuideSprite = EnsureGetMemberSprite("LibraryButtonGuideSprite");
  LibraryButtonGuidePosition =
      EnsureGetMemberVec2("LibraryButtonGuidePosition");
  LibraryMaskSprite = EnsureGetMemberSprite("LibraryMaskSprite");

  auto drawType = Game::DrawComponentType::_from_integral_unchecked(
      EnsureGetMemberInt("DrawType"));

  auto library = new UI::CCLCC::LibraryMenu();
  UI::Menus[drawType].push_back(library);
}

}  // namespace LibraryMenu
}  // namespace CCLCC
}  // namespace Profile
}  // namespace Impacto