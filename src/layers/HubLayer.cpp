#include "HubLayer.hpp"
#include "ListLayer.hpp"
#include "../UI.hpp"
#include "../popups/PlayersPopup.hpp"
#include "../popups/ChangesPopup.hpp"

using namespace geode::prelude;

namespace {
    constexpr float CARD_W = 150.f;
    constexpr float CARD_H = 136.f;
    constexpr float CARD_GAP = 14.f;
    constexpr ccColor3B CARD_FILL = { 12, 16, 34 };
    constexpr ccColor3B SPAM_RIM = { 255, 107, 107 };

    std::string countText(int count, const char* one, const char* many) {
        return fmt::format("{} {}", count, count == 1 ? one : many);
    }
}

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
    header->setContentSize({ mark->getScaledContentWidth() + 8.f + title->getScaledContentWidth(), 42.f });
    header->setAnchorPoint({ 0.5f, 0.5f });
    header->setLayout(RowLayout::create()->setGap(8.f)->setAutoScale(false));
    header->setPosition({ winSize.width / 2.f, winSize.height - 30.f });
    addChild(header);

    m_statsLabel = usl::ui::label("Loading...", "chatFont.fnt", 0.6f, { 190, 215, 255 });
    m_statsLabel->setPosition({ winSize.width / 2.f, winSize.height - 57.f });
    m_statsLabel->setID("stats-label"_spr);
    addChild(m_statsLabel);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    menu->setID("main-menu"_spr);
    addChild(menu, 2);

    auto back = usl::ui::backButton([this](auto) { onBack(); });
    back->setPosition({ 24.f, winSize.height - 23.f });
    menu->addChild(back);

    // Cards: Classic, Challenge and one card for the three spam lists.
    // The row is scaled down as a whole on narrow (4:3) screens instead of overlapping the side art.
    float rowW = CARD_W * 3.f + CARD_GAP * 2.f;
    auto cards = CCNode::create();
    cards->setID("lists"_spr);
    cards->setContentSize({ rowW, CARD_H });
    cards->setAnchorPoint({ 0.5f, 0.5f });
    cards->setScale(std::min(1.f, (winSize.width - 70.f) / rowW));
    cards->setPosition({ winSize.width / 2.f, winSize.height / 2.f + 6.f });
    addChild(cards, 2);

    auto cardMenu = CCMenu::create();
    cardMenu->setPosition({ 0.f, 0.f });
    cardMenu->setContentSize(cards->getContentSize());
    cardMenu->setID("list-buttons"_spr);
    cards->addChild(cardMenu, 2);

    for (auto type : { usl::ListType::Classic, usl::ListType::Challenge }) {
        auto idx = static_cast<float>(type);
        auto tile = buildListTile(type, { CARD_W, CARD_H });
        auto btn = CCMenuItemExt::createSpriteExtra(tile, [this, type](auto) { openList(type); });
        // Big cards only lean in a little when pressed.
        btn->m_scaleMultiplier = 1.06f;
        btn->setID(fmt::format("{}-list-button", usl::listId(type)));
        btn->setPosition({ CARD_W / 2.f + (CARD_W + CARD_GAP) * idx, CARD_H / 2.f });
        cardMenu->addChild(btn);
    }

    auto spamCard = buildSpamCard({ CARD_W, CARD_H });
    spamCard->setPosition({ (CARD_W + CARD_GAP) * 2.f, 0.f });
    cards->addChild(spamCard, 1);

    // One row per spam list inside the spam card.
    constexpr float rowH = 24.f;
    constexpr float rowGap = 5.f;
    float rowX = (CARD_W + CARD_GAP) * 2.f + CARD_W / 2.f;
    float rowY = 14.f + rowH / 2.f;
    for (auto it = usl::SPAM_LISTS.rbegin(); it != usl::SPAM_LISTS.rend(); ++it) {
        auto type = *it;
        auto idx = static_cast<size_t>(type);
        float w = CARD_W - 16.f;
        auto row = CCNode::create();
        row->setContentSize({ w, rowH });
        auto rim = usl::ui::panel({ w, rowH }, usl::listColor(type), 255);
        rim->setPosition({ w / 2.f, rowH / 2.f });
        row->addChild(rim);
        auto fill = usl::ui::panel({ w - 2.f, rowH - 2.f }, { 24, 20, 38 }, 245);
        fill->setPosition(rim->getPosition());
        row->addChild(fill);

        auto name = usl::ui::label(usl::listShortName(type), "bigFont.fnt", 0.4f, usl::listColor(type));
        name->setAnchorPoint({ 0.f, 0.5f });
        name->setPosition({ 8.f, rowH / 2.f + 1.f });
        name->limitLabelWidth(70.f, 0.4f, 0.2f);
        row->addChild(name);

        auto count = usl::ui::label("...", "goldFont.fnt", 0.42f);
        count->setAnchorPoint({ 1.f, 0.5f });
        count->setPosition({ w - 8.f, rowH / 2.f });
        row->addChild(count);
        m_levelLabels[idx] = count;

        auto btn = CCMenuItemExt::createSpriteExtra(row, [this, type](auto) { openList(type); });
        btn->m_scaleMultiplier = 1.08f;
        btn->setID(fmt::format("{}-list-button", usl::listId(type)));
        btn->setPosition({ rowX, rowY });
        cardMenu->addChild(btn);
        rowY += rowH + rowGap;
    }

    // Bottom actions.
    auto bottom = CCMenu::create();
    bottom->setID("actions-menu"_spr);
    bottom->setContentSize({ 260.f, 60.f });
    bottom->setPosition({ winSize.width / 2.f, 52.f });

    auto addAction = [&](const char* iconFile, const char* text, const char* id, CircleBaseColor color, std::function<void()> cb) {
        auto node = CCNode::create();
        auto circle = CircleButtonSprite::create(usl::ui::crispSprite(iconFile), color, CircleBaseSize::Small);
        circle->setTopRelativeScale(0.95f);
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

    m_latestLabel = usl::ui::label("", "goldFont.fnt", 0.42f);
    m_latestLabel->setPosition({ winSize.width / 2.f, 13.f });
    m_latestLabel->setID("latest-change"_spr);
    addChild(m_latestLabel, 2);

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
    usl::Store::get().loadChanges(usl::ListType::Classic, [self](bool ok) {
        if (auto layer = self.lock(); layer && ok) layer->refreshLatestChange();
    });
    refreshLevels();
    return true;
}

