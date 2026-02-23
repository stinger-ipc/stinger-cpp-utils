#include <stinger/utils/conversions.hpp>

#include <chrono>
#include <stdexcept>

#include <gtest/gtest.h>

using namespace stinger::utils;

// ---------------------------------------------------------------------------
// parseIsoTimestamp – timezone correctness
// ---------------------------------------------------------------------------

// The Unix epoch must map to time_since_epoch() == 0.
// This catches the classic mktime() bug: mktime() interprets struct tm as
// *local* time, so on any timezone other than UTC the result will be off by
// the UTC offset.  The correct implementation must use timegm() (POSIX) or an
// equivalent UTC-aware conversion.
TEST(ParseIsoTimestampTest, EpochIsTimeZeroUtc) {
    auto tp = parseIsoTimestamp("1970-01-01T00:00:00Z");
    auto epoch = std::chrono::system_clock::from_time_t(0);
    EXPECT_EQ(tp, epoch) << "parseIsoTimestamp treats 'Z' as local time instead of UTC";
}

// Verify an arbitrary known UTC timestamp against its well-known Unix value.
// 2024-01-01T00:00:00Z == Unix time 1704067200.
TEST(ParseIsoTimestampTest, KnownUtcTimestamp) {
    auto tp = parseIsoTimestamp("2024-01-01T00:00:00Z");
    auto expected = std::chrono::system_clock::from_time_t(1704067200);
    EXPECT_EQ(tp, expected);
}

// ---------------------------------------------------------------------------
// parseIsoTimestamp – relative differences (timezone-independent)
// ---------------------------------------------------------------------------

// Two timestamps that differ by exactly one hour must produce a 1-hour
// duration regardless of the system timezone.
TEST(ParseIsoTimestampTest, RelativeDifferenceOneHour) {
    auto tp1 = parseIsoTimestamp("2023-06-15T12:00:00Z");
    auto tp2 = parseIsoTimestamp("2023-06-15T13:00:00Z");
    EXPECT_EQ(tp2 - tp1, std::chrono::hours(1));
}

// Cross-midnight boundary: 24-hour difference.
TEST(ParseIsoTimestampTest, RelativeDifferenceOneDay) {
    auto tp1 = parseIsoTimestamp("2023-06-15T00:00:00Z");
    auto tp2 = parseIsoTimestamp("2023-06-16T00:00:00Z");
    EXPECT_EQ(tp2 - tp1, std::chrono::hours(24));
}

// Difference that crosses a year boundary.
TEST(ParseIsoTimestampTest, RelativeDifferenceAcrossYearBoundary) {
    auto tp1 = parseIsoTimestamp("2023-12-31T23:00:00Z");
    auto tp2 = parseIsoTimestamp("2024-01-01T01:00:00Z");
    EXPECT_EQ(tp2 - tp1, std::chrono::hours(2));
}

// ---------------------------------------------------------------------------
// parseIsoTimestamp – subsecond handling
// ---------------------------------------------------------------------------

// Fractional seconds (.NNN) in the input must not cause a parse failure.
TEST(ParseIsoTimestampTest, SubsecondsDoNotThrow) {
    EXPECT_NO_THROW(parseIsoTimestamp("2023-12-01T15:30:45.123Z"));
    EXPECT_NO_THROW(parseIsoTimestamp("2023-12-01T15:30:45.999Z"));
}

// When subseconds are present they are expected to be dropped (whole-second
// precision); the result must be within 1 second of the no-subsecond form.
TEST(ParseIsoTimestampTest, SubsecondsAreDiscarded) {
    auto tp_whole = parseIsoTimestamp("2023-12-01T15:30:45Z");
    auto tp_frac = parseIsoTimestamp("2023-12-01T15:30:45.123Z");
    auto diff = std::chrono::abs(tp_frac - tp_whole);
    EXPECT_LT(diff, std::chrono::seconds(1));
}

// ---------------------------------------------------------------------------
// parseIsoTimestamp – round-trip with timePointToIsoString
// ---------------------------------------------------------------------------

TEST(ParseIsoTimestampTest, RoundTripWithTimePointToIsoString) {
    const std::string original = "2023-12-01T15:30:45Z";
    EXPECT_EQ(timePointToIsoString(parseIsoTimestamp(original)), original);
}

TEST(ParseIsoTimestampTest, RoundTripEpoch) {
    const std::string original = "1970-01-01T00:00:00Z";
    EXPECT_EQ(timePointToIsoString(parseIsoTimestamp(original)), original);
}

// ---------------------------------------------------------------------------
// parseIsoTimestamp – error handling
// ---------------------------------------------------------------------------

TEST(ParseIsoTimestampTest, ThrowsOnEmptyString) {
    EXPECT_THROW(parseIsoTimestamp(""), std::runtime_error);
}

TEST(ParseIsoTimestampTest, ThrowsOnGarbageInput) {
    EXPECT_THROW(parseIsoTimestamp("not-a-timestamp"), std::runtime_error);
}

TEST(ParseIsoTimestampTest, ThrowsOnSlashDateFormat) {
    EXPECT_THROW(parseIsoTimestamp("2023/12/01 15:30:45"), std::runtime_error);
}

// Date-only input (no time component) is accepted leniently: missing time
// fields are zero-initialised, so the result is midnight UTC on that date.
TEST(ParseIsoTimestampTest, DateOnlyParsesAsMidnightUtc) {
    // 2023-12-01T00:00:00Z == Unix time 1701388800
    auto tp = parseIsoTimestamp("2023-12-01");
    auto expected = std::chrono::system_clock::from_time_t(1701388800);
    EXPECT_EQ(tp, expected);
}
