#pragma once

#include "../USL.hpp"
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/LoadingSpinner.hpp>

/// USL card for one level: placement, category, points and the record list.
class USLLevelPopup : public geode::Popup {
public:
    static USLLevelPopup* create(usl::Level const& level);

protected:
    bool init(usl::Level const& level);
    void buildRecords(std::vector<usl::Record> const& records);

    usl::Level m_level;
    geode::ScrollLayer* m_scroll = nullptr;
    geode::LoadingSpinner* m_spinner = nullptr;
    cocos2d::CCLabelBMFont* m_recordsTitle = nullptr;
};