CCNode* USLHubLayer::buildListTile(usl::ListType type, CCSize size) {
    auto idx = static_cast<size_t>(type);
    auto tile = CCNode::create();
    tile->setContentSize(size);

    // Coloured rim around a dark card so the list colour reads without washing out the text.
    auto rim = usl::ui::panel(size, usl::listColor(type), 255);
    rim->setPosition({ size.width / 2.f, size.height / 2.f });
    tile->addChild(rim);
    auto bg = usl::ui::panel(size - CCSize { 4.f, 4.f }, CARD_FILL, 245);
    bg->setPosition({ size.width / 2.f, size.height / 2.f });
    tile->addChild(bg);
    auto glow = usl::ui::panel({ size.width - 4.f, 62.f }, usl::listDarkColor(type), 200);
    glow->setPosition({ size.width / 2.f, size.height - 33.f });
    tile->addChild(glow);

    auto icon = usl::ui::listIcon(type, 46.f);
    icon->setPosition({ size.width / 2.f, size.height - 34.f });
    tile->addChild(icon);

    auto name = usl::ui::label(usl::listName(type), "bigFont.fnt", 0.5f);
    name->limitLabelWidth(size.width - 16.f, 0.5f, 0.2f);
    name->setPosition({ size.width / 2.f, 62.f });
    tile->addChild(name);

    auto levels = usl::ui::label("...", "goldFont.fnt", 0.5f);
    levels->setPosition({ size.width / 2.f, 44.f });
    tile->addChild(levels);
    m_levelLabels[idx] = levels;

    auto records = usl::ui::label("", "chatFont.fnt", 0.55f, { 210, 220, 240 });
    records->setPosition({ size.width / 2.f, 29.f });
    tile->addChild(records);
    if (type == usl::ListType::Classic) m_classicRecords = records;
    else m_challengeRecords = records;

    auto top = usl::ui::label("", "chatFont.fnt", 0.5f, usl::listColor(type));
    top->setPosition({ size.width / 2.f, 13.f });
    tile->addChild(top);
    m_topLabels[idx] = top;
    return tile;
}

