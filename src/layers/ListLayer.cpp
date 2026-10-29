#include "ListLayer.hpp"
#include "../UI.hpp"
#include <algorithm>
#include <cctype>

using namespace geode::prelude;

namespace {
    constexpr float LIST_W = 356.f;
    constexpr float LIST_H = 184.f;
    constexpr float LIST_Y = 60.f;
    constexpr float CONTROLS_Y = 18.f;
    constexpr const char* CATEGORIES[] = { "", "main", "extended", "advanced" };

    std::string lower(std::string text) {
        std::ranges::transform(text, text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return text;
    }

    std::string listInfo(usl::ListType type) {
        constexpr const char* sections =
            "<cy>Main</c> is the top 75, <cj>Extended</c> the rest that give points, <cp>Advanced</c> everything past that.\n";
        constexpr const char* records =
            "Beat a level to <cg>100%</c> (or reach its list percent) and send the record on <cl>ultimateshittylist.fun</c>.";
        switch (type) {
            case usl::ListType::Classic:
                return fmt::format("Rated and 1.0-style <cr>Shitty</c> levels, placed by the USL team.\n{}{}", sections, records);
            case usl::ListType::Challenge:
                return fmt::format("Short Shitty challenges, placed by the USL team.\n{}{}", sections, records);
            case usl::ListType::Scl:
                return fmt::format("Pure CPS spam under Spam Challenge List rules: up to two inputs and four fingers, "
                    "no drag, scroll or lip spam.\n{}", records);
            case usl::ListType::Controlled:
                return fmt::format("Spam that tests control: click patterns, rhythm changes and tight spots where "
                    "accuracy matters as much as speed.\n{}", records);
            case usl::ListType::Unlimited:
                return fmt::format("Any spam a human does: drag, rake, scroll, lip, any number of keys. "
                    "Macros and autoclickers are banned.\n{}", records);
        }
        return records;
    }
}

USLListLayer* USLListLayer::create(usl::ListType type) {
    auto ret = new USLListLayer();
    if (ret->init(type)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCScene* USLListLayer::scene(usl::ListType type) {
    auto scene = CCScene::create();
    scene->addChild(USLListLayer::create(type));
    return scene;
}

bool USLListLayer::init(usl::ListType type) {
    if (!CCLayer::init()) return false;
    setID("USLListLayer");
    m_type = type;

    auto winSize = CCDirector::get()->getWinSize();
    usl::ui::addBackground(this, usl::ui::MENU_BLUE);

    m_list = GJListLayer::create(nullptr, usl::listName(type), { 191, 114, 62, 255 }, LIST_W, LIST_H, 0);
    m_list->setPosition({ winSize.width / 2.f - LIST_W / 2.f, LIST_Y });
    m_list->setID("level-list"_spr);
    addChild(m_list, 2);

    m_error = usl::ui::label("", "bigFont.fnt", 0.5f);
    m_error->setAlignment(kCCTextAlignmentCenter);
    m_error->setPosition({ LIST_W / 2.f, LIST_H / 2.f });
    m_error->setVisible(false);
    m_list->addChild(m_error, 10);

    m_spinner = LoadingSpinner::create(50.f);
    m_spinner->setPosition({ winSize.width / 2.f, m_list->getPositionY() + LIST_H / 2.f });
    addChild(m_spinner, 10);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    menu->setID("controls-menu"_spr);
    addChild(menu, 5);

    auto back = usl::ui::backButton([this](auto) { onBack(); });
    back->setPosition({ 24.f, winSize.height - 23.f });
    menu->addChild(back);

    auto prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    m_prev = CCMenuItemExt::createSpriteExtra(prevSpr, [this](auto) { showPage(m_page - 1); });
    m_prev->setPosition({ 24.f, LIST_Y + LIST_H / 2.f });
    m_prev->setID("prev-page-button");
    menu->addChild(m_prev);

    auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextSpr->setFlipX(true);
    m_next = CCMenuItemExt::createSpriteExtra(nextSpr, [this](auto) { showPage(m_page + 1); });
    m_next->setPosition({ winSize.width - 24.f, LIST_Y + LIST_H / 2.f });
    m_next->setID("next-page-button");
    menu->addChild(m_next);

    auto refreshSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    auto refresh = CCMenuItemExt::createSpriteExtra(refreshSpr, [this](auto) {
        showLoading(true);
        WeakRef<USLListLayer> self = this;
        auto type = m_type;
        usl::Store::get().loadLevels(type, [self, type](bool ok) {
            auto layer = self.lock();
            if (!layer || layer->m_type != type) return;
            if (ok) layer->applyFilter();
            else layer->showError("Couldn't load the list.\nCheck your connection.");
        }, true);
    });
    refresh->setPosition({ winSize.width - 26.f, 26.f });
    refresh->setID("refresh-button");
    menu->addChild(refresh);

    // The text depends on the list, so it is built when tapped, not when the layer opens.
    auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    infoSpr->setScale(0.8f);
    auto infoBtn = CCMenuItemExt::createSpriteExtra(infoSpr, [this](auto) {
        FLAlertLayer::create(nullptr, usl::listName(m_type), listInfo(m_type), "OK", nullptr, 360.f)->show();
    });
    infoBtn->setPosition({ 24.f, winSize.height - 60.f });
    infoBtn->setID("info-button");
    menu->addChild(infoBtn);

    m_categorySpr = ButtonSprite::create("All", 76, true, "bigFont.fnt", "GJ_button_04.png", 28.f, 0.5f);
    m_categorySpr->setScale(0.75f);
    m_categoryBtn = CCMenuItemExt::createSpriteExtra(m_categorySpr, [this](auto) { cycleCategory(); });
    m_categoryBtn->setPosition({ winSize.width / 2.f + 128.f, CONTROLS_Y });
    m_categoryBtn->setID("category-button");
    menu->addChild(m_categoryBtn);

    m_search = TextInput::create(250.f, "Search level, creator or ID");
    m_search->setPosition({ winSize.width / 2.f - 42.f, CONTROLS_Y });
    m_search->setScale(0.75f);
    m_search->setMaxCharCount(40);
    m_search->setID("search-input"_spr);
    m_search->setCallback([this](std::string const&) {
        // Wait until typing stops so every keystroke doesn't hit the GD servers.
        unschedule(schedule_selector(USLListLayer::onSearchTimer));
        scheduleOnce(schedule_selector(USLListLayer::onSearchTimer), 0.45f);
    });
    addChild(m_search, 5);

    m_pageInfo = usl::ui::label("", "goldFont.fnt", 0.45f);
    m_pageInfo->setAnchorPoint({ 1.f, 1.f });
    m_pageInfo->setPosition({ winSize.width - 7.f, winSize.height - 4.f });
    m_pageInfo->setID("page-info"_spr);
    addChild(m_pageInfo, 5);

    setKeypadEnabled(true);
    setKeyboardEnabled(true);

    loadList();
    return true;
}

void USLListLayer::loadList() {
    m_page = 1;
    showLoading(true);
    WeakRef<USLListLayer> self = this;
    auto type = m_type;
    usl::Store::get().loadLevels(type, [self, type](bool ok) {
        auto layer = self.lock();
        if (!layer || layer->m_type != type) return;
        if (ok) layer->applyFilter();
        else layer->showError("Couldn't load the list.\nCheck your connection.");
    });
}

void USLListLayer::onSearchTimer(float) {
    runSearch();
}

void USLListLayer::runSearch() {
    unschedule(schedule_selector(USLListLayer::onSearchTimer));
    auto query = lower(std::string(m_search->getString()));
    // Trim spaces and a leading '#', so "#12" and "12 " both find rank 12.
    while (!query.empty() && std::isspace(static_cast<unsigned char>(query.back()))) query.pop_back();
    while (!query.empty() && (std::isspace(static_cast<unsigned char>(query.front())) || query.front() == '#')) query.erase(0, 1);
    if (query == m_query) return;
    m_query = query;
    m_page = 1;
    applyFilter();
}

void USLListLayer::cycleCategory() {
    size_t current = 0;
    for (size_t i = 0; i < std::size(CATEGORIES); i++) {
        if (m_category == CATEGORIES[i]) current = i;
    }
    m_category = CATEGORIES[(current + 1) % std::size(CATEGORIES)];
    auto text = m_category.empty() ? std::string("All") : usl::categoryName(m_category);
    m_categorySpr->setString(text.c_str());
    if (m_categorySpr->m_label) {
        m_categorySpr->m_label->setColor(m_category.empty() ? ccColor3B{ 255, 255, 255 } : usl::categoryColor(m_category));
    }
    m_page = 1;
    applyFilter();
}

void USLListLayer::applyFilter() {
    m_filtered.clear();
    for (auto const& level : usl::Store::get().levels(m_type)) {
        if (!m_category.empty() && level.category != m_category) continue;
        if (!m_query.empty()) {
            bool hit = lower(level.name).find(m_query) != std::string::npos ||
                lower(level.creator).find(m_query) != std::string::npos ||
                lower(level.verifier).find(m_query) != std::string::npos ||
                std::to_string(level.rank) == m_query ||
                std::to_string(level.gdId) == m_query;
            if (!hit) continue;
        }
        m_filtered.push_back(level);
    }
    showPage(m_page);
}

int USLListLayer::maxPage() const {
    return std::max(1, (static_cast<int>(m_filtered.size()) + m_perPage - 1) / m_perPage);
}

void USLListLayer::showPage(int page) {
    m_page = std::clamp(page, 1, maxPage());

    if (m_filtered.empty()) {
        setListView(CCArray::create());
        showLoading(false);
        showError(m_query.empty() && m_category.empty() ? "This list is empty." : "No levels match.");
        m_pageInfo->setString("0 levels");
        return;
    }

    size_t begin = static_cast<size_t>(m_perPage * (m_page - 1));
    size_t end = std::min(begin + m_perPage, m_filtered.size());
    std::string ids;
    for (size_t i = begin; i < end; i++) {
        if (i > begin) ids += ",";
        ids += std::to_string(m_filtered[i].gdId);
    }

    m_pageInfo->setString(fmt::format("{} to {} of {}", begin + 1, end, m_filtered.size()).c_str());

    showLoading(true);
    auto glm = GameLevelManager::get();
    glm->m_levelManagerDelegate = this;

    auto search = GJSearchObject::create(SearchType::Type26);
    search->m_searchQuery = ids;
    std::string key = search->getKey();
    m_pendingKey = key;
    if (auto stored = glm->getStoredOnlineLevels(key.substr(std::max<ptrdiff_t>(0, static_cast<ptrdiff_t>(key.size()) - 256)).c_str())) {
        setListView(stored);
        showLoading(false);
    } else {
        glm->getOnlineLevels(search);
    }
}

void USLListLayer::setListView(CCArray* levels) {
    // Keep USL order: the GD server answers in its own order.
    std::unordered_map<int, int> order;
    size_t begin = static_cast<size_t>(m_perPage * (m_page - 1));
    for (size_t i = begin; i < std::min(begin + m_perPage, m_filtered.size()); i++) {
        order[m_filtered[i].gdId] = m_filtered[i].rank;
    }
    std::vector<GJGameLevel*> sorted;
    for (auto level : CCArrayExt<GJGameLevel*>(levels)) {
        if (order.contains(level->m_levelID.value())) sorted.push_back(level);
    }
    std::ranges::sort(sorted, {}, [&](GJGameLevel* l) { return order[l->m_levelID.value()]; });
    auto arr = CCArray::create();
    for (auto level : sorted) arr->addObject(level);

    // GJListLayer releases m_listView on destruction, so it holds an extra reference.
    if (auto old = m_list->m_listView) {
        old->removeFromParent();
        old->release();
    }
    auto view = CustomListView::create(arr, BoomListType::Level, LIST_H, LIST_W);
    view->retain();
    m_list->addChild(view, 6, 9);
    m_list->m_listView = view;
}

void USLListLayer::loadLevelsFinished(CCArray* levels, char const* key, int) {
    loadLevelsFinished(levels, key);
}

void USLListLayer::loadLevelsFinished(CCArray* levels, char const* key) {
    if (!isCurrentKey(key)) return;
    setListView(levels);
    showLoading(false);
}

void USLListLayer::loadLevelsFailed(char const* key, int) {
    loadLevelsFailed(key);
}

void USLListLayer::loadLevelsFailed(char const* key) {
    if (!isCurrentKey(key)) return;
    showError("GD servers didn't answer.\nTry again in a moment.");
}

void USLListLayer::setupPageInfo(gd::string, char const*) {}

bool USLListLayer::isCurrentKey(char const* key) const {
    if (!key) return true;
    std::string_view k = key;
    return m_pendingKey == k || std::string_view(m_pendingKey).ends_with(k);
}

void USLListLayer::onEnter() {
    CCLayer::onEnter();
    // A page request may have been dropped while a level page was on top.
    if (m_spinner && m_spinner->isVisible() && !m_filtered.empty()) showPage(m_page);
}

void USLListLayer::showLoading(bool loading) {
    m_spinner->setVisible(loading);
    m_error->setVisible(false);
    if (auto view = m_list->m_listView) view->setVisible(!loading);
    bool paged = !loading && m_filtered.size() > static_cast<size_t>(m_perPage);
    m_prev->setVisible(paged && m_page > 1);
    m_next->setVisible(paged && m_page < maxPage());
}

void USLListLayer::showError(std::string const& text) {
    showLoading(false);
    if (auto view = m_list->m_listView) view->setVisible(false);
    m_error->setString(text.c_str());
    m_error->limitLabelWidth(LIST_W - 30.f, 0.5f, 0.2f);
    m_error->setVisible(true);
}

void USLListLayer::keyDown(enumKeyCodes key, double timestamp) {
    switch (key) {
        case KEY_Left:
        case CONTROLLER_Left:
            if (m_prev->isVisible()) showPage(m_page - 1);
            break;
        case KEY_Right:
        case CONTROLLER_Right:
            if (m_next->isVisible()) showPage(m_page + 1);
            break;
        case KEY_Enter:
            runSearch();
            break;
        default:
            CCLayer::keyDown(key, timestamp);
    }
}

void USLListLayer::keyBackClicked() {
    onBack();
}

void USLListLayer::onExit() {
    auto glm = GameLevelManager::get();
    if (glm->m_levelManagerDelegate == this) glm->m_levelManagerDelegate = nullptr;
    if (m_search) m_search->defocus();
    CCLayer::onExit();
}

void USLListLayer::onBack() {
    CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
}
