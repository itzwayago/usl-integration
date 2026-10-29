#pragma once

#include "../USL.hpp"

/// Entry screen: the three USL lists, players, recent changes and site stats.
class USLHubLayer : public cocos2d::CCLayer {
public:
    static USLHubLayer* create();
    static cocos2d::CCScene* scene();

    void keyBackClicked() override;

protected:
    bool init() override;
    void refreshStats();
    void onBack();

    std::array<cocos2d::CCLabelBMFont*, 3> m_levelLabels {};
    std::array<cocos2d::CCLabelBMFont*, 3> m_recordLabels {};
    cocos2d::CCLabelBMFont* m_statsLabel = nullptr;
};