CCNode* USLHubLayer::buildSpamCard(CCSize size) {
    auto card = CCNode::create();
    card->setContentSize(size);
    card->setID("spam-card"_spr);

    auto rim = usl::ui::panel(size, SPAM_RIM, 255);
    rim->setPosition({ size.width / 2.f, size.height / 2.f });
    card->addChild(rim);
    auto bg = usl::ui::panel(size - CCSize { 4.f, 4.f }, CARD_FILL, 245);
    bg->setPosition({ size.width / 2.f, size.height / 2.f });
    card->addChild(bg);
    auto glow = usl::ui::panel({ size.width - 4.f, 44.f }, usl::listDarkColor(usl::ListType::Scl), 200);
    glow->setPosition({ size.width / 2.f, size.height - 24.f });
    card->addChild(glow);

    auto heading = CCNode::create();
    auto icon = usl::ui::listIcon(usl::ListType::Scl, 28.f);
    heading->addChild(icon);
    auto name = usl::ui::label("Spam Lists", "bigFont.fnt", 0.5f);
    heading->addChild(name);
    heading->setContentSize({ icon->getScaledContentWidth() + 6.f + name->getScaledContentWidth(), 28.f });
    heading->setAnchorPoint({ 0.5f, 0.5f });
    heading->setLayout(RowLayout::create()->setGap(6.f)->setAutoScale(false));
    heading->setScale(std::min(1.f, (size.width - 16.f) / heading->getContentWidth()));
    heading->setPosition({ size.width / 2.f, size.height - 18.f });
    card->addChild(heading);

    m_spamRecords = usl::ui::label("", "chatFont.fnt", 0.5f, { 210, 220, 240 });
    m_spamRecords->setPosition({ size.width / 2.f, size.height - 38.f });
    card->addChild(m_spamRecords);
    return card;
}

void USLHubLayer::refreshStats() {
    auto const& stats = usl::Store::get().stats();
    m_statsLabel->setString(fmt::format(
        "{} players   |   {} records   |   {} levels", stats.users, stats.records, stats.levels
    ).c_str());
    m_classicRecords->setString(countText(stats.classicRecords, "record", "records").c_str());
    m_challengeRecords->setString(countText(stats.challengeRecords, "record", "records").c_str());
    m_spamRecords->setString(fmt::format("3 lists, {}", countText(stats.spamRecords, "record", "records")).c_str());
}

void USLHubLayer::refreshLevels() {
    auto& store = usl::Store::get();
    for (auto type : usl::LIST_TYPES) {
        auto idx = static_cast<size_t>(type);
        if (!store.hasLevels(type)) continue;
        auto const& levels = store.levels(type);
        if (m_levelLabels[idx]) {
            m_levelLabels[idx]->setString(usl::isSpam(type)
                ? std::to_string(levels.size()).c_str()
                : countText(static_cast<int>(levels.size()), "level", "levels").c_str());
        }
        if (m_topLabels[idx] && !levels.empty()) {
            m_topLabels[idx]->setString(fmt::format("#1 {}", levels.front().name).c_str());
            m_topLabels[idx]->limitLabelWidth(CARD_W - 16.f, 0.5f, 0.2f);
        }
    }
}

void USLHubLayer::refreshLatestChange() {
    auto const& changes = usl::Store::get().changes(usl::ListType::Classic);
    if (changes.empty()) return;
    auto const& c = changes.front();
    std::string text;
    if (c.kind == "added") text = fmt::format("New on the Classic List: {} at #{}", c.name, c.toRank);
    else if (c.kind == "removed") text = fmt::format("{} left the Classic List", c.name);
    else text = fmt::format("{} moved from #{} to #{}", c.name, c.fromRank, c.toRank);
    m_latestLabel->setString(fmt::format("{}   ({})", text, usl::timeAgo(c.capturedAt)).c_str());
    m_latestLabel->limitLabelWidth(CCDirector::get()->getWinSize().width - 120.f, 0.42f, 0.2f);
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
