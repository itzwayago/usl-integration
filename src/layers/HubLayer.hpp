#pragma once

#include "../USL.hpp"

/// Entry screen in the style of the game's own menus: one button per USL list, players, changes and the site.
class USLHubLayer : public cocos2d::CCLayer {
public:
    static USLHubLayer* create();
    static cocos2d::CCScene* scene();

    void keyBackClicked() override;

protected:
    bool init() override;
    cocos2d::CCMenu* listRow(std::initializer_list<usl::ListType> types, float buttonWidth, float buttonHeight, float textScale);
    void refreshStats();
    void refreshLevels();
    void openList(usl::ListType type);
    void onBack();

    std::array<cocos2d::CCLabelBMFont*, usl::LIST_COUNT> m_countLabels {};
    cocos2d::CCLabelBMFont* m_statsLabel = nullptr;
};
