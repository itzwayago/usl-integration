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

        Level parseLevel(matjson::Value const& v, ListType fallbackList) {
            Level level;
            level.slug = str(v, "slug");
            level.name = str(v, "name");
            level.creator = str(v, "creator");
            level.verifier = str(v, "verifier");
            level.category = str(v, "category");
            level.youtubeUrl = str(v, "youtubeUrl");
            level.list = listFromKey(str(v, "listType")).value_or(fallbackList);
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

    const char* listKey(ListType type) {
        switch (type) {
            case ListType::Challenge: return "challenge";
            case ListType::Spam: return "spam";
            default: return "classic";
        }
    }

    const char* listName(ListType type) {
        switch (type) {
            case ListType::Challenge: return "Challenge List";
            case ListType::Spam: return "Spam List";
            default: return "Classic List";
        }
    }

    const char* listShortName(ListType type) {
        switch (type) {
            case ListType::Challenge: return "Challenge";
            case ListType::Spam: return "Spam";
            default: return "Classic";
        }
    }

    const char* listIcon(ListType type) {
        switch (type) {
            case ListType::Challenge: return "list_challenge.png"_spr;
            case ListType::Spam: return "list_spam.png"_spr;
            default: return "list_classic.png"_spr;
        }
    }

    ccColor3B listColor(ListType type) {
        switch (type) {
            case ListType::Challenge: return { 255, 194, 61 };
            case ListType::Spam: return { 255, 107, 107 };
            default: return { 71, 176, 255 };
        }
    }

    ccColor3B listDarkColor(ListType type) {
        switch (type) {
            case ListType::Challenge: return { 120, 74, 18 };
            case ListType::Spam: return { 112, 30, 42 };
            default: return { 22, 58, 112 };
        }
    }

    std::optional<ListType> listFromKey(std::string_view key) {
        if (key == "classic") return ListType::Classic;
        if (key == "challenge") return ListType::Challenge;
        if (key == "spam") return ListType::Spam;
        return std::nullopt;
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

    void Store::loadLevels(ListType type, Done done, bool force) {
        auto idx = static_cast<size_t>(type);
        auto& slot = m_levelSlots[idx];
        if (!force && slot.loaded && nowMs() - slot.loadedAt < LEVELS_TTL_MS) {
            if (done) done(true);
            return;
        }
        slot.waiting.push_back(std::move(done));
        if (slot.loading) return;
        slot.loading = true;

        fetch(fmt::format("/levels?list={}", listKey(type)), [this, type, idx](std::optional<matjson::Value> data) {
            bool ok = data && data->isArray();
            if (ok) {
                std::vector<Level> parsed;
                for (auto const& item : *data) {
                    auto level = parseLevel(item, type);
                    if (level.rank > 0) parsed.push_back(std::move(level));
                }
                std::ranges::sort(parsed, {}, &Level::rank);
                m_levels[idx] = std::move(parsed);
                rebuildIndex();
            }
            finish(m_levelSlots[idx], ok);
        });
    }

    void Store::loadAllLevels(Done done) {
        auto remaining = std::make_shared<int>(3);
        auto allOk = std::make_shared<bool>(true);
        for (auto type : LIST_TYPES) {
            loadLevels(type, [remaining, allOk, done](bool ok) {
                *allOk = *allOk && ok;
                if (--*remaining == 0 && done) done(*allOk);
            });
        }
    }

    void Store::loadPlayers(ListType type, Done done) {
        auto idx = static_cast<size_t>(type);
        auto& slot = m_playerSlots[idx];
        if (slot.loaded && nowMs() - slot.loadedAt < SHORT_TTL_MS) {
            if (done) done(true);
            return;
        }
        slot.waiting.push_back(std::move(done));
        if (slot.loading) return;
        slot.loading = true;

        fetch(fmt::format("/leaderboards/players?list={}", listKey(type)), [this, idx](std::optional<matjson::Value> data) {
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
        auto& slot = m_changeSlots[idx];
        if (slot.loaded && nowMs() - slot.loadedAt < SHORT_TTL_MS) {
            if (done) done(true);
            return;
        }
        slot.waiting.push_back(std::move(done));
        if (slot.loading) return;
        slot.loading = true;

        fetch(fmt::format("/changes?list={}", listKey(type)), [this, idx](std::optional<matjson::Value> data) {
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
                m_changes[idx] = std::move(parsed);
            }
            finish(m_changeSlots[idx], ok);
        });
    }

    void Store::loadStats(Done done) {
        auto& slot = m_statsSlot;
        if (slot.loaded && nowMs() - slot.loadedAt < SHORT_TTL_MS) {
            if (done) done(true);
            return;
        }
        slot.waiting.push_back(std::move(done));
        if (slot.loading) return;
        slot.loading = true;

        fetch("/stats", [this](std::optional<matjson::Value> data) {
            bool ok = data && data->isObject();
            if (ok) {
                Stats s;
                s.users = integer(*data, "users");
                s.records = integer(*data, "approvedRecords");
                s.levels = integer(*data, "approvedLevels");
                if (data->contains("byList") && (*data)["byList"].isObject()) {
                    auto const& byList = (*data)["byList"];
                    for (auto type : LIST_TYPES) {
                        auto key = listKey(type);
                        if (!byList.contains(key)) continue;
                        s.levelsByList[static_cast<size_t>(type)] = integer(byList[key], "approvedLevels");
                        s.recordsByList[static_cast<size_t>(type)] = integer(byList[key], "approvedRecords");
                    }
                }
                m_stats = s;
            }
            finish(m_statsSlot, ok);
        });
    }

    void Store::loadLevel(std::string const& slug, std::function<void(std::optional<LevelDetails>)> done) {
        fetch(fmt::format("/levels/{}", slug), [done = std::move(done)](std::optional<matjson::Value> data) {
            if (!data || !data->isObject() || !data->contains("level")) {
                done(std::nullopt);
                return;
            }
            LevelDetails details;
            details.level = parseLevel((*data)["level"], ListType::Classic);
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
        for (auto const& list : m_levels) {
            for (auto const& level : list) {
                if (level.gdId > 0) m_byGdId.emplace(level.gdId, &level);
            }
        }
    }
}
