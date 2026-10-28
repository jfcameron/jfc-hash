// © Joseph Cameron - MIT License

#include <jfc/catch.hpp>

#include <jfc/hash.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <string>
#include <string_view>

using namespace jfc::hash;

static_assert(std::uniform_random_bit_generator<splitmix64>);
static_assert(mix64(0) == 0xe220a8397b1dcdafull);
static_assert(fnv1a64{}.add("foobar").value() == 0x85944171f73967e8ull);
static_assert(values(1, 2, 3) == 0x6c650e5fbf57ac44ull);

static_assert([] {
    splitmix64 random(1234567);

    random();
    random();

    return random();
}() == 9817491932198370423ull);

TEST_CASE("splitmix64 gives the published sequence", "[splitmix64]") {
    splitmix64 random(1234567);

    CHECK(random() == 6457827717110365317ull);
    CHECK(random() == 3203168211198807973ull);
    CHECK(random() == 9817491932198370423ull);
    CHECK(random() == 4593380528125082431ull);
    CHECK(random() == 16408922859458223821ull);
}

TEST_CASE("mix64 is splitmix64's output for a state, and a generator reseeded from state() carries on", "[mix64]") {
    splitmix64 random(42);

    CHECK(random() == mix64(42));

    random();

    splitmix64 carried(random.state());

    CHECK(carried() == random());
    CHECK(carried() == random());
}

TEST_CASE("fnv1a64 gives the published values", "[fnv1a64]") {
    CHECK(fnv1a64{}.value() == 0xcbf29ce484222325ull);
    CHECK(fnv1a64{}.add("a").value() == 0xaf63dc4c8601ec8cull);
    CHECK(fnv1a64{}.add("foobar").value() == 0x85944171f73967e8ull);
}

TEST_CASE("fnv1a64 takes pieces with nothing between them", "[fnv1a64]") {
    CHECK(fnv1a64{}.add("ab").add("c").value() == fnv1a64{}.add("abc").value());
    CHECK(fnv1a64{}.add("ab").add(std::uint8_t{0}).add("c").value()
        != fnv1a64{}.add("a").add(std::uint8_t{0}).add("bc").value());

    const std::array bytes{std::byte{'a'}, std::byte{'b'}, std::byte{'c'}};

    CHECK(fnv1a64{}.add(bytes).value() == fnv1a64{}.add(std::string("abc")).value());
}

TEST_CASE("fnv1a64 takes an integer's bytes little endian, at its own width", "[fnv1a64]") {
    const std::array little{std::byte{4}, std::byte{3}, std::byte{2}, std::byte{1}};

    CHECK(fnv1a64{}.add(std::uint32_t{0x01020304}).value() == 0xb345225e3644edb5ull);
    CHECK(fnv1a64{}.add(std::uint32_t{0x01020304}).value() == fnv1a64{}.add(little).value());
    CHECK(fnv1a64{}.add(std::int64_t{-2}).value() == 0xfc1a35225397861cull);
    CHECK(fnv1a64{}.add(std::int32_t{7}).value() != fnv1a64{}.add(std::int64_t{7}).value());
}

TEST_CASE("values hashes integers in order, each at its own width", "[values]") {
    CHECK(values() == GOLDEN);
    CHECK(values(1, 2, 3) == 0x6c650e5fbf57ac44ull);
    CHECK(values(3, 2, 1) == 0x56582478affeb1baull);
    CHECK(values(-1, 0, 7) == 0x0d2b8835d23cc9a6ull);
    CHECK(values(-1) == 0x59627b931ee7de0full);
    CHECK(values(std::int64_t{-1}) == 0xa64a4e47b26b306full);
    CHECK(values(5) == combine(GOLDEN, mix64(5)));
}

TEST_CASE("values tells neighbouring cells apart", "[values]") {
    std::set<std::uint64_t> seen;

    for (int x = -32; x < 32; ++x) for (int z = -32; z < 32; ++z) seen.insert(values(x, z));

    CHECK(seen.size() == 64 * 64);
}

TEST_CASE("unit_float and unit_double stay below one", "[unit]") {
    constexpr auto MOST = std::numeric_limits<std::uint64_t>::max();

    CHECK(unit_float(0) == 0.0f);
    CHECK(unit_float(MOST) < 1.0f);
    CHECK(unit_float(MOST) == 1.0f - 0x1.0p-24f);
    CHECK(unit_double(0) == 0.0);
    CHECK(unit_double(MOST) < 1.0);
    CHECK(unit_double(MOST) == 1.0 - 0x1.0p-53);
    CHECK(unit_float(std::uint64_t{1} << 63) == 0.5f);
}

TEST_CASE("splitmix64 drives a standard distribution", "[splitmix64]") {
    splitmix64 random(7);

    std::uniform_int_distribution<int> die(1, 6);

    for (int i = 0; i < 1000; ++i) {
        const auto rolled = die(random);

        REQUIRE((rolled >= 1 && rolled <= 6));
    }
}

TEST_CASE("splitmix64 defaults to a seed of nought, and copy-list-initialises", "[splitmix64]") {
    splitmix64 defaulted = {};
    splitmix64 seeded(0);

    CHECK(defaulted() == seeded());
    CHECK(defaulted.state() == seeded.state());
}

TEST_CASE("fixed-width integers give the same bits everywhere", "[values][fnv1a64]") {
    CHECK(values(std::int32_t{-1}) == 0x59627b931ee7de0full);
    CHECK(values(std::int64_t{-1}) == 0xa64a4e47b26b306full);
    CHECK(values(std::uint64_t{5}) == values(std::uint32_t{5}));

    CHECK(fnv1a64{}.add(std::uint64_t{5}).value() == 0x0de21504f16dc720ull);
    CHECK(fnv1a64{}.add(std::uint32_t{5}).value() == 0x2d401a55eec16520ull);
}

TEST_CASE("a char hashes as its byte, whichever way the platform signs it", "[values][fnv1a64]") {
    CHECK(values(static_cast<char>(-1)) == values(std::uint8_t{0xff}));
    CHECK(values(static_cast<char>(0xff)) == values(std::uint8_t{0xff}));

    CHECK(fnv1a64{}.add(static_cast<char>(-1)).value() == fnv1a64{}.add(std::uint8_t{0xff}).value());
    CHECK(fnv1a64{}.add(std::string_view("\xff")).value() == fnv1a64{}.add(std::uint8_t{0xff}).value());
}
