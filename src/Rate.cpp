#include "Rate.hpp"

using namespace geode::prelude;

namespace usl::rate {
    namespace {
        // Only demon faces (6 to 10) have a long variant with "Extreme Demon" written under them,
        // and it is a good 15% taller. Keep it only where the game already showed one; any other
        // face becomes the short "Demon" one, which is about the size of the face it replaces.
        GJDifficultyName nameOf(GJDifficultySprite* sprite) {
            for (int difficulty = 6; difficulty <= 10; difficulty++) {
                std::string longFrame = GJDifficultySprite::getDifficultyFrame(difficulty, GJDifficultyName::Long);
                if (isSpriteFrameName(sprite, longFrame.c_str())) return GJDifficultyName::Long;
            }
            return GJDifficultyName::Short;
        }
    }

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

    void apply(GJDifficultySprite* sprite, Level const& level) {
        if (!sprite || !hasRate(level)) return;
        // A rate without a difficulty keeps the game's face and only changes the glow.
        if (level.difficulty != Difficulty::None) {
            sprite->updateDifficultyFrame(demonFace(level.difficulty), nameOf(sprite));
        }
        sprite->updateFeatureState(featureState(level.quality));
    }
}
