#include "ChangesPopup.hpp"
#include "../UI.hpp"

using namespace geode::prelude;

USLChangesPopup* USLChangesPopup::create(usl::ListType type) {
    auto ret = new USLChangesPopup();
    if (ret->init(type)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool USLChangesPopup::init(usl::ListType type) {
    if (!initList(type, "Recent Changes", { 400.f, 270.f })) return false;
    setID("USLChangesPopup");
    return true;
}

void USLChangesPopup::load(usl::ListType type) {
    WeakRef<USLChangesPopup> self = this;
    usl::Store::get().loadChanges(type, [self, type](bool ok) {
        auto popup = self.lock();
        if (!popup || popup->m_type != type) return;
        if (!ok) {
            popup->setStatus("Couldn't load changes.");
            return;
        }
        popup->build();
    });
}

void USLChangesPopup::build() {
    clearRows();
    auto const& changes = usl::Store::get().changes(m_type);
    if (changes.empty()) {
        setStatus("Nothing changed recently.");
        return;
    }
    setStatus("");

    float w = rowWidth();
    size_t i = 0;
    for (auto const& c : changes) {
        auto row = rowBase(i++, 26.f);

        std::string badge;
        std::string move;
        ccColor3B color;
        if (c.kind == "added") {
            badge = "NEW";
            move = fmt::format("Top {}", c.toRank);
            color = { 100, 255, 120 };
        } else if (c.kind == "removed") {
            badge = "OUT";
            move = fmt::format("was Top {}", c.fromRank);
            color = { 255, 100, 100 };
        } else {
            bool up = c.toRank < c.fromRank;
            badge = up ? "UP" : "DOWN";
            move = fmt::format("{} to {}", c.fromRank, c.toRank);
            color = up ? ccColor3B { 120, 220, 255 } : ccColor3B { 255, 180, 90 };
        }

        auto badgeBg = usl::ui::panel({ 40.f, 16.f }, color, 255);
        badgeBg->setPosition({ 26.f, 13.f });
        row->addChild(badgeBg);
        auto badgeLbl = usl::ui::label(badge, "bigFont.fnt", 0.3f, { 20, 20, 30 });
        badgeLbl->setPosition(badgeBg->getPosition());
        badgeLbl->limitLabelWidth(34.f, 0.3f, 0.15f);
        row->addChild(badgeLbl);

        auto name = usl::ui::label(c.name, "bigFont.fnt", 0.4f);
        name->setAnchorPoint({ 0.f, 0.5f });
        name->setPosition({ 54.f, 13.f });
        name->limitLabelWidth(160.f, 0.4f, 0.2f);
        row->addChild(name);

        auto moveLbl = usl::ui::label(move, "goldFont.fnt", 0.45f, color);
        moveLbl->setAnchorPoint({ 1.f, 0.5f });
        moveLbl->setPosition({ w - 64.f, 13.f });
        moveLbl->limitLabelWidth(110.f, 0.45f, 0.2f);
        row->addChild(moveLbl);

        auto when = usl::ui::label(usl::timeAgo(c.capturedAt), "chatFont.fnt", 0.5f, { 190, 200, 220 });
        when->setAnchorPoint({ 1.f, 0.5f });
        when->setPosition({ w - 8.f, 13.f });
        row->addChild(when);

        addRow(row);
    }
    finishRows();
}
