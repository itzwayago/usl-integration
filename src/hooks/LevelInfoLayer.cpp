#include <Geode/modify/LevelInfoLayer.hpp>
#include "../UI.hpp"
#include "../Rate.hpp"
#include "../popups/LevelPopup.hpp"

using namespace geode::prelude;

class $modify(USLLevelInfoLayer, LevelInfoLayer) {
    static bool isOnline(GJGameLevel* level) {
        return level && level->m_levelType != GJLevelType::Main && level->m_levelType != GJLevelType::Editor;
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        if (!isOnline(level)) return true;

        int gdId = level->m_levelID.value();
        if (usl::Store::get().byGdId(gdId)) {
            applyUSL();
        } else {
            // Lists may still be loading right after startup.
            WeakRef<LevelInfoLayer> self = this;
            usl::Store::get().loadAllLevels([self](bool) {
                if (auto layer = self.lock()) static_cast<USLLevelInfoLayer*>(layer.data())->applyUSL();
            });
        }
        return true;
    }

    // The game redraws the face whenever the level data changes (download, update, rating).
    void updateLabelValues() {
        LevelInfoLayer::updateLabelValues();
        if (isOnline(m_level)) applyRate();
    }

    // Both may redraw the glow straight from the level data, so put the USL rate back after them.
    void levelDownloadFinished(GJGameLevel* level) {
        LevelInfoLayer::levelDownloadFinished(level);
        if (isOnline(m_level)) applyRate();
    }

    void levelUpdateFinished(GJGameLevel* level, UpdateResponse response) {
        LevelInfoLayer::levelUpdateFinished(level, response);
        if (isOnline(m_level)) applyRate();
    }

    void applyUSL() {
        applyRate();
        if (Mod::get()->getSettingValue<bool>("show-placements")) addUSLBadge();
    }

    void applyRate() {
        if (!usl::rate::enabled() || !m_level || !m_difficultySprite) return;
        if (auto entry = usl::Store::get().byGdId(m_level->m_levelID.value())) {
            usl::rate::apply(m_difficultySprite, *entry);
        }
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
        // Spam placements are per spam list, so say which one, short enough to fit the stat column.
        auto prefix = usl::isSpam(entry->list) ? fmt::format("{} ", usl::listAbbr(entry->list)) : std::string("Top ");
        auto plate = usl::ui::rankBadge(entry->list, entry->rank, 20.f, prefix);
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
        // Never run past the right edge, whatever the placement and points add up to.
        // Scaled before the button is made, so its touch area matches.
        float left = iconX - 11.f;
        float room = CCDirector::get()->getWinSize().width - 8.f - left;
        if (width > room) {
            badge->setScale(room / width);
            width = room;
        }

        auto level = *entry;
        auto btn = CCMenuItemExt::createSpriteExtra(badge, [level](auto) {
            USLLevelPopup::create(level)->show();
        });
        btn->setID("usl-badge"_spr);

        auto menu = CCMenu::create();
        menu->setID("usl-badge-menu"_spr);
        menu->setPosition({ 0.f, 0.f });
        btn->setPosition({ left + width / 2.f, rowY });
        menu->addChild(btn);
        addChild(menu, 5);
    }
};
