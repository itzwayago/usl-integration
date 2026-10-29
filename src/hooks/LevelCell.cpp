#include <Geode/modify/LevelCell.hpp>
#include "../UI.hpp"
#include "../Rate.hpp"

using namespace geode::prelude;

class $modify(USLLevelCell, LevelCell) {
    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);
        if (!level || level->m_levelType == GJLevelType::Main || level->m_levelType == GJLevelType::Editor) return;

        auto entry = usl::Store::get().byGdId(level->m_levelID.value());
        if (!entry) return;

        if (usl::rate::enabled()) {
            auto face = typeinfo_cast<GJDifficultySprite*>(m_mainLayer->getChildByIDRecursive("difficulty-sprite"));
            usl::rate::apply(face, *entry, GJDifficultyName::Short);
        }

        if (!usl::settings::placements() || m_mainLayer->getChildByID("usl-rank"_spr)) return;
        if (m_compactView) addStatBadge(*entry);
        else addCornerBadge(*entry);
    }

    // Normal cells: placement plate in the free top-left corner above the difficulty face.
    void addCornerBadge(usl::Level const& entry) {
        auto badge = usl::ui::rankBadge(entry.list, entry.rank, 15.f);
        badge->setID("usl-rank"_spr);
        badge->setAnchorPoint({ 0.5f, 1.f });
        // Centre the plate over the difficulty face (node-ids: difficulty-container).
        float faceX = 26.f;
        if (auto face = m_mainLayer->getChildByID("difficulty-container")) {
            faceX = face->getPositionX() + face->getScaledContentWidth() * (0.5f - face->getAnchorPoint().x);
        }
        faceX = std::max(faceX, badge->getContentWidth() / 2.f + 2.f);
        badge->setPosition({ faceX, 88.f });
        m_mainLayer->addChild(badge, 10);
    }

    // Compact cells have no free corner, so the placement joins the stat row.
    void addStatBadge(usl::Level const& entry) {
        auto likesLabel = m_mainLayer->getChildByID("likes-label");
        if (!likesLabel) return;
        auto orbsLabel = m_mainLayer->getChildByID("orbs-label");
        CCNode* last = orbsLabel && orbsLabel->isVisible() ? orbsLabel : likesLabel;

        auto badge = usl::ui::rankBadge(entry.list, entry.rank, 11.f);
        badge->setID("usl-rank"_spr);
        badge->setAnchorPoint({ 0.f, 0.5f });
        float x = last->getPositionX() + last->getScaledContentWidth() + 8.f;
        badge->setPosition({ std::min(x, 300.f - badge->getContentWidth()), likesLabel->getPositionY() });
        m_mainLayer->addChild(badge, 10);
    }
};
