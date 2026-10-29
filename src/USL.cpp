#include "USL.hpp"
#include <Geode/utils/web.hpp>
#include <chrono>

using namespace geode::prelude;

namespace usl {
    namespace {
        constexpr int64_t LEVELS_TTL_MS = 30 * 60 * 1000;
        constexpr int64_t SHORT_TTL_MS = 2 * 60 * 1000;

        int64_t nowMs() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
        }

        std::string str(matjson::Value const& obj, std::string_view key) {
            if (!obj.contains(key)) return "";
            auto const& v = obj[key];
            if (v.isString()) return v.asString().unwrapOrDefault();
            if (v.isNumber()) return v.dump();
            return "";
        }

        double num(matjson::Value const& obj, std::string_view key, double fallback = 0.0) {
            if (!obj.contains(key)) return fallback;
            auto const& v = obj[key];
            if (v.isNumber()) return v.asDouble().unwrapOr(fallback);
            if (v.isString()) return numFromString<double>(v.asString().unwrapOrDefault()).unwrapOr(fallback);
            return fallback;
        }

        int integer(matjson::Value const& obj, std::string_view key, int fallback = 0) {
            return static_cast<int>(num(obj, key, fallback));
        }

        std::optional<ListType> listFromKeys(std::string_view list, std::string_view spam) {
            if (list == "classic") return ListType::Classic;
            if (list == "challenge") return ListType::Challenge;
            if (list != "spam") return std::nullopt;
            if (spam == "controlled") return ListType::Controlled;
            if (spam == "unlimited") return ListType::Unlimited;
            return ListType::Scl;
        }

        Difficulty difficultyFromKey(std::string_view key) {
            if (key == "easy_demon") return Difficulty::Easy;
            if (key == "medium_demon") return Difficulty::Medium;
            if (key == "hard_demon") return Difficulty::Hard;
            if (key == "insane_demon") return Difficulty::Insane;
            if (key == "extreme_demon") return Difficulty::Extreme;
            return Difficulty::None;
        }

        Quality qualityFromKey(std::string_view key) {
            if (key == "featured") return Quality::Featured;
            if (key == "epic") return Quality::Epic;
            if (key == "legendary") return Quality::Legendary;
            if (key == "mythic") return Quality::Mythic;
            return Quality::None;
        }

        Level parseLevel(matjson::Value const& v, ListType fallbackList) {
            Level level;
            level.slug = str(v, "slug");
            level.name = str(v, "name");
            level.creator = str(v, "creator");
            level.verifier = str(v, "verifier");
            level.category = str(v, "category");
            level.youtubeUrl = str(v, "youtubeUrl");
            level.list = listFromKeys(str(v, "listType"), str(v, "spamList")).value_or(fallbackList);
            level.rank = integer(v, "rank");
            level.listPercent = integer(v, "listPercent", 100);
            level.points = num(v, "points");
            level.gdId = integer(v, "gdId");
            return level;
        }

