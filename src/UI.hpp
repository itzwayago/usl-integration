#pragma once

#include "USL.hpp"
#include <Geode/ui/NineSlice.hpp>

namespace usl::ui {
    cocos2d::CCSprite* listIcon(ListType type, float size);
    cocos2d::CCSprite* iconSprite(const char* file, float size);
    cocos2d::CCLabelBMFont* label(std::string const& text, const char* font, float scale, cocos2d::ccColor3B color = { 255, 255, 255 });
    geode::NineSlice* panel(cocos2d::CCSize size, cocos2d::ccColor3B color, GLubyte opacity);

    /// Dark plate with a list-coloured rim: list icon + white placement number. `height` in GD units.
    cocos2d::CCNode* rankBadge(ListType type, int rank, float height, std::string const& prefix = "");

    /// Full-screen GD gradient background tinted with the given colour, plus corner art.
    void addBackground(cocos2d::CCLayer* layer, cocos2d::ccColor3B color);
    CCMenuItemSpriteExtra* backButton(geode::Function<void(CCMenuItemSpriteExtra*)> callback);

    /// Row of three list tabs (Classic / Challenge / Spam). Returns the menu; `onSelect` fires on change.
    cocos2d::CCMenu* listTabs(ListType selected, float scale, std::function<void(ListType)> onSelect);

    void openSite(std::string const& path);
    std::string trimText(std::string const& text, size_t maxChars);
}
