#include "UI.hpp"
#include "Rate.hpp"
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

namespace usl::ui {
    namespace {
        // Mod sprites are 256 px (UHD) or smaller and drawn anywhere from 8 to 50 units.
        // Without mipmaps such a downscale skips texels and the outline breaks up,
        // so every mod texture gets them once. GLES2 only mipmaps power-of-two sizes.
        void smooth(CCTexture2D* texture) {
            if (!texture || texture->hasMipmaps()) return;
            auto w = texture->getPixelsWide();
            auto h = texture->getPixelsHigh();
            auto pot = [](unsigned int v) { return v > 0 && (v & (v - 1)) == 0; };
            if (!pot(w) || !pot(h)) return;
            texture->generateMipmap();
            // Raw GL on purpose: setTexParameters would make Android remember the mipmap filter
            // and reapply it after a lost GL context, when the reloaded texture has no mipmaps
            // and would draw black. This way a reload just falls back to plain linear filtering,
            // and the next sprite made from the texture rebuilds the mipmaps.
            ccGLBindTexture2D(texture->getName());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

    CCSprite* crispSprite(const char* file) {
        auto spr = CCSprite::create(file);
        if (!spr) {
            log::warn("USL: missing sprite {}", file);
            return CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        }
        smooth(spr->getTexture());
        return spr;
    }

    CCSprite* iconSprite(const char* file, float size) {
        auto spr = crispSprite(file);
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

    CCNode* rateFace(Level const& level, float height) {
        if (level.difficulty == Difficulty::None) return nullptr;
        auto face = GJDifficultySprite::create(rate::demonFace(level.difficulty), GJDifficultyName::Short);
        if (!face) return nullptr;
        face->updateFeatureState(rate::featureState(level.quality));
        // The glow is a child drawn bigger than the face; wrap it so layouts see one square node.
        auto holder = CCNode::create();
        holder->setContentSize({ height, height });
        holder->setAnchorPoint({ 0.5f, 0.5f });
        face->setScale(height / face->getContentHeight());
        face->setPosition({ height / 2.f, height / 2.f });
        holder->addChild(face);
        return holder;
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

    CCNode* listSwitcher(ListType selected, float width, std::function<void(ListType)> onSelect) {
        constexpr float height = 24.f;
        // The name sits next to the menu, not in it: a CCMenu treats every child as a menu item.
        auto node = CCNode::create();
        node->setID("list-switcher"_spr);
        node->setAnchorPoint({ 0.5f, 0.5f });
        node->setContentSize({ width, height });
        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setContentSize({ width, height });
        node->addChild(menu);

        auto step = [selected](int delta) {
            auto count = static_cast<int>(LIST_COUNT);
            return LIST_TYPES[static_cast<size_t>((static_cast<int>(selected) + delta + count) % count)];
        };
        auto arrow = [&](bool next) {
            auto spr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
            spr->setScale(0.45f);
            if (next) spr->setFlipX(true);
            auto target = step(next ? 1 : -1);
            auto btn = CCMenuItemExt::createSpriteExtra(spr, [onSelect, target](auto) { if (onSelect) onSelect(target); });
            btn->setID(next ? "next-list-button" : "prev-list-button");
            btn->setPosition({ next ? width - 10.f : 10.f, height / 2.f });
            menu->addChild(btn);
        };
        arrow(false);
        arrow(true);

        auto name = label(listName(selected), "bigFont.fnt", 0.5f);
        name->limitLabelWidth(width - 50.f, 0.5f, 0.2f);
        name->setPosition({ width / 2.f, height / 2.f });
        name->setID("list-name");
        node->addChild(name);
        return node;
    }

    void openSite(std::string const& path) {
        web::openLinkInBrowser(fmt::format("{}{}", SITE, path));
    }
}
