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

    enum class ListType { Classic = 0, Challenge = 1, Spam = 2 };
    constexpr std::array<ListType, 3> LIST_TYPES = { ListType::Classic, ListType::Challenge, ListType::Spam };

    const char* listKey(ListType type);
    const char* listName(ListType type);
    const char* listShortName(ListType type);
    const char* listIcon(ListType type);
    cocos2d::ccColor3B listColor(ListType type);
    cocos2d::ccColor3B listDarkColor(ListType type);
    std::optional<ListType> listFromKey(std::string_view key);

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
        std::array<int, 3> levelsByList {};
        std::array<int, 3> recordsByList {};
    };

    std::string categoryName(std::string_view category);
    cocos2d::ccColor3B categoryColor(std::string_view category);
    std::string formatPoints(double points);
    std::string timeAgo(int64_t millis);

    using Done = std::function<void(bool ok)>;

    /// Main-thread cache of everything the mod reads from the USL public API.
    class Store {
    public:
        static Store& get();

        void loadLevels(ListType type, Done done, bool force = false);
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
        Level const* byGdId(int gdId) const;

    private:
        struct Slot {
            bool loaded = false;
            bool loading = false;
            int64_t loadedAt = 0;
            std::vector<Done> waiting;
        };

        void fetch(std::string const& path, std::function<void(std::optional<matjson::Value>)> done);
        static void finish(Slot& slot, bool ok);
        void rebuildIndex();

        std::array<std::vector<Level>, 3> m_levels;
        std::array<std::vector<Player>, 3> m_players;
        std::array<std::vector<Change>, 3> m_changes;
        Stats m_stats;
        std::array<Slot, 3> m_levelSlots;
        std::array<Slot, 3> m_playerSlots;
        std::array<Slot, 3> m_changeSlots;
        Slot m_statsSlot;
        std::unordered_map<int, Level const*> m_byGdId;
    };
}
