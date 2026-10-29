#include "LevelPopup.hpp"
#include "../UI.hpp"
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

namespace {
    constexpr float W = 390.f;
    constexpr float H = 270.f;

    CCNode* chip(CCNode* icon, std::string const& text, ccColor3B textColor, ccColor3B bgColor) {
        auto node = CCNode::create();
        auto lbl = usl::ui::label(text, "bigFont.fnt", 0.38f, textColor);
        float iconW = icon ? 16.f : 0.f;
        float width = lbl->getScaledContentWidth() + iconW + 14.f;
        node->setContentSize({ width, 20.f });
        auto bg = usl::ui::panel({ width, 20.f }, bgColor, 210);
        bg->setPosition({ width / 2.f, 10.f });
        node->addChild(bg);
        if (icon) {
            icon->setPosition({ 7.f + iconW / 2.f, 10.f });
            node->addChild(icon);
        }
        lbl->setAnchorPoint({ 0.f, 0.5f });
        lbl->setPosition({ 7.f + iconW + (icon ? 2.f : 0.f), 10.5f });
        node->addChild(lbl);
        return node;
    }
}

USLLevelPopup* USLLevelPopup::create(usl::Level const& level) {
    auto ret = new USLLevelPopup();
    if (ret->init(level)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool USLLevelPopup::init(usl::Level const& level) {
    if (!Popup::init(W, H)) return false;
    setID("USLLevelPopup");
    m_level = level;
    setTitle(level.name.c_str(), "bigFont.fnt", 0.6f, 20.f);
    if (m_title) m_title->limitLabelWidth(W - 70.f, 0.6f, 0.25f);

    auto credits = level.verifier.empty()
        ? fmt::format("by {}", level.creator)
        : fmt::format("by {}  -  verified by {}", level.creator, level.verifier);
    auto creditsLbl = usl::ui::label(credits, "goldFont.fnt", 0.5f);
    creditsLbl->limitLabelWidth(W - 40.f, 0.5f, 0.2f);
    creditsLbl->setPosition({ W / 2.f, H - 42.f });
    m_mainLayer->addChild(creditsLbl);

    // Chips: placement, category, points, list percent.
    auto chips = CCNode::create();
    chips->setID("chips"_spr);
    chips->addChild(usl::ui::rankBadge(level.list, level.rank, 20.f, fmt::format("{} Top ", usl::listShortName(level.list))));
    chips->addChild(chip(nullptr, usl::categoryName(level.category), usl::categoryColor(level.category), { 30, 30, 45 }));
    chips->addChild(chip(nullptr, fmt::format("{} pts", usl::formatPoints(level.points)), { 120, 255, 140 }, { 30, 30, 45 }));
    chips->addChild(chip(nullptr, fmt::format("List {}%", level.listPercent), { 140, 210, 255 }, { 30, 30, 45 }));
    chips->setContentSize({ W - 30.f, 20.f });
    chips->setAnchorPoint({ 0.5f, 0.5f });
    chips->setLayout(RowLayout::create()->setGap(6.f)->setAutoScale(true));
    chips->setPosition({ W / 2.f, H - 66.f });
    m_mainLayer->addChild(chips);

    m_recordsTitle = usl::ui::label("Records", "goldFont.fnt", 0.55f);
    m_recordsTitle->setAnchorPoint({ 0.f, 0.5f });
    m_recordsTitle->setPosition({ 18.f, H - 90.f });
    m_mainLayer->addChild(m_recordsTitle);

    auto listSize = CCSize { W - 30.f, H - 146.f };
    auto frame = usl::ui::panel(listSize + CCSize { 6.f, 6.f }, { 0, 0, 0 }, 90);
    frame->setPosition({ W / 2.f, 46.f + listSize.height / 2.f });
    m_mainLayer->addChild(frame);

    m_scroll = ScrollLayer::create(listSize);
    m_scroll->setPosition({ W / 2.f - listSize.width / 2.f, 46.f });
    m_scroll->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout(2.f));
    m_mainLayer->addChild(m_scroll, 2);

    m_spinner = LoadingSpinner::create(30.f);
    m_spinner->setPosition(frame->getPosition());
    m_mainLayer->addChild(m_spinner, 5);

    // Bottom buttons.
    auto buttons = CCMenu::create();
    buttons->setContentSize({ W - 40.f, 30.f });
    buttons->setPosition({ W / 2.f, 24.f });
    buttons->setID("bottom-menu"_spr);

    if (!level.youtubeUrl.empty()) {
        auto spr = ButtonSprite::create("Video", 80, true, "bigFont.fnt", "GJ_button_06.png", 28.f, 0.55f);
        spr->setScale(0.8f);
        auto url = level.youtubeUrl;
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [url](auto) { web::openLinkInBrowser(url); });
        btn->setID("video-button");
        buttons->addChild(btn);
    }
    {
        auto spr = ButtonSprite::create("Open on site", 120, true, "bigFont.fnt", "GJ_button_01.png", 28.f, 0.55f);
        spr->setScale(0.8f);
        auto slug = level.slug;
        auto btn = CCMenuItemExt::createSpriteExtra(spr, [slug](auto) { usl::ui::openSite(fmt::format("/list/{}", slug)); });
        btn->setID("site-button");
        buttons->addChild(btn);
    }
    buttons->setLayout(RowLayout::create()->setGap(10.f)->setAutoScale(false));
    m_mainLayer->addChild(buttons, 3);

    WeakRef<USLLevelPopup> self = this;
    usl::Store::get().loadLevel(level.slug, [self](std::optional<usl::LevelDetails> details) {
        auto popup = self.lock();
        if (!popup) return;
        popup->m_spinner->setVisible(false);
        if (!details) {
            popup->m_recordsTitle->setString("Records - couldn't load");
            return;
        }
        popup->buildRecords(details->records);
    });
    return true;
}

