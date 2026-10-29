#include "HubLayer.hpp"
#include "ListLayer.hpp"
#include "../UI.hpp"
#include "../popups/PlayersPopup.hpp"
#include "../popups/ChangesPopup.hpp"

using namespace geode::prelude;

USLHubLayer* USLHubLayer::create() {
    auto ret = new USLHubLayer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCScene* USLHubLayer::scene() {
    auto scene = CCScene::create();
    scene->addChild(USLHubLayer::create());
    return scene;
}

bool USLHubLayer::init() {
    if (!CCLayer::init()) return false;
    setID("USLHubLayer");

    auto winSize = CCDirector::get()->getWinSize();
    usl::ui::addBackground(this, { 28, 52, 110 });

    // Header: site mark + name.
    auto header = CCNode::create();
    header->setID("header"_spr);
    auto mark = usl::ui::iconSprite("usl_mark.png"_spr, 42.f);
    header->addChild(mark);
    auto title = usl::ui::label("Ultimate Shitty List", "goldFont.fnt", 0.85f);
    header->addChild(title);
    header->setContentSize({ mark->getScaledContentWidth() + 8.f + title->getScaledContentWidth(), 38.f });
    header->setAnchorPoint({ 0.5f, 0.5f });
    header->setLayout(RowLayout::create()->setGap(8.f)->setAutoScale(false));
    header->setPosition({ winSize.width / 2.f, winSize.height - 30.f });
    addChild(header);

    auto subtitle = usl::ui::label("1.0-style and rated Shitty levels", "chatFont.fnt", 0.6f, { 190, 215, 255 });
    subtitle->setPosition({ winSize.width / 2.f, winSize.height - 55.f });
    subtitle->setID("subtitle"_spr);
    addChild(subtitle);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    menu->setID("main-menu"_spr);
    addChild(menu, 2);

    auto back = usl::ui::backButton([this](auto) { onBack(); });
    back->setPosition({ 24.f, winSize.height - 23.f });
    menu->addChild(back);

    // Three list tiles.
    constexpr float tileW = 150.f;
    constexpr float tileH = 128.f;
    constexpr float gap = 14.f;
    float startX = winSize.width / 2.f - tileW - gap;
    float tileY = winSize.height / 2.f + 8.f;

    for (auto type : usl::LIST_TYPES) {
        auto idx = static_cast<size_t>(type);
        auto tile = CCNode::create();
        tile->setContentSize({ tileW, tileH });

        // Coloured rim around a dark card so the list colour reads without washing out the text.
        auto rim = usl::ui::panel({ tileW, tileH }, usl::listColor(type), 255);
        rim->setPosition({ tileW / 2.f, tileH / 2.f });
        tile->addChild(rim);
        auto bg = usl::ui::panel({ tileW - 4.f, tileH - 4.f }, { 12, 16, 34 }, 245);
        bg->setPosition({ tileW / 2.f, tileH / 2.f });
        tile->addChild(bg);
        auto glow = usl::ui::panel({ tileW - 4.f, 64.f }, usl::listDarkColor(type), 200);
        glow->setPosition({ tileW / 2.f, tileH - 34.f });
        tile->addChild(glow);

        auto icon = usl::ui::listIcon(type, 50.f);
        icon->setPosition({ tileW / 2.f, tileH - 42.f });
        tile->addChild(icon);

        auto name = usl::ui::label(usl::listName(type), "bigFont.fnt", 0.5f);
        name->limitLabelWidth(tileW - 16.f, 0.5f, 0.2f);
        name->setPosition({ tileW / 2.f, 46.f });
        tile->addChild(name);

        auto levels = usl::ui::label("... levels", "goldFont.fnt", 0.5f);
        levels->setPosition({ tileW / 2.f, 27.f });
        tile->addChild(levels);
        m_levelLabels[idx] = levels;

        auto records = usl::ui::label("... records", "chatFont.fnt", 0.55f, { 210, 220, 240 });
        records->setPosition({ tileW / 2.f, 12.f });
        tile->addChild(records);
        m_recordLabels[idx] = records;

        auto btn = CCMenuItemExt::createSpriteExtra(tile, [type](auto) {
            CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, USLListLayer::scene(type)));
        });
        btn->setID(fmt::format("{}-list-button", usl::listKey(type)));
        btn->setPosition({ startX + (tileW + gap) * static_cast<float>(idx), tileY });
        menu->addChild(btn);
    }

    // Bottom actions.
    auto bottom = CCMenu::create();
    bottom->setID("actions-menu"_spr);
    bottom->setContentSize({ 260.f, 60.f });
    bottom->setPosition({ winSize.width / 2.f, 52.f });

    auto addAction = [&](const char* iconFile, const char* text, const char* id, CircleBaseColor color, std::function<void()> cb) {
        auto node = CCNode::create();
        auto circle = CircleButtonSprite::createWithSprite(iconFile, 0.95f, color, CircleBaseSize::Small);
        node->setContentSize({ 70.f, circle->getContentHeight() + 14.f });
        circle->setPosition({ 35.f, node->getContentHeight() - circle->getContentHeight() / 2.f });
        node->addChild(circle);
        auto lbl = usl::ui::label(text, "bigFont.fnt", 0.32f);
        lbl->setPosition({ 35.f, 5.f });
        node->addChild(lbl);
        auto btn = CCMenuItemExt::createSpriteExtra(node, [cb](auto) { cb(); });
        btn->setID(id);
        bottom->addChild(btn);
    };

    addAction("icon_players.png"_spr, "Players", "players-button", CircleBaseColor::Green, [] {
        USLPlayersPopup::create(usl::ListType::Classic)->show();
    });
    addAction("icon_changes.png"_spr, "Changes", "changes-button", CircleBaseColor::Cyan, [] {
        USLChangesPopup::create(usl::ListType::Classic)->show();
    });
    addAction("icon_site.png"_spr, "Website", "site-button", CircleBaseColor::Blue, [] {
        usl::ui::openSite("/");
    });
    bottom->setLayout(RowLayout::create()->setGap(16.f)->setAutoScale(false));
    addChild(bottom, 2);

    m_statsLabel = usl::ui::label("Loading stats...", "goldFont.fnt", 0.45f);
    m_statsLabel->setPosition({ winSize.width / 2.f, 14.f });
    m_statsLabel->setID("stats-label"_spr);
    addChild(m_statsLabel, 2);

    setKeypadEnabled(true);

    WeakRef<USLHubLayer> self = this;
    usl::Store::get().loadStats([self](bool ok) {
        if (auto layer = self.lock()) {
            if (ok) layer->refreshStats();
            else layer->m_statsLabel->setString("Couldn't reach ultimateshittylist.fun");
        }
    });
    usl::Store::get().loadAllLevels(nullptr);
    return true;
}

void USLHubLayer::refreshStats() {
    auto const& stats = usl::Store::get().stats();
    for (auto type : usl::LIST_TYPES) {
        auto idx = static_cast<size_t>(type);
        m_levelLabels[idx]->setString(fmt::format("{} levels", stats.levelsByList[idx]).c_str());
        m_recordLabels[idx]->setString(fmt::format("{} records", stats.recordsByList[idx]).c_str());
    }
    m_statsLabel->setString(fmt::format(
        "{} players  -  {} records  -  {} levels", stats.users, stats.records, stats.levels
    ).c_str());
}

void USLHubLayer::keyBackClicked() {
    onBack();
}

void USLHubLayer::onBack() {
    CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
}
