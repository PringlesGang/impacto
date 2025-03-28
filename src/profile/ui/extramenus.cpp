#include "extramenus.h"
#include "../profile_internal.h"
#include "../../ui/ui.h"
#include "../games/mo6tw/clearlistmenu.h"
#include "../games/mo6tw/moviemenu.h"
#include "../games/mo6tw/actorsvoicemenu.h"
#include "../games/mo6tw/musicmenu.h"
#include "../games/mo6tw/albummenu.h"
#include "../games/cclcc/librarymenu.h"
#include "../games/cclcc/clearlistmenu.h"
#include "../games/chlcc/clearlistmenu.h"
#include "../games/chlcc/moviemenu.h"
#include "../games/chlcc/musicmenu.h"
#include "../games/chlcc/albummenu.h"

namespace Impacto {
namespace Profile {
namespace ExtraMenus {

using namespace Impacto::UI;

void Configure() {
  if (TryPushMember("ExtraMenus")) {
    AssertIs(LUA_TTABLE);

    if (TryPushMember("LibraryMenu")) {
      LibraryMenuType = LibraryMenuType::_from_integral_unchecked(
          EnsureGetMember<int>("Type"));

      if (LibraryMenuType == +LibraryMenuType::CCLCC) {
        CCLCC::LibraryMenu::Configure();
      }

      Pop();
    }

    if (TryPushMember("ClearListMenu")) {
      ClearListType = ClearListMenuType::_from_integral_unchecked(
          EnsureGetMember<int>("Type"));

      if (ClearListType == +ClearListMenuType::MO6TW) {
        MO6TW::ClearListMenu::Configure();
      } else if (ClearListType == +ClearListMenuType::CCLCC) {
        CCLCC::ClearListMenu::Configure();
      } else if (ClearListType == +ClearListMenuType::CHLCC) {
        CHLCC::ClearListMenu::Configure();
      }

      Pop();
    }

    if (TryPushMember("AlbumMenu")) {
      AlbumType =
          AlbumMenuType::_from_integral_unchecked(EnsureGetMember<int>("Type"));

      if (AlbumType == +AlbumMenuType::MO6TW) {
        MO6TW::AlbumMenu::Configure();
      } else if (AlbumType == +AlbumMenuType::CHLCC) {
        CHLCC::AlbumMenu::Configure();
      }

      Pop();
    }

    if (TryPushMember("MusicMenu")) {
      MusicType =
          MusicMenuType::_from_integral_unchecked(EnsureGetMember<int>("Type"));

      if (MusicType == +MusicMenuType::MO6TW) {
        MO6TW::MusicMenu::Configure();
      } else if (MusicType == +MusicMenuType::CHLCC) {
        CHLCC::MusicMenu::Configure();
      }

      Pop();
    }

    if (TryPushMember("MovieMenu")) {
      MovieType =
          MovieMenuType::_from_integral_unchecked(EnsureGetMember<int>("Type"));

      if (MovieType == +MovieMenuType::MO6TW) {
        MO6TW::MovieMenu::Configure();
      } else if (MovieType == +MovieMenuType::CHLCC) {
        CHLCC::MovieMenu::Configure();
      }

      Pop();
    }

    if (TryPushMember("ActorsVoiceMenu")) {
      ActorsVoiceType = ActorsVoiceMenuType::_from_integral_unchecked(
          EnsureGetMember<int>("Type"));

      if (ActorsVoiceType == +ActorsVoiceMenuType::MO6TW) {
        MO6TW::ActorsVoiceMenu::Configure();
      }

      Pop();
    }

    Pop();
  }
}

}  // namespace ExtraMenus
}  // namespace Profile
}  // namespace Impacto