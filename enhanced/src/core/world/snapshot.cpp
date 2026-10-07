#include "core/world/snapshot.h"

#include "core/data/gamedata.h"
#include "core/world/game.h"

#include "snapshot_id.h"

#include <array>
#include <cstring>
#include <deque>
#include <format>
#include <string_view>
#include <type_traits>

namespace th
{

namespace
{

constexpr std::array<uint8_t, 8> kMagic = {'T', 'H', 'S', 'N', 'A', 'P', 0, 1};

// The structures copied as raw bytes, whose sizes go into the build id.
template <typename... T> constexpr uint64_t layout_hash()
{
    uint64_t h = 0xcbf29ce484222325ull;
    for (size_t size : {sizeof(T)...})
        h = (h ^ size) * 0x100000001b3ull;
    return h;
}
constexpr uint64_t kLayoutHash =
    layout_hash<Thing, RoomInfo, MapCell, ObjectPlacing, BuildState, Finance, Advice, ArrivalState, TownLand,
                HospitalSettings, StaffMarket, Disasters, Event, Date, World::Pager, World::PlayerCall,
                World::ThingPager, World::Emergency, World::Epidemic, PanelPopups, AdCounts, Game::ChoiceScreen,
                Balance::Gbv, Balance::AwardsTrophies, Research, YearEnd, void *, bool, int>();

// Fields of World, Game and the structures they hold that are not copied
// as a whole must each be listed below; on the build the code is written on,
// these sizes catch one added without being listed.
#if defined(__GLIBCXX__) && defined(__x86_64__)
static_assert(sizeof(World) == 139560, "World changed: list its new fields in snapshot.cpp");
static_assert(sizeof(Building) == 224, "Building changed: list its new fields in snapshot.cpp");
static_assert(sizeof(Advisor) == 1160, "Advisor changed: list its new fields in snapshot.cpp");
static_assert(sizeof(Balance) == 3776, "Balance changed: list its new fields in snapshot.cpp");
static_assert(sizeof(EventLog) == 96, "EventLog changed: list its new fields in snapshot.cpp");
static_assert(sizeof(Game) == 317688, "Game changed: list its new fields in snapshot.cpp");
#endif

class Writer
{
  public:
    explicit Writer(std::vector<uint8_t> &out) : out_(out) {}

    template <typename T> void operator()(const T &v)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        bytes(&v, sizeof v);
    }
    template <typename T> void operator()(const std::vector<T> &v)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        (*this)(uint32_t(v.size()));
        bytes(v.data(), v.size() * sizeof(T));
    }
    void operator()(const std::vector<bool> &v)
    {
        (*this)(uint32_t(v.size()));
        for (bool b : v)
            (*this)(uint8_t(b));
    }
    void operator()(const std::string &s)
    {
        (*this)(uint32_t(s.size()));
        bytes(s.data(), s.size());
    }
    void operator()(const std::vector<std::string> &v)
    {
        (*this)(uint32_t(v.size()));
        for (const std::string &s : v)
            (*this)(s);
    }
    void operator()(const std::deque<TraceEvent> &events)
    {
        (*this)(uint32_t(events.size()));
        for (const TraceEvent &e : events) {
            (*this)(e.turn);
            (*this)(e.text);
        }
    }

  private:
    void bytes(const void *p, size_t n)
    {
        const auto *b = static_cast<const uint8_t *>(p);
        out_.insert(out_.end(), b, b + n);
    }
    std::vector<uint8_t> &out_;
};

class Reader
{
  public:
    explicit Reader(std::span<const uint8_t> in) : in_(in) {}
    bool ok() const { return ok_; }
    bool at_end() const { return pos_ == in_.size(); }

    template <typename T> void operator()(T &v)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        bytes(&v, sizeof v);
    }
    template <typename T> void operator()(std::vector<T> &v)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        size_t n = count(sizeof(T));
        v.resize(n);
        bytes(v.data(), n * sizeof(T));
    }
    void operator()(std::vector<bool> &v)
    {
        size_t n = count(1);
        v.assign(n, false);
        for (size_t i = 0; i < n; i++) {
            uint8_t b = 0;
            (*this)(b);
            v[i] = b != 0;
        }
    }
    void operator()(std::string &s)
    {
        size_t n = count(1);
        s.resize(n);
        bytes(s.data(), n);
    }
    void operator()(std::vector<std::string> &v)
    {
        v.resize(count(4));
        for (std::string &s : v)
            (*this)(s);
    }
    void operator()(std::deque<TraceEvent> &events)
    {
        events.resize(count(8));
        for (TraceEvent &e : events) {
            (*this)(e.turn);
            (*this)(e.text);
        }
    }

  private:
    // A count of entries of at least `min_size` bytes each, 0 if more than
    // what is left could hold.
    size_t count(size_t min_size)
    {
        uint32_t n = 0;
        (*this)(n);
        if (n > (in_.size() - pos_) / min_size) {
            ok_ = false;
            return 0;
        }
        return n;
    }
    void bytes(void *p, size_t n)
    {
        if (!ok_ || n > in_.size() - pos_) {
            ok_ = false;
            return;
        }
        std::memcpy(p, in_.data() + pos_, n);
        pos_ += n;
    }
    std::span<const uint8_t> in_;
    size_t pos_ = 0;
    bool ok_ = true;
};

// Each fields() passes every field of its structure to `io`, a Writer with
// const structures or a Reader; World's pointers are left out.
template <typename IO, typename B> void balance_fields(IO &io, B &b)
{
    io(b.expertise);
    io(b.objects);
    io(b.towns);
    io(b.rooms);
    io(b.staff);
    io(b.gbv);
    io(b.win_criteria);
    io(b.lose_criteria);
    io(b.net_criteria);
    io(b.popn);
    io(b.staff_levels);
    io(b.emergency_control);
    io(b.quake_control);
    io(b.computer);
    io(b.awards_trophies);
    io(b.non_visuals);
    io(b.visuals);
    io(b.visuals_available);
    io(b.illness_kinds);
    io(b.visuals_total);
    io(b.non_visuals_total);
}

template <typename IO, typename B> void building_fields(IO &io, B &b)
{
    io(b.build);
    io(b.mouse);
    io(b.door_invalid);
    io(b.door_shown);
    io(b.door_piece_invalid);
    io(b.pointer);
    io(b.saved_buy_count);
    io(b.shop_adjust);
    io(b.last_built_room);
    io(b.sounds);
}

template <typename IO, typename W> void world_fields(IO &io, W &w)
{
    io(w.map);
    io(w.things);
    io(w.first_of_kind);
    io(w.seed);
    io(w.path_finder);
    io(w.entrances);
    io(w.placing);
    io(w.rooms);
    io(w.queue_near_dist);
    io(w.object_near_dist);
    io(w.rand_pct);
    io(w.shake_x);
    io(w.shake_y);
    io(w.cure_available);
    io(w.ins_control);
    io(w.patients_left_count);
    io(w.bench_shortage);
    io(w.plants);
    io(w.thirsty_plants);
    io(w.plant_health_sum);
    io(w.advisor_events);
    io(w.announce_stuck);
    io(w.unpaid_treatment);
    io(w.too_expensive);
    io(w.drinks_income);
    io(w.held_staff);
    io(w.held_staff_tile);
    io(w.idle_nurses);
    building_fields(io, w.building);
    io(w.editing_room);
    io(w.pagers);
    io(w.room_of_type);
    io(w.calls);
    io(w.next_call_id);
    io(w.first_call);
    io(w.popups);
    io(w.counts);
    io(w.litter_count);
    io(w.repair_pager);
    io(w.plant_pager);
    io(w.emergency);
    io(w.mayor);
    io(w.epidemic);
    io(w.epidemic_sounds);
    io(w.mishaps);
    io(w.machine_exploded);
    io(w.machine_sounds);
    io(w.machine_announcements);
    io(w.research);
    io(w.announce);
}

template <typename IO, typename A> void advisor_fields(IO &io, A &a)
{
    io(a.advice_id);
    io(a.forced_advice_id);
    io(a.mouse_advice_id);
    io(a.anim_frame);
    io(a.anim_state);
    io(a.state_advanced);
    io(a.time_left);
    io(a.text);
    io(a.flags);
    io(a.tutorial_highlight);
    io(a.line_count);
    io(a.extra_segments);
    io(a.rewrap);
    io(a.no_pieces);
    io(a.box_left);
    io(a.event_flags);
    io(a.heli_landed);
    io(a.enabled);
    io(a.given);
    io(a.dismiss);
    io(a.tutorial_step);
    io(a.tutorial_shown);
    io(a.advice);
    io(a.drawn);
    io(a.drawn_frame);
    io(a.bubble);
}

// Game's public fields; its private ones are the friends' below.
template <typename IO, typename G> void game_fields(IO &io, G &g)
{
    world_fields(io, g.world);
    io(g.date);
    io(g.turn_count);
    io(g.paused);
    io(g.opening_countdown);
    io(g.open_countdown);
    io(g.open);
    io(g.balance);
    io(g.finance);
    advisor_fields(io, g.advisor);
    io(g.arrivals);
    io(g.land);
    io(g.hospital);
    io(g.progress);
    io(g.staff_market);
    io(g.disasters);
    io(g.event);
    io(g.choice_screen);
    io(g.command_log);
    io(g.events.enabled);
    io(g.events.rand);
    io(g.events.limit);
    io(g.events.events);
    io(g.new_patients);
}

} // namespace

uint64_t snapshot_build_id()
{
    return TH_SNAPSHOT_SOURCE_HASH ^ kLayoutHash;
}

bool save_snapshot(const Game &game, std::vector<uint8_t> &out)
{
    if (!game.level_)
        return false;
    out.clear();
    Writer w(out);
    w(kMagic);
    w(snapshot_build_id());
    w(int32_t(game.level_number_));
    game_fields(w, game);
    balance_fields(w, game.settings_);
    w(game.walkers_);
    w(game.logged_inputs_);
    w(game.inputs_logged_);
    return true;
}

bool load_snapshot(Game &game, std::span<const uint8_t> data, const GameData &gamedata, std::string *error)
{
    auto fail = [&](std::string why) {
        if (error)
            *error = std::move(why);
        return false;
    };
    Reader r(data);
    std::array<uint8_t, 8> magic{};
    uint64_t build = 0;
    int32_t level_number = 0;
    r(magic);
    r(build);
    r(level_number);
    if (!r.ok() || magic != kMagic)
        return fail("not a snapshot");
    if (build != snapshot_build_id())
        return fail(std::format("snapshot of build {:016x}, this is build {:016x}: load it with the build that "
                                "wrote it",
                                build, snapshot_build_id()));
    std::string level_error;
    auto level = gamedata.load_level(level_number, &level_error);
    if (!level)
        return fail(std::format("snapshot of level {}: {}", level_number, level_error));

    game_fields(r, game);
    balance_fields(r, game.settings_);
    r(game.walkers_);
    r(game.logged_inputs_);
    r(game.inputs_logged_);
    if (!r.ok() || !r.at_end() || game.world.things.size() != size_t(kThingCount) ||
        game.walkers_.size() != size_t(kThingCount))
        return fail("snapshot cut short or damaged");
    game.level_ = std::make_unique<Level>(*level);
    game.level_number_ = level_number;
    game.anims_ = &gamedata.anims();
    game.complexes_ = &gamedata.person_complexes();
    game.staff_complexes_ = &gamedata.staff_complexes();
    game.connect_world();
    return true;
}

} // namespace th