        std::string url(std::string_view path) {
            return fmt::format("{}{}", API, path);
        }
    }

    bool isSpam(ListType type) {
        return type == ListType::Scl || type == ListType::Controlled || type == ListType::Unlimited;
    }

    const char* listKey(ListType type) {
        switch (type) {
            case ListType::Classic: return "classic";
            case ListType::Challenge: return "challenge";
            default: return "spam";
        }
    }

    const char* listId(ListType type) {
        switch (type) {
            case ListType::Challenge: return "challenge";
            case ListType::Scl: return "scl";
            case ListType::Controlled: return "controlled";
            case ListType::Unlimited: return "unlimited";
            default: return "classic";
        }
    }

    std::string scopeQuery(ListType type) {
        switch (type) {
            case ListType::Scl: return "list=spam&spam=scl";
            case ListType::Controlled: return "list=spam&spam=controlled";
            case ListType::Unlimited: return "list=spam&spam=unlimited";
            default: return fmt::format("list={}", listKey(type));
        }
    }

    const char* listName(ListType type) {
        switch (type) {
            case ListType::Challenge: return "Challenge List";
            case ListType::Scl: return "SCL Spam List";
            case ListType::Controlled: return "Controlled Spam List";
            case ListType::Unlimited: return "Unlimited Spam List";
            default: return "Classic List";
        }
    }

    const char* listShortName(ListType type) {
        switch (type) {
            case ListType::Challenge: return "Challenge";
            case ListType::Scl: return "SCL";
            case ListType::Controlled: return "Controlled";
            case ListType::Unlimited: return "Unlimited";
            default: return "Classic";
        }
    }

    const char* listAbbr(ListType type) {
        switch (type) {
            case ListType::Controlled: return "CTRL";
            case ListType::Unlimited: return "UNL";
            default: return listShortName(type);
        }
    }

    const char* listIcon(ListType type) {
        switch (type) {
            case ListType::Classic: return "list_classic.png"_spr;
            case ListType::Challenge: return "list_challenge.png"_spr;
            default: return "list_spam.png"_spr;
        }
    }

    ccColor3B listColor(ListType type) {
        switch (type) {
            case ListType::Challenge: return { 255, 194, 61 };
            case ListType::Scl: return { 255, 107, 107 };
            case ListType::Controlled: return { 255, 150, 80 };
            case ListType::Unlimited: return { 214, 120, 255 };
            default: return { 71, 176, 255 };
        }
    }

    ccColor3B listDarkColor(ListType type) {
        switch (type) {
            case ListType::Challenge: return { 120, 74, 18 };
            case ListType::Scl: return { 112, 30, 42 };
            case ListType::Controlled: return { 112, 52, 22 };
            case ListType::Unlimited: return { 74, 34, 112 };
            default: return { 22, 58, 112 };
        }
    }

    const char* difficultyName(Difficulty difficulty) {
        switch (difficulty) {
            case Difficulty::Easy: return "Easy Demon";
            case Difficulty::Medium: return "Medium Demon";
            case Difficulty::Hard: return "Hard Demon";
            case Difficulty::Insane: return "Insane Demon";
            case Difficulty::Extreme: return "Extreme Demon";
            default: return "";
        }
    }

    const char* qualityName(Quality quality) {
        switch (quality) {
            case Quality::Featured: return "Featured";
            case Quality::Epic: return "Epic";
            case Quality::Legendary: return "Legendary";
            case Quality::Mythic: return "Mythic";
            default: return "";
        }
    }

    std::string categoryName(std::string_view category) {
        if (category == "main") return "Main";
        if (category == "extended") return "Extended";
        if (category == "advanced") return "Advanced";
        if (category == "unbounded") return "Unbounded";
        return std::string(category);
    }

    ccColor3B categoryColor(std::string_view category) {
        if (category == "main") return { 255, 214, 92 };
        if (category == "extended") return { 140, 220, 255 };
        if (category == "advanced") return { 200, 160, 255 };
        return { 190, 190, 190 };
    }

    std::string formatPoints(double points) {
        auto text = fmt::format("{:.2f}", points);
        while (!text.empty() && text.back() == '0') text.pop_back();
        if (!text.empty() && text.back() == '.') text.pop_back();
        return text;
    }

    std::string timeAgo(int64_t millis) {
        auto diff = std::max<int64_t>(0, (nowMs() - millis) / 1000);
        if (diff < 60) return "just now";
        if (diff < 3600) return fmt::format("{}m ago", diff / 60);
        if (diff < 86400) return fmt::format("{}h ago", diff / 3600);
        if (diff < 86400 * 30) return fmt::format("{}d ago", diff / 86400);
        return fmt::format("{}mo ago", diff / (86400 * 30));
    }

    Store& Store::get() {
        static Store store;
        return store;
    }

    namespace {
        using FetchDone = std::function<void(std::optional<matjson::Value>)>;

        // Runs on the async runtime; the result is handed back on the main thread.
        // Parameters are taken by value so they live in the coroutine frame.
        arc::Future<void> runFetch(std::string target, FetchDone done) {
            auto req = web::WebRequest()
                .userAgent(fmt::format("USL-Geode/{}", Mod::get()->getVersion().toVString()))
                .timeout(std::chrono::seconds(20));
            auto res = co_await req.get(target);

            std::optional<matjson::Value> data;
            if (!res.ok()) {
                log::warn("USL request {} failed with HTTP {}", target, res.code());
            } else if (auto json = res.json(); json.isErr() || !json.unwrap().contains("data")) {
                log::warn("USL request {} returned unexpected JSON", target);
            } else {
                data = json.unwrap()["data"];
            }
            queueInMainThread([done = std::move(done), data = std::move(data)]() mutable {
                done(std::move(data));
            });
        }
    }

    void Store::fetch(std::string const& path, std::function<void(std::optional<matjson::Value>)> done) {
        async::spawn(runFetch(url(path), std::move(done)));
    }

    // Answers straight away from a fresh cache, otherwise queues `done`.
    // Returns true when the caller has to start the request.
    bool Store::claim(Slot& slot, Done& done, int64_t ttl, bool force) {
        if (!force && slot.loaded && nowMs() - slot.loadedAt < ttl) {
            if (done) done(true);
            return false;
        }
        slot.waiting.push_back(std::move(done));
        if (slot.loading) return false;
        slot.loading = true;
        return true;
    }

    void Store::finish(Slot& slot, bool ok) {
        slot.loading = false;
        if (ok) {
            slot.loaded = true;
            slot.loadedAt = nowMs();
        }
        auto waiting = std::move(slot.waiting);
        slot.waiting.clear();
        for (auto& cb : waiting) {
            if (cb) cb(ok);
        }
    }

    void Store::applyRates(std::vector<Level>& levels) const {
        for (auto& level : levels) {
            auto it = m_rates.find(level.slug);
            if (it == m_rates.end()) continue;
            level.difficulty = it->second.difficulty;
            level.quality = it->second.quality;
        }
    }

    void Store::loadLevels(ListType type, Done done, bool force) {
        auto idx = static_cast<size_t>(type);
        if (!claim(m_levelSlots[idx], done, LEVELS_TTL_MS, force)) return;

        fetch(fmt::format("/levels?{}", scopeQuery(type)), [this, type, idx](std::optional<matjson::Value> data) {
            bool ok = data && data->isArray();
            if (ok) {
                std::vector<Level> parsed;
                for (auto const& item : *data) {
                    auto level = parseLevel(item, type);
                    // Upcoming levels have no placement yet.
                    if (level.rank > 0) parsed.push_back(std::move(level));
                }
                std::ranges::sort(parsed, {}, &Level::rank);
                applyRates(parsed);
                m_levels[idx] = std::move(parsed);
                rebuildIndex();
            }
            finish(m_levelSlots[idx], ok);
        });
    }

    void Store::loadRates(Done done) {
        if (!claim(m_rateSlot, done, LEVELS_TTL_MS)) return;

        fetch("/community/levels", [this](std::optional<matjson::Value> data) {
            bool ok = data && data->isObject() && data->contains("levels") && (*data)["levels"].isArray();
            if (ok) {
                m_rates.clear();
                for (auto const& item : (*data)["levels"]) {
                    Rate rate { difficultyFromKey(str(item, "difficulty")), qualityFromKey(str(item, "quality")) };
                    if (rate.difficulty != Difficulty::None || rate.quality != Quality::None) {
                        m_rates.emplace(str(item, "slug"), rate);
                    }
                }
                for (auto& levels : m_levels) applyRates(levels);
            }
            finish(m_rateSlot, ok);
        });
    }

    void Store::loadAllLevels(Done done) {
        auto remaining = std::make_shared<size_t>(LIST_COUNT + 1);
        auto allOk = std::make_shared<bool>(true);
        auto step = [remaining, allOk, done](bool ok) {
            *allOk = *allOk && ok;
            if (--*remaining == 0 && done) done(*allOk);
        };
        loadRates(step);
        for (auto type : LIST_TYPES) loadLevels(type, step);
    }

    void Store::loadPlayers(ListType type, Done done) {
        auto idx = static_cast<size_t>(type);
        if (!claim(m_playerSlots[idx], done, SHORT_TTL_MS)) return;

        fetch(fmt::format("/leaderboards/players?{}", scopeQuery(type)), [this, idx](std::optional<matjson::Value> data) {
            bool ok = data && data->isArray();
            if (ok) {
                std::vector<Player> parsed;
                for (auto const& item : *data) {
                    Player p;
                    p.rank = integer(item, "rank");
                    p.username = str(item, "username");
                    p.slug = str(item, "slug");
                    p.countryCode = str(item, "countryCode");
                    p.score = num(item, "score");
                    p.records = integer(item, "records");
                    p.verified = integer(item, "verifiedLevels");
                    if (item.contains("hardestLevel") && item["hardestLevel"].isObject()) {
                        p.hardestName = str(item["hardestLevel"], "name");
                        p.hardestRank = integer(item["hardestLevel"], "rank");
                    }
                    parsed.push_back(std::move(p));
                }
                m_players[idx] = std::move(parsed);
            }
            finish(m_playerSlots[idx], ok);
        });
    }

    void Store::loadChanges(ListType type, Done done) {
        auto idx = static_cast<size_t>(type);
        if (!claim(m_changeSlots[idx], done, SHORT_TTL_MS)) return;

        fetch(fmt::format("/changes?{}", scopeQuery(type)), [this, idx](std::optional<matjson::Value> data) {
            bool ok = data && data->isArray();
            if (ok) {
                std::vector<Change> parsed;
                for (auto const& item : *data) {
                    Change c;
                    c.kind = str(item, "kind");
                    c.name = str(item, "name");
                    c.slug = str(item, "slug");
                    c.fromRank = integer(item, "fromRank");
                    c.toRank = integer(item, "toRank");
                    c.capturedAt = static_cast<int64_t>(num(item, "capturedAt"));
                    parsed.push_back(std::move(c));
                }
                // Newest first, whatever order the API answers in.
                std::ranges::stable_sort(parsed, std::greater<>{}, &Change::capturedAt);
                m_changes[idx] = std::move(parsed);
            }
            finish(m_changeSlots[idx], ok);
        });
    }

    void Store::loadStats(Done done) {
        if (!claim(m_statsSlot, done, SHORT_TTL_MS)) return;

        fetch("/stats", [this](std::optional<matjson::Value> data) {
            bool ok = data && data->isObject();
            if (ok) {
                Stats s;
                s.users = integer(*data, "users");
                s.records = integer(*data, "approvedRecords");
                s.levels = integer(*data, "approvedLevels");
                if (data->contains("byList") && (*data)["byList"].isObject()) {
                    auto const& byList = (*data)["byList"];
                    auto records = [&](const char* key) {
                        return byList.contains(key) ? integer(byList[key], "approvedRecords") : 0;
                    };
                    s.classicRecords = records("classic");
                    s.challengeRecords = records("challenge");
                    s.spamRecords = records("spam");
                }
                m_stats = s;
            }
            finish(m_statsSlot, ok);
        });
    }

    void Store::loadLevel(std::string const& slug, std::function<void(std::optional<LevelDetails>)> done) {
        fetch(fmt::format("/levels/{}", slug), [this, done = std::move(done)](std::optional<matjson::Value> data) {
            if (!data || !data->isObject() || !data->contains("level")) {
                done(std::nullopt);
                return;
            }
            LevelDetails details;
            details.level = parseLevel((*data)["level"], ListType::Classic);
            if (auto it = m_rates.find(details.level.slug); it != m_rates.end()) {
                details.level.difficulty = it->second.difficulty;
                details.level.quality = it->second.quality;
            }
            if (data->contains("records") && (*data)["records"].isArray()) {
                for (auto const& item : (*data)["records"]) {
                    Record r;
                    r.username = str(item, "username");
                    r.countryCode = str(item, "countryCode");
                    r.progress = integer(item, "progress");
                    r.score = num(item, "score");
                    r.videoUrl = str(item, "videoUrl");
                    details.records.push_back(std::move(r));
                }
            }
            done(std::move(details));
        });
    }

    std::vector<Level> const& Store::levels(ListType type) const { return m_levels[static_cast<size_t>(type)]; }
    std::vector<Player> const& Store::players(ListType type) const { return m_players[static_cast<size_t>(type)]; }
    std::vector<Change> const& Store::changes(ListType type) const { return m_changes[static_cast<size_t>(type)]; }
    Stats const& Store::stats() const { return m_stats; }
    bool Store::hasLevels(ListType type) const { return m_levelSlots[static_cast<size_t>(type)].loaded; }

    Level const* Store::byGdId(int gdId) const {
        auto it = m_byGdId.find(gdId);
        return it == m_byGdId.end() ? nullptr : it->second;
    }

    void Store::rebuildIndex() {
        m_byGdId.clear();
        // LIST_TYPES order decides which placement a level shows when it sits on two lists.
        for (auto const& list : m_levels) {
            for (auto const& level : list) {
                if (level.gdId > 0) m_byGdId.emplace(level.gdId, &level);
            }
        }
    }
}
