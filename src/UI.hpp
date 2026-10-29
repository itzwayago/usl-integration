#pragma once

#include "USL.hpp"
#include <Geode/ui/NineSlice.hpp>

namespace usl::ui {
    /// Mod sprite drawn with mipmaps, so a big source stays clean at icon size.
    cocos2d::CCSprite* crispSprite(const char* file);
    cocos2d::CCSprite* iconSprite(const char* file, float size);
    cocos2d::CCSprite* listIcon(ListType type, float size);
    cocos2d::CCLabelBMFont* label(std::string const& text, const char* font, float scale, cocos2d::ccColor3B color = { 255, 255, 255 });
    geode::NineSlice* panel(cocos2d::CCSize size, cocos2d::ccColor3B color, GLubyte opacity);

    /// Dark plate with a list-coloured rim: list icon + white placement number. `height` in GD units.
    cocos2d::CCNode* rankBadge(ListType type, int rank, float height, std::string const& prefix = "");

    /// GD demon face with the rate glow for a level USL rated, or nullptr when it has neither.
    cocos2d::CCNode* rateFace(Level const& level, float height);

    /// Full-screen GD gradient background tinted with the given colour, plus corner art.
    void addBackground(cocos2d::CCLayer* layer, cocos2d::ccColor3B color);
    CCMenuItemSpriteExtra* backButton(geode::Function<void(CCMenuItemSpriteExtra*)> callback);

    /// One tab per USL list (Classic, Challenge and the three spam lists), sized to fit `width`.
    cocos2d::CCMenu* listTabs(ListType selected, float width, std::function<void(ListType)> onSelect);

    void openSite(std::string const& path);
}
