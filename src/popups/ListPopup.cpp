#include "ListPopup.hpp"
#include "../UI.hpp"

using namespace geode::prelude;

bool USLListPopup::initList(usl::ListType type, std::string const& title, CCSize size) {
    if (!Popup::init(size.width, size.height)) return false;
    setTitle(title.c_str(), "goldFont.fnt", 0.75f, 18.f);

    auto listSize = CCSize { size.width - 30.f, size.height - 86.f };
    auto frame = usl::ui::panel(listSize + CCSize { 6.f, 6.f }, { 0, 0, 0 }, 90);
    frame->setPosition({ size.width / 2.f, 20.f + listSize.height / 2.f });
    frame->setID("list-frame"_spr);
    m_mainLayer->addChild(frame);

    m_scroll = ScrollLayer::create(listSize);
    m_scroll->setPosition({ size.width / 2.f - listSize.width / 2.f, 20.f });
    m_scroll->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout(2.f));
    m_scroll->setID("list"_spr);
    m_mainLayer->addChild(m_scroll, 2);

    m_spinner = LoadingSpinner::create(36.f);
    m_spinner->setPosition(frame->getPosition());
    m_mainLayer->addChild(m_spinner, 5);

    m_status = usl::ui::label("", "bigFont.fnt", 0.45f);
    m_status->setPosition(frame->getPosition());
    m_status->setVisible(false);
    m_mainLayer->addChild(m_status, 5);

    selectList(type);
    return true;
}

float USLListPopup::rowWidth() const {
    return m_scroll->getContentWidth();
}

void USLListPopup::selectList(usl::ListType type) {
    m_type = type;
    if (m_tabs) m_tabs->removeFromParent();
    m_tabs = usl::ui::listTabs(type, 0.72f, [this](usl::ListType next) { selectList(next); });
    m_tabs->setPosition({ m_size.width / 2.f, m_size.height - 46.f });
    m_mainLayer->addChild(m_tabs, 5);

    clearRows();
    m_status->setVisible(false);
    m_spinner->setVisible(true);
    load(type);
}

void USLListPopup::setStatus(std::string const& text) {
    m_spinner->setVisible(false);
    m_status->setString(text.c_str());
    m_status->limitLabelWidth(rowWidth() - 20.f, 0.45f, 0.2f);
    m_status->setVisible(!text.empty());
}

void USLListPopup::clearRows() {
    m_scroll->m_contentLayer->removeAllChildren();
    m_scroll->m_contentLayer->updateLayout();
}

CCNode* USLListPopup::rowBase(size_t index, float height) {
    auto row = CCNode::create();
    row->setContentSize({ rowWidth(), height });
    auto bg = usl::ui::panel({ rowWidth() - 4.f, height }, index % 2 ? ccColor3B { 40, 40, 60 } : ccColor3B { 60, 60, 90 }, 150);
    bg->setPosition({ rowWidth() / 2.f, height / 2.f });
    row->addChild(bg, -1);
    return row;
}

void USLListPopup::addRow(CCNode* row) {
    m_scroll->m_contentLayer->addChild(row);
}

void USLListPopup::finishRows() {
    m_spinner->setVisible(false);
    m_scroll->m_contentLayer->updateLayout();
    m_scroll->scrollToTop();
}
