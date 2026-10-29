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
    usl::ui::addBackground(this, usl::ui::MENU_BLUE);

    auto title = usl::ui::label("Ultimate Shitty List", "goldFont.fnt", 0.9f);
    title->limitLabelWidth(winSize.width - 120.f, 0.9f, 0.4f);
    title->setPosition({ winSize.width / 2.f, winSize.height - 28.f });
    title->setID("title"_spr);
    addChild(title);

    m_statsLabel = usl::ui::label("", "chatFont.fnt", 0.7f);
    m_statsLabel->setPosition({ winSize.width / 2.f, winSize.height - 54.f });
    m_statsLabel->setID("stats-label"_spr);
    addChild(m_statsLabel);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    menu->setID("main-menu"_spr);
    addChild(menu, 2);

    auto back = usl::ui::backButton([this](auto) { onBack(); });
    back->setPosition({ 24.f, winSize.height - 23.f });
    menu->addChild(back);

    float maxWidth = winSize.width - 90.f;

    auto mainRow = listRow({ usl::ListType::Classic, usl::ListType::Challenge }, 150.f, 40.f, 0.7f);
    mainRow->setID("main-lists"_spr);
    mainRow->setScale(std::min(1.f, maxWidth / mainRow->getContentWidth()));
    mainRow->setPosition({ winSize.width / 2.f, winSize.height / 2.f + 46.f });
    addChild(mainRow, 2);

    auto spamTitle = usl::ui::label("Spam Lists", "goldFont.fnt", 0.65f);
    spamTitle->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 4.f });
    spamTitle->setID("spam-title"_spr);
    addChild(spamTitle);

    auto spamRow = listRow({ usl::ListType::Scl, usl::ListType::Controlled, usl::ListType::Unlimited }, 104.f, 32.f, 0.6f);
    spamRow->setID("spam-lists"_spr);
    spamRow->setScale(std::min(1.f, maxWidth / spamRow->getContentWidth()));
    spamRow->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 44.f });
    addChild(spamRow, 2);

    // Bottom row of plain green circle buttons, like the game's main menu.
    auto bottom = CCMenu::create();
    bottom->setID("actions-menu"_spr);
    bottom->setContentSize({ 200.f, 40.f });
    bottom->setPosition({ winSize.width / 2.f, 34.f });
    auto addAction = [&](const char* iconFile, const char* id, std::function<void()> cb) {
        auto circle = CircleButtonSprite::create(usl::ui::crispSprite(iconFile), CircleBaseColor::Green, CircleBaseSize::Small);
        circle->setTopRelativeScale(0.9f);
        auto btn = CCMenuItemExt::createSpriteExtra(circle, [cb](auto) { cb(); });
        btn->setID(id);
        bottom->addChild(btn);
    };
    addAction("icon_players.png"_spr, "players-button", [] { USLPlayersPopup::create(usl::ListType::Classic)->show(); });
    addAction("icon_changes.png"_spr, "changes-button", [] { USLChangesPopup::create(usl::ListType::Classic)->show(); });
    addAction("icon_site.png"_spr, "site-button", [] { usl::ui::openSite("/"); });
    bottom->setLayout(RowLayout::create()->setGap(14.f)->setAutoScale(false));
    addChild(bottom, 2);

    setKeypadEnabled(true);

    WeakRef<USLHubLayer> self = this;
    usl::Store::get().loadStats([self](bool ok) {
        if (auto layer = self.lock()) {
            if (ok) layer->refreshStats();
            else layer->m_statsLabel->setString("Couldn't reach ultimateshittylist.fun");
        }
    });
    usl::Store::get().loadAllLevels([self](bool) {
        if (auto layer = self.lock()) layer->refreshLevels();
    });
    refreshLevels();
    return true;
}

CCMenu* USLHubLayer::listRow(std::initializer_list<usl::ListType> types, float buttonWidth, float buttonHeight, float textScale) {
    constexpr float countHeight = 16.f;
    constexpr float gap = 12.f;
    auto row = CCMenu::create();
    row->ignoreAnchorPointForPosition(false);
    for (auto type : types) {
        // A stock green button with the level count under it, both pressed together.
        auto node = CCNode::create();
        node->setContentSize({ buttonWidth, buttonHeight + countHeight });
        auto spr = ButtonSprite::create(usl::listShortName(type), static_cast<int>(buttonWidth), true, "bigFont.fnt",
            "GJ_button_01.png", buttonHeight, textScale);
        spr->setPosition({ buttonWidth / 2.f, countHeight + buttonHeight / 2.f });
        node->addChild(spr);

        auto count = usl::ui::label("...", "goldFont.fnt", 0.45f);
        count->setPosition({ buttonWidth / 2.f, countHeight / 2.f });
        node->addChild(count);
        m_countLabels[static_cast<size_t>(type)] = count;

        auto btn = CCMenuItemExt::createSpriteExtra(node, [this, type](auto) { openList(type); });
        btn->setID(fmt::format("{}-list-button", usl::listId(type)));
        row->addChild(btn);
    }
    auto count = static_cast<float>(types.size());
    row->setContentSize({ buttonWidth * count + gap * (count - 1.f), buttonHeight + countHeight });
    row->setLayout(RowLayout::create()->setGap(gap)->setAutoScale(false));
    return row;
}

void USLHubLayer::refreshStats() {
    auto const& stats = usl::Store::get().stats();
    m_statsLabel->setString(fmt::format(
        "{} players   {} records   {} levels", stats.users, stats.records, stats.levels
    ).c_str());
}

void USLHubLayer::refreshLevels() {
    auto& store = usl::Store::get();
    for (auto type : usl::LIST_TYPES) {
        auto label = m_countLabels[static_cast<size_t>(type)];
        if (!label || !store.hasLevels(type)) continue;
        auto size = store.levels(type).size();
        label->setString(fmt::format("{} {}", size, size == 1 ? "level" : "levels").c_str());
    }
}

void USLHubLayer::openList(usl::ListType type) {
    CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, USLListLayer::scene(type)));
}

void USLHubLayer::keyBackClicked() {
    onBack();
}

void USLHubLayer::onBack() {
    CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
}
