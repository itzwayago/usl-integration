#include <Geode/modify/LevelSearchLayer.hpp>
#include "../layers/HubLayer.hpp"

using namespace geode::prelude;

class $modify(USLLevelSearchLayer, LevelSearchLayer) {
    bool init(int type) {
        if (!LevelSearchLayer::init(type)) return false;

        auto menu = getChildByID("other-filter-menu");
        if (!menu || menu->getChildByID("hub-button"_spr)) return true;

        // The round USL badge is a finished button itself; match the size of the menu's other buttons.
        float size = 38.f;
        if (auto first = menu->getChildrenCount() ? typeinfo_cast<CCNode*>(menu->getChildren()->objectAtIndex(0)) : nullptr) {
            size = std::clamp(first->getScaledContentHeight(), 30.f, 46.f);
        }
        auto spr = CCSprite::create("usl_mark.png"_spr);
        spr->setScale(size / spr->getContentHeight());
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(USLLevelSearchLayer::onUSLHub));
        btn->setID("hub-button"_spr);
        menu->addChild(btn);
        menu->updateLayout();
        return true;
    }

    void onUSLHub(CCObject*) {
        CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, USLHubLayer::scene()));
    }
};
