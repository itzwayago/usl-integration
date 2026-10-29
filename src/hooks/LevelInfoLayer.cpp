#include <Geode/modify/LevelInfoLayer.hpp>
#include "../UI.hpp"
#include "../popups/LevelPopup.hpp"

using namespace geode::prelude;

class $modify(USLLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        if (!level || level->m_levelType == GJLevelType::Main || level->m_levelType == GJLevelType::Editor) return true;
        if (!Mod::get()->getSettingValue<bool>("show-placements")) return true;

        int gdId = level->m_levelID.value();
        if (usl::Store::get().byGdId(gdId)) {
            addUSLBadge();
        } else {
            // Lists may still be loading right after startup.
            WeakRef<LevelInfoLayer> self = this;
            usl::Store::get().loadAllLevels([self, gdId](bool) {
                if (auto layer = self.lock(); layer && usl::Store::get().byGdId(gdId)) {
                    static_cast<USLLevelInfoLayer*>(layer.data())->addUSLBadge();
                }
            });
        }
        return true;
    }

    void addUSLBadge() {
        if (getChildByID("usl-badge-menu"_spr) || !m_level) return;
        auto entry = usl::Store::get().byGdId(m_level->m_levelID.value());
        if (!entry) return;

        auto downloadsIcon = getChildByID("downloads-icon");
        auto lengthIcon = getChildByID("length-icon");
        // Orbs only exist on rated levels, so they are optional (many Spam/Challenge levels are unrated).
        if (!downloadsIcon || !m_downloadsLabel || !lengthIcon || !m_lengthLabel || !m_likesIcon || !m_likesLabel) {
            log::warn("USL: level page stat nodes missing, placement not shown");
            return;
        }
        bool hasOrbs = m_orbsIcon && m_orbsLabel && m_orbsIcon->isVisible();

        // Make room under the stat column (downloads / likes / length / orbs) for one more row.
        auto lift = [](CCNode* node, float dy) { if (node) node->setPositionY(node->getPositionY() + dy); };
        lift(downloadsIcon, 10.f);
        lift(m_downloadsLabel, 10.f);
        lift(m_likesIcon, 12.f);
        lift(m_likesLabel, 12.f);
        lift(lengthIcon, 14.f);
        lift(m_lengthLabel, 14.f);
        lift(m_exactLengthLabel, 14.f);
        lift(m_orbsIcon, 16.f);
        lift(m_orbsLabel, 16.f);

        float step = downloadsIcon->getPositionY() - m_likesIcon->getPositionY();
        float rowY = (hasOrbs ? m_orbsIcon->getPositionY() : lengthIcon->getPositionY()) - step;
        float iconX = lengthIcon->getPositionX();

        auto badge = CCNode::create();
        auto plate = usl::ui::rankBadge(entry->list, entry->rank, 20.f, "Top ");
        plate->setAnchorPoint({ 0.f, 0.f });
        plate->setPosition({ 0.f, 1.f });
        badge->addChild(plate);
        float width = plate->getContentWidth();

        if (Mod::get()->getSettingValue<bool>("show-points")) {
            auto pts = usl::ui::label(fmt::format("{} pts", usl::formatPoints(entry->points)), "goldFont.fnt", 0.45f);
            pts->setAnchorPoint({ 0.f, 0.5f });
            pts->setPosition({ width + 5.f, 11.f });
            badge->addChild(pts);
            width = pts->getPositionX() + pts->getScaledContentWidth();
        }
        badge->setContentSize({ width, 22.f });

        auto level = *entry;
        auto btn = CCMenuItemExt::createSpriteExtra(badge, [level](auto) {
            USLLevelPopup::create(level)->show();
        });
        btn->setID("usl-badge"_spr);

        auto menu = CCMenu::create();
        menu->setID("usl-badge-menu"_spr);
        menu->setPosition({ 0.f, 0.f });
        btn->setPosition({ iconX - 11.f + width / 2.f, rowY });
        menu->addChild(btn);
        addChild(menu, 5);
    }
};
