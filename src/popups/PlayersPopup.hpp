#pragma once

#include "ListPopup.hpp"

/// Players leaderboard for one list (top 100), like the site's leaderboard page.
class USLPlayersPopup : public USLListPopup {
public:
    static USLPlayersPopup* create(usl::ListType type);

protected:
    bool init(usl::ListType type);
    void load(usl::ListType type) override;
    void build();
    void showPlayer(usl::Player const& player);
};
