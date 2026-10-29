#include "PlayersPopup.hpp"
#include "../UI.hpp"

using namespace geode::prelude;

namespace {
    constexpr size_t MAX_ROWS = 100;

    ccColor3B rankColor(int rank) {
        if (rank == 1) return { 255, 215, 64 };
        if (rank == 2) return { 215, 225, 240 };
        if (rank == 3) return { 230, 150, 90 };
        return { 255, 255, 255 };
    }
}

USLPlayersPopup* USLPlayersPopup::create(usl::ListType type) {
    auto ret = new USLPlayersPopup();
    if (ret->init(type)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool USLPlayersPopup::init(usl::ListType type) {
    if (!initList(type, "Players", { 420.f, 280.f })) return false;
    setID("USLPlayersPopup");

    auto siteBtn = CCMenuItemExt::createSpriteExtra(
        usl::ui::iconSprite("icon_site.png"_spr, 22.f),
        [this](auto) { usl::ui::openSite(fmt::format("/leaderboard/players?{}", usl::scopeQuery(m_type))); }
    );
    siteBtn->setPosition({ m_size.width - 22.f, m_size.height - 20.f });
    siteBtn->setID("site-button");
    m_buttonMenu->addChild(siteBtn);
    return true;
}

void USLPlayersPopup::load(usl::ListType type) {
    WeakRef<USLPlayersPopup> self = this;
    usl::Store::get().loadPlayers(type, [self, type](bool ok) {
        auto popup = self.lock();
        if (!popup || popup->m_type != type) return;
        if (!ok) {
            popup->setStatus("Couldn't load players.");
            return;
        }
        popup->build();
    });
}

void USLPlayersPopup::build() {
    clearRows();
    auto const& players = usl::Store::get().players(m_type);
    if (players.empty()) {
        setStatus("No records yet.");
        return;
    }
    setStatus("");

    float w = rowWidth();
    for (size_t i = 0; i < std::min(players.size(), MAX_ROWS); i++) {
        auto const& p = players[i];
        auto row = rowBase(i, 28.f);

        auto rank = usl::ui::label(fmt::format("{}", p.rank), "goldFont.fnt", 0.5f, rankColor(p.rank));
        rank->setAnchorPoint({ 0.f, 0.5f });
        rank->setPosition({ 8.f, 14.f });
        rank->limitLabelWidth(38.f, 0.5f, 0.2f);
        row->addChild(rank);

        if (p.rank <= 3) {
            auto crown = usl::ui::iconSprite("icon_crown.png"_spr, 13.f);
            crown->setColor(rankColor(p.rank));
            crown->setPosition({ 50.f, 14.f });
            row->addChild(crown);
        }

        auto name = usl::ui::label(p.username, "bigFont.fnt", 0.42f);
        name->setAnchorPoint({ 0.f, 0.5f });
        name->setPosition({ 62.f, 17.f });
        name->limitLabelWidth(150.f, 0.42f, 0.2f);
        row->addChild(name);

        auto sub = p.hardestName.empty()
            ? fmt::format("{} records", p.records)
            : fmt::format("{} records   |   hardest: #{} {}", p.records, p.hardestRank, p.hardestName);
        auto subLbl = usl::ui::label(sub, "chatFont.fnt", 0.45f, { 200, 210, 230 });
        subLbl->setAnchorPoint({ 0.f, 0.5f });
        subLbl->setPosition({ 62.f, 6.5f });
        subLbl->limitLabelWidth(200.f, 0.45f, 0.2f);
        row->addChild(subLbl);

        if (!p.countryCode.empty()) {
            auto cc = usl::ui::label(p.countryCode, "chatFont.fnt", 0.6f, { 180, 200, 255 });
            cc->setPosition({ w - 112.f, 14.f });
            row->addChild(cc);
        }

        auto score = usl::ui::label(usl::formatPoints(p.score), "goldFont.fnt", 0.5f);
        score->setAnchorPoint({ 1.f, 0.5f });
        score->setPosition({ w - 34.f, 15.f });
        score->limitLabelWidth(70.f, 0.5f, 0.2f);
        row->addChild(score);

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setContentSize(row->getContentSize());
        auto infoBtn = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_infoIcon_001.png", 0.5f, [this, p](auto) {
            showPlayer(p);
        });
        infoBtn->setPosition({ w - 16.f, 14.f });
        menu->addChild(infoBtn);
        row->addChild(menu);

        addRow(row);
    }
    finishRows();
}

void USLPlayersPopup::showPlayer(usl::Player const& p) {
    auto text = fmt::format(
        "<cy>#{}</c> on the {}\n"
        "Score: <cg>{}</c>\n"
        "Records: <cl>{}</c>   Verified: <cp>{}</c>\n"
        "{}",
        p.rank, usl::listName(m_type), usl::formatPoints(p.score), p.records, p.verified,
        p.hardestName.empty() ? std::string() : fmt::format("Hardest: <co>#{} {}</c>", p.hardestRank, p.hardestName)
    );
    auto slug = p.slug;
    createQuickPopup(p.username.c_str(), text, "Close", "Profile", [slug](auto, bool profile) {
        if (profile && !slug.empty()) usl::ui::openSite(fmt::format("/profile/{}", slug));
    });
}
