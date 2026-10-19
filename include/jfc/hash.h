// © Joseph Cameron - All Rights Reserved

#ifndef JFC_HASH_H
#define JFC_HASH_H

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>
#include <type_traits>

/// \file
/// \brief stable, non-cryptographic hashing: the same input gives the same bits on every platform, compiler and
/// version. 
namespace jfc::hash {
    //! integers, without bool
    template <typename value_type>
    concept integer = std::integral<value_type> && !std::same_as<std::remove_cv_t<value_type>, bool>;

    //! 2^64 / φ
    inline constexpr std::uint64_t GOLDEN = 0x9e3779b97f4a7c15ull;

    /// \brief a bijection: SplitMix64's output for state x (Steele, Lea, Flood 2014; Vigna's constants)
    [[nodiscard]] constexpr std::uint64_t mix64(std::uint64_t x) noexcept {
        x += GOLDEN;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;

        return x ^ (x >> 31);
    }

    //! boost's hash_combine before 1.81, at 64 bits. Order matters
    [[nodiscard]] constexpr std::uint64_t combine(const std::uint64_t aHash, const std::uint64_t aValue) noexcept {
        return aHash ^ (aValue + GOLDEN + (aHash << 6) + (aHash >> 2));
    }

    /// \brief a hash of integers in order, a cell's coordinates and a salt for example
    ///
    /// Each is taken at its own width, unsigned, so an int -1 and an int64_t -1 hash differently.
    template <integer... value_types>
    [[nodiscard]] constexpr std::uint64_t values(const value_types... aValues) noexcept {
        std::uint64_t hash = GOLDEN;

        ((hash = combine(hash, mix64(static_cast<std::make_unsigned_t<value_types>>(aValues)))), ...);

        return hash;
    }

    //! [0, 1), from the top 24 bits
    [[nodiscard]] constexpr float unit_float(const std::uint64_t aHash) noexcept {
        return static_cast<float>(aHash >> 40) * 0x1.0p-24f;
    }

    //! [0, 1), from the top 53 bits
    [[nodiscard]] constexpr double unit_double(const std::uint64_t aHash) noexcept {
        return static_cast<double>(aHash >> 11) * 0x1.0p-53;
    }

    /// \brief FNV-1a, 64 bits (Fowler, Noll, Vo), fed a piece at a time
    ///
    /// Text and bytes are taken as they are, with nothing between pieces: "ab" then "c" is "a" then "bc". A caller
    /// that needs the pieces told apart adds a separator or a length.
    class fnv1a64 final {
    public:
        constexpr fnv1a64 &add(const std::span<const std::byte> aBytes) noexcept {
            for (const auto each : aBytes) eat(static_cast<std::uint8_t>(each));

            return *this;
        }

        constexpr fnv1a64 &add(const std::string_view aText) noexcept {
            for (const auto each : aText) eat(static_cast<std::uint8_t>(each));

            return *this;
        }

        //! its bytes, little endian whatever the machine's order
        template <integer value_type>
        constexpr fnv1a64 &add(const value_type aValue) noexcept {
            const auto bits = static_cast<std::make_unsigned_t<value_type>>(aValue);

            for (std::size_t i = 0; i < sizeof(value_type); ++i) eat(static_cast<std::uint8_t>(bits >> (i * 8)));

            return *this;
        }

        [[nodiscard]] constexpr std::uint64_t value() const noexcept { return m_Value; }

    private:
        constexpr void eat(const std::uint8_t aByte) noexcept {
            m_Value ^= aByte;
            m_Value *= 1099511628211ull;
        }

        std::uint64_t m_Value = 14695981039346656037ull;
    };

    //! SplitMix64 as a generator: each output is mix64 of a counter stepped by GOLDEN. Usable with <random>
    class splitmix64 final {
    public:
        using result_type = std::uint64_t;

        constexpr explicit splitmix64(const std::uint64_t aSeed = 0) noexcept
        : m_State(aSeed)
        {}

        [[nodiscard]] static constexpr result_type min() noexcept { return 0; }

        [[nodiscard]] static constexpr result_type max() noexcept {
            return std::numeric_limits<result_type>::max();
        }

        constexpr result_type operator()() noexcept {
            const auto out = mix64(m_State);

            m_State += GOLDEN;

            return out;
        }

        //! what to seed another with to carry on from here
        [[nodiscard]] constexpr std::uint64_t state() const noexcept { return m_State; }

    private:
        std::uint64_t m_State;
    };
}

#endif
