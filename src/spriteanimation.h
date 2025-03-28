#pragma once

#include "spritesheet.h"
#include "animation.h"

namespace Impacto {

class SpriteAnimationDef;

struct SpriteAnimation : public Animation {
  SpriteAnimationDef* Def = 0;

  bool Exists();
  virtual Sprite CurrentSprite();
};

/*
  Sprite animation that has a separate in and out animation
  Consists of one contiguous Sprite array, with an int FixSpriteId
  denoting the 0-indexed id of the last frame of the in animation.

  If the animation is out and gets told to move in, it will play
  the in animation to completion.
  Similarly, if the animation is in and gets told to move out, it will
  play the out animation to completion.
  If either the in or out animation is playing, and it suddenly gets told
  to do the other animation, it will finish the currently playing animation
  and then immediately move on to the requested animation.

  The progress is split up such that the interval
  [0, 1 - FixSpriteId / FrameCount] is the out animation (with 0 being fully
  out), and the interval [1 - FixSpriteId / FrameCount, 1] is the in animation
  (with 1 being fully in).
*/
struct FixedSpriteAnimation : public SpriteAnimation {
  void StartInImpl(bool reset) override;
  void StartOutImpl(bool reset) override;
  void UpdateImpl(float dt) override;
  Sprite CurrentSprite() override;
  float GetFixedSpriteProgress() const;
};

class SpriteAnimationDef {
 public:
  float Duration;
  int FrameCount;
  Sprite* Frames;

  // Sprite to hold on in case of SpriteAnimFixed type
  int FixSpriteId = 0;

  SpriteAnimation Instantiate();
};

}  // namespace Impacto