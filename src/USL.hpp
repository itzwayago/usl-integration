#pragma once

#include <Geode/Geode.hpp>
#include <array>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace usl {
    constexpr const char* SITE = "https://ultimateshittylist.fun";
    constexpr const char* API = "https://ultimateshittylist.fun/api";

    /// Every ranking USL keeps. Spam is three separate lists with their own placements and points.
    enum class ListType { Classic = 0, Challenge = 1, Scl = 2, Controlled = 3, Unlimited = 4 };
    constexpr std::array<ListType, 5> LIST_TYPES = {
        ListType::Classic, ListType::Challenge, ListType::Scl, ListType::Controlled, ListType::Unlimited
    };
    constexpr std::array<ListType, 3> SPAM_LISTS = { ListType::Scl, ListType::Controlled, ListType::Unlimited };
    constexpr size_t LIST_COUNT = LIST_TYPES.size();

    bool isSpam(ListType type);
    /// `list=...` (and `spam=...` for spam lists) for the public API.
    std::string scopeQuery(ListType type);
    const char* listKey(ListType type);
    /// Lowercase id for node IDs and links: classic, challenge, scl, controlled, unlimited.
    const char* listId(ListType type);
    const char* listName(ListType type);
    const char* listShortName(ListType type);
    /// Three-four letters for tight spots: SCL, CTRL, UNL (Classic and Challenge get their short name).
    const char* listAbbr(ListType type);
    const char* listIcon(ListType type);
    cocos2d::ccColor3B listColor(ListType type);

    /// Demon difficulty staff set on the site. Only Classic levels carry one for now.
    enum class Difficulty { None, Easy, Medium, Hard, Insane, Extreme };
    /// Rate the level got from the Rate Advisors, shown as the GD feature glow.
    enum class Quality { None, Featured, Epic, Legendary, Mythic };

    const char* difficultyName(Difficulty difficulty);
    const char* qualityName(Quality quality);

    struct Level {
        std::string slug;
        std::string name;
        std::string creator;
        std::string verifier;
        std::string category;
        std::string youtubeUrl;
        ListType list = ListType::Classic;
        int rank = 0;
        int listPercent = 100;
        double points = 0.0;
        int gdId = 0;
        Difficulty difficulty = Difficulty::None;
        Quality quality = Quality::None;
    };

    struct Record {
        std::string username;
        std::string countryCode;
        int progress = 0;
        double score = 0.0;
        std::string videoUrl;
    };

    struct LevelDetails {
        Level level;
        std::vector<Record> records;
    };

    struct Player {
        int rank = 0;
        std::string username;
        std::string slug;
        std::string countryCode;
        double score = 0.0;
        int records = 0;
        int verified = 0;
        std::string hardestName;
        int hardestRank = 0;
    };

    struct Change {
        std::string kind;
        std::string name;
        std::string slug;
        int fromRank = 0;
        int toRank = 0;
        int64_t capturedAt = 0;
    };

    struct Stats {
        int users = 0;
        int records = 0;
        int levels = 0;
        int classicRecords = 0;
        int challengeRecords = 0;
        int spamRecords = 0;
    };

    std::string categoryName(std::string_view category);
    cocos2d::ccColor3B categoryColor(std::string_view category);
    std::string formatPoints(double points);
    std::string timeAgo(int64_t millis);

    /// The three switches on the mod's settings page, read where they are used.
    namespace settings {
        bool placements();
        bool points();
    }

    using Done = std::function<void(bool ok)>;

    /// Main-thread cache of everything the mod reads from the USL public API.
    class Store {
    public:
        static Store& get();

        void loadLevels(ListType type, Done done, bool force = false);
        /// All five lists plus the rate data, so placements and fake rates work everywhere.
        void loadAllLevels(Done done);
        void loadPlayers(ListType type, Done done);
        void loadChanges(ListType type, Done done);
        void loadStats(Done done);
        void loadLevel(std::string const& slug, std::function<void(std::optional<LevelDetails>)> done);

        std::vector<Level> const& levels(ListType type) const;
        std::vector<Player> const& players(ListType type) const;
        std::vector<Change> const& changes(ListType type) const;
        Stats const& stats() const;
        bool hasLevels(ListType type) const;

        /// Level placed on any list with this GD id, if the lists are loaded.
        /// The pointer is only valid until the next list refresh; copy what you keep.
        Level const* byGdId(int gdId) const;

    private:
        struct Slot {
            bool loaded = false;
            bool loading = false;
            int64_t loadedAt = 0;
            int64_t failedAt = 0;
            std::vector<Done> waiting;
        };
        struct Rate {
            Difficulty difficulty = Difficulty::None;
            Quality quality = Quality::None;
        };

        void fetch(std::string const& path, std::function<void(std::optional<matjson::Value>)> done);
        void loadRates(Done done);
        static bool claim(Slot& slot, Done& done, int64_t ttl, bool force = false);
        static void finish(Slot& slot, bool ok);
        void applyRates(std::vector<Level>& levels) const;
        void rebuildIndex();

        std::array<std::vector<Level>, LIST_COUNT> m_levels;
        std::array<std::vector<Player>, LIST_COUNT> m_players;
        std::array<std::vector<Change>, LIST_COUNT> m_changes;
        Stats m_stats;
        std::array<Slot, LIST_COUNT> m_levelSlots;
        std::array<Slot, LIST_COUNT> m_playerSlots;
        std::array<Slot, LIST_COUNT> m_changeSlots;
        Slot m_statsSlot;
        Slot m_rateSlot;
        std::unordered_map<std::string, Rate> m_rates;
        std::unordered_map<int, Level const*> m_byGdId;
    };
}
