#pragma once

#include "../USL.hpp"
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/LoadingSpinner.hpp>

/// One USL list rendered with native GD level cells, 10 levels per page, like the game's level browser.
class USLListLayer : public cocos2d::CCLayer, public LevelManagerDelegate {
public:
    static USLListLayer* create(usl::ListType type);
    static cocos2d::CCScene* scene(usl::ListType type);

    void keyBackClicked() override;
    void keyDown(cocos2d::enumKeyCodes key, double timestamp) override;
    void onExit() override;
    void onEnter() override;

    void loadLevelsFinished(cocos2d::CCArray* levels, char const* key, int type) override;
    void loadLevelsFailed(char const* key, int type) override;
    void loadLevelsFinished(cocos2d::CCArray* levels, char const* key) override;
    void loadLevelsFailed(char const* key) override;
    void setupPageInfo(gd::string info, char const* key) override;

protected:
    bool init(usl::ListType type);
    void loadList();
    void applyFilter();
    void showPage(int page);
    void showLoading(bool loading);
    void showError(std::string const& text);
    void setListView(cocos2d::CCArray* levels);
    void cycleCategory();
    void onSearchTimer(float);
    void runSearch();
    int maxPage() const;
    bool isCurrentKey(char const* key) const;
    void onBack();

    usl::ListType m_type = usl::ListType::Classic;
    std::vector<usl::Level> m_filtered;
    std::string m_query;
    std::string m_category;
    int m_page = 1;
    int m_perPage = 10;
    std::string m_pendingKey;

    GJListLayer* m_list = nullptr;
    geode::TextInput* m_search = nullptr;
    geode::LoadingSpinner* m_spinner = nullptr;
    cocos2d::CCLabelBMFont* m_error = nullptr;
    cocos2d::CCLabelBMFont* m_pageInfo = nullptr;
    CCMenuItemSpriteExtra* m_prev = nullptr;
    CCMenuItemSpriteExtra* m_next = nullptr;
    CCMenuItemSpriteExtra* m_categoryBtn = nullptr;
    ButtonSprite* m_categorySpr = nullptr;
};
