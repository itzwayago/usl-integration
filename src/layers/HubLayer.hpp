#pragma once

#include "../USL.hpp"

/// Entry screen: Classic, Challenge and the three spam lists, players, recent changes and site stats.
class USLHubLayer : public cocos2d::CCLayer {
public:
    static USLHubLayer* create();
    static cocos2d::CCScene* scene();

    void keyBackClicked() override;

protected:
    bool init() override;
    cocos2d::CCNode* buildListTile(usl::ListType type, cocos2d::CCSize size);
    cocos2d::CCNode* buildSpamCard(cocos2d::CCSize size);
    void refreshStats();
    void refreshLevels();
    void refreshLatestChange();
    void openList(usl::ListType type);
    void onBack();

    std::array<cocos2d::CCLabelBMFont*, usl::LIST_COUNT> m_levelLabels {};
    std::array<cocos2d::CCLabelBMFont*, usl::LIST_COUNT> m_topLabels {};
    cocos2d::CCLabelBMFont* m_classicRecords = nullptr;
    cocos2d::CCLabelBMFont* m_challengeRecords = nullptr;
    cocos2d::CCLabelBMFont* m_spamRecords = nullptr;
    cocos2d::CCLabelBMFont* m_statsLabel = nullptr;
    cocos2d::CCLabelBMFont* m_latestLabel = nullptr;
};
