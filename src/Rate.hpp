#pragma once

#include "USL.hpp"

/// Fake rate: GD difficulty faces and rate glows follow what USL set for the level.
namespace usl::rate {
    bool enabled();
    /// Whether USL set anything for this level that should replace the game's own rating.
    bool hasRate(Level const& level);
    /// GD difficulty value of a demon face (6 hard, 7 easy, 8 medium, 9 insane, 10 extreme).
    int demonFace(Difficulty difficulty);
    GJFeatureState featureState(Quality quality);
    /// Swaps the face and glow of a game difficulty sprite. `name` is the variant the game uses there:
    /// level pages draw the long one ("Extreme Demon"), level cells the short one ("Demon").
    void apply(GJDifficultySprite* sprite, Level const& level, GJDifficultyName name);
}
