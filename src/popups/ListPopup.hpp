#pragma once

#include "../USL.hpp"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/LoadingSpinner.hpp>

/// Base for popups that show one scrolling list per USL list type, with tabs on top.
class USLListPopup : public geode::Popup {
protected:
    bool initList(usl::ListType type, std::string const& title, cocos2d::CCSize size);
    void selectList(usl::ListType type);
    void setStatus(std::string const& text);
    void clearRows();
    void addRow(cocos2d::CCNode* row);
    void finishRows();
    cocos2d::CCNode* rowBase(size_t index, float height = 26.f);
    float rowWidth() const;

    virtual void load(usl::ListType type) = 0;

    usl::ListType m_type = usl::ListType::Classic;
    cocos2d::CCMenu* m_tabs = nullptr;
    geode::ScrollLayer* m_scroll = nullptr;
    geode::LoadingSpinner* m_spinner = nullptr;
    cocos2d::CCLabelBMFont* m_status = nullptr;
};
