#include "Rate.hpp"

using namespace geode::prelude;

namespace usl::rate {
    bool enabled() {
        return Mod::get()->getSettingValue<bool>("fake-rate");
    }

    bool hasRate(Level const& level) {
        return level.difficulty != Difficulty::None || level.quality != Quality::None;
    }

    int demonFace(Difficulty difficulty) {
        switch (difficulty) {
            case Difficulty::Easy: return 7;
            case Difficulty::Medium: return 8;
            case Difficulty::Insane: return 9;
            case Difficulty::Extreme: return 10;
            default: return 6;
        }
    }

    GJFeatureState featureState(Quality quality) {
        switch (quality) {
            case Quality::Featured: return GJFeatureState::Featured;
            case Quality::Epic: return GJFeatureState::Epic;
            case Quality::Legendary: return GJFeatureState::Legendary;
            case Quality::Mythic: return GJFeatureState::Mythic;
            default: return GJFeatureState::None;
        }
    }

    void apply(GJDifficultySprite* sprite, Level const& level, GJDifficultyName name) {
        if (!sprite || !hasRate(level)) return;
        // A rate without a difficulty keeps the game's face and only changes the glow.
        if (level.difficulty != Difficulty::None) {
            sprite->updateDifficultyFrame(demonFace(level.difficulty), name);
        }
        sprite->updateFeatureState(featureState(level.quality));
    }
}