void USLLevelPopup::buildRecords(std::vector<usl::Record> const& records) {
    m_recordsTitle->setString(fmt::format("Records ({})", records.size()).c_str());
    auto content = m_scroll->m_contentLayer;
    float w = m_scroll->getContentWidth();

    if (records.empty()) {
        auto none = usl::ui::label("No records yet - be the first!", "bigFont.fnt", 0.4f);
        none->setPosition({ w / 2.f, m_scroll->getContentHeight() / 2.f });
        m_scroll->addChild(none, 5);
        return;
    }

    for (size_t i = 0; i < records.size(); i++) {
        auto const& r = records[i];
        auto row = CCNode::create();
        row->setContentSize({ w, 22.f });
        bool full = r.progress >= 100;
        auto bg = usl::ui::panel({ w - 4.f, 22.f }, i % 2 ? ccColor3B { 40, 40, 60 } : ccColor3B { 60, 60, 90 }, 150);
        bg->setPosition({ w / 2.f, 11.f });
        row->addChild(bg, -1);

        auto idx = usl::ui::label(fmt::format("{}.", i + 1), "goldFont.fnt", 0.42f);
        idx->setAnchorPoint({ 0.f, 0.5f });
        idx->setPosition({ 8.f, 11.f });
        row->addChild(idx);

        auto name = usl::ui::label(r.username, "bigFont.fnt", 0.38f);
        name->setAnchorPoint({ 0.f, 0.5f });
        name->setPosition({ 40.f, 11.f });
        name->limitLabelWidth(150.f, 0.38f, 0.2f);
        row->addChild(name);

        if (!r.countryCode.empty()) {
            auto cc = usl::ui::label(r.countryCode, "chatFont.fnt", 0.55f, { 180, 200, 255 });
            cc->setPosition({ 210.f, 11.f });
            row->addChild(cc);
        }

        auto progress = usl::ui::label(fmt::format("{}%", r.progress), "goldFont.fnt", 0.45f,
            full ? ccColor3B { 120, 255, 140 } : ccColor3B { 255, 220, 120 });
        progress->setPosition({ w - 110.f, 11.f });
        row->addChild(progress);

        auto score = usl::ui::label(fmt::format("{} pts", usl::formatPoints(r.score)), "chatFont.fnt", 0.55f, { 220, 230, 255 });
        score->setAnchorPoint({ 1.f, 0.5f });
        score->setPosition({ w - 34.f, 11.f });
        row->addChild(score);

        if (!r.videoUrl.empty()) {
            auto menu = CCMenu::create();
            menu->setPosition({ 0.f, 0.f });
            menu->setContentSize(row->getContentSize());
            auto url = r.videoUrl;
            auto btn = CCMenuItemExt::createSpriteExtra(usl::ui::iconSprite("icon_video.png"_spr, 16.f), [url](auto) {
                web::openLinkInBrowser(url);
            });
            btn->setPosition({ w - 16.f, 11.f });
            menu->addChild(btn);
            row->addChild(menu);
        }
        content->addChild(row);
    }
    content->updateLayout();
    m_scroll->scrollToTop();
}
