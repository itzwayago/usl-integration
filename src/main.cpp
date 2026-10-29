#include <Geode/Geode.hpp>
#include "USL.hpp"

using namespace geode::prelude;

class USLRefresher : public CCObject {
public:
    void refresh(float) {
        usl::Store::get().loadAllLevels(nullptr);
    }
};

$on_game(Loaded) {
    // Placements are needed by level cells and level pages, so load the lists up front.
    usl::Store::get().loadAllLevels([](bool ok) {
        if (!ok) log::warn("USL lists failed to load on startup");
    });
    static auto refresher = new USLRefresher();
    CCDirector::get()->getScheduler()->scheduleSelector(
        schedule_selector(USLRefresher::refresh), refresher, 1800.f, false
    );
}
