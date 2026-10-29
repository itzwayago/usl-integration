#pragma once

#include "ListPopup.hpp"

/// Recent placement changes for one list, like the site's changelog feed.
class USLChangesPopup : public USLListPopup {
public:
    static USLChangesPopup* create(usl::ListType type);

protected:
    bool init(usl::ListType type);
    void load(usl::ListType type) override;
    void build();
};
