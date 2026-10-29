#include "UI.hpp"
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

namespace usl::ui {
    CCSprite* iconSprite(const char* file, float size) {
        auto spr = CCSprite::create(file);
        if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        spr->setScale(size / std::max(spr->getContentWidth(), spr->getContentHeight()));
        return spr;
    }

    CCSprite* listIcon(ListType type, float size) {
        return iconSprite(usl::listIcon(type), size);
    }

    CCLabelBMFont* label(std::string const& text, const char* font, float scale, ccColor3B color) {
        auto lbl = CCLabelBMFont::create(text.c_str(), font);
        lbl->setScale(scale);
        lbl->setColor(color);
        return lbl;
    }

    NineSlice* panel(CCSize size, ccColor3B color, GLubyte opacity) {
        auto bg = NineSlice::create("square02b_001.png");
        // The texture's rounded corners are ~13 units; shrink them for small panels so they don't overlap.
        bg->setScaleMultiplier(std::clamp(std::min(size.width, size.height) / 30.f, 0.15f, 1.f));
        bg->setContentSize(size);
        bg->setColor(color);
        bg->setOpacity(opacity);
        return bg;
    }

    CCNode* rankBadge(ListType type, int rank, float height, std::string const& prefix) {
        auto node = CCNode::create();
        float iconSize = height - 3.f;
        auto text = label(fmt::format("{}{}", prefix, rank), "bigFont.fnt", 1.f);
        text->setScale((height - 5.f) / text->getContentHeight() * 1.25f);
        float pad = height * 0.28f;
        float width = pad + iconSize + 2.f + text->getScaledContentWidth() + pad;
        node->setContentSize({ width, height });

        auto rim = panel({ width, height }, listColor(type), 255);
        rim->setPosition({ width / 2.f, height / 2.f });
        node->addChild(rim);
        auto fill = panel({ width - 2.f, height - 2.f }, { 14, 16, 28 }, 235);
        fill->setPosition(rim->getPosition());
        node->addChild(fill);

        auto icon = listIcon(type, iconSize);
        icon->setPosition({ pad + iconSize / 2.f, height / 2.f });
        node->addChild(icon);

        text->setAnchorPoint({ 0.f, 0.5f });
        text->setPosition({ pad + iconSize + 2.f, height / 2.f + 0.5f });
        node->addChild(text);
        return node;
    }

    void addBackground(CCLayer* layer, ccColor3B color) {
        auto winSize = CCDirector::get()->getWinSize();
        auto bg = CCSprite::create("GJ_gradientBG.png");
        bg->setAnchorPoint({ 0.f, 0.f });
        bg->setScaleX((winSize.width + 10.f) / bg->getTextureRect().size.width);
        bg->setScaleY((winSize.height + 10.f) / bg->getTextureRect().size.height);
        bg->setPosition({ -5.f, -5.f });
        bg->setColor(color);
        bg->setID("background"_spr);
        layer->addChild(bg, -2);

        auto left = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
        left->setAnchorPoint({ 0.f, 0.f });
        left->setPosition({ -1.f, -1.f });
        left->setID("corner-left"_spr);
        layer->addChild(left, -1);

        auto right = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
        right->setAnchorPoint({ 1.f, 0.f });
        right->setFlipX(true);
        right->setPosition({ winSize.width + 1.f, -1.f });
        right->setID("corner-right"_spr);
        layer->addChild(right, -1);
    }

    CCMenuItemSpriteExtra* backButton(geode::Function<void(CCMenuItemSpriteExtra*)> callback) {
        auto btn = CCMenuItemExt::createSpriteExtra(
            CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"), std::move(callback)
        );
        btn->setID("back-button"_spr);
        return btn;
    }

    CCMenu* listTabs(ListType selected, float scale, std::function<void(ListType)> onSelect) {
        auto menu = CCMenu::create();
        menu->setID("list-tabs"_spr);
        menu->setContentSize({ 350.f * scale, 30.f * scale });
        menu->ignoreAnchorPointForPosition(false);

        // One caption scale for all tabs, sized so the longest name fits next to the icon.
        constexpr float captionWidth = 62.f;
        float captionScale = 0.55f;
        for (auto type : LIST_TYPES) {
            auto probe = CCLabelBMFont::create(listShortName(type), "bigFont.fnt");
            captionScale = std::min(captionScale, captionWidth / probe->getContentWidth());
        }

        for (auto type : LIST_TYPES) {
            bool active = type == selected;
            auto spr = ButtonSprite::create(
                listShortName(type), 90, true, "bigFont.fnt",
                active ? "GJ_button_02.png" : "GJ_button_04.png", 30.f, captionScale
            );
            // Make room for the list icon on the left of the caption.
            if (auto lbl = spr->m_label) {
                lbl->setScale(captionScale);
                lbl->setPositionX(lbl->getPositionX() + 10.f);
            }
            auto icon = listIcon(type, 22.f);
            icon->setPosition({ 15.f, spr->getContentHeight() / 2.f + 1.f });
            spr->addChild(icon, 5);
            spr->setScale(scale);

            auto btn = CCMenuItemExt::createSpriteExtra(spr, [onSelect, type](auto) {
                if (onSelect) onSelect(type);
            });
            btn->setID(fmt::format("tab-{}", listKey(type)));
            btn->setEnabled(!active);
            menu->addChild(btn);
        }

        menu->setLayout(RowLayout::create()->setGap(6.f * scale)->setAutoScale(false));
        return menu;
    }

    void openSite(std::string const& path) {
        web::openLinkInBrowser(fmt::format("{}{}", SITE, path));
    }

    std::string trimText(std::string const& text, size_t maxChars) {
        if (text.size() <= maxChars) return text;
        return text.substr(0, maxChars - 2) + "..";
    }
}
