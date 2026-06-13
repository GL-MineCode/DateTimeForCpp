Select a language | 选择语言

[**简体中文**](README_cn.md)  
[**English**](README.md)  

# DateTime for C++

A C++ header-only library that mimics the C# `DateTime` and `TimeSpan` classes, providing nearly identical functionality and usage.

## Features

- **Single header** — Just `#include "GL_DateTime.hpp"`, no extra dependencies
- **C#-style formatting** — Use familiar format strings like `yyyy-MM-dd HH:mm:ss`
- **Comprehensive parsing** — `Parse()` / `TryParse()` with automatic format detection or explicit format specification
- **Date/time arithmetic** — Add/subtract years, months, days, hours, minutes, seconds, milliseconds
- **TimeSpan support** — Full `TimeSpan` class mirroring `System.TimeSpan` (arithmetic, comparison, formatting)
- **DateTime arithmetic with TimeSpan** — `DateTime ± TimeSpan = DateTime`, `DateTime - DateTime = TimeSpan`
- **UTC/Local conversion** — `ToUniversalTime()` / `ToLocalTime()` with `DateTimeKind` tracking
- **Comparison operators** — Full set: `==`, `!=`, `<`, `<=`, `>`, `>=`
- **Stream operators** — Direct `<<` / `>>` support for `cin`/`cout`
- **Ticks-based** — Internal storage uses 100-nanosecond ticks since 0001-01-01, matching C# exactly
- **Full date range** — Supports years 0001 through 9999
- **C++20** — Requires C++20

## Quick Start

```cpp
#include "GL_DateTime.hpp"
#include <iostream>

int main() {
    // Current time
    DateTime now = DateTime::Now();
    std::cout << "Now: " << now << std::endl;

    // Construct with specific date/time
    DateTime dt(2024, 12, 25, 10, 30, 0, 123);
    std::cout << dt.ToString("yyyy-MM-dd HH:mm:ss.fff") << std::endl;

    // Parse from string
    DateTime parsed = DateTime::Parse("2024-06-07 14:30:00");

    // Date arithmetic with TimeSpan
    DateTime tomorrow = now.AddDays(1);
    TimeSpan diff = tomorrow - now;
    std::cout << "Hours until tomorrow: " << diff.GetTotalHours() << std::endl;

    return 0;
}
```

## Building

This is a header-only library — just include the header:

```bash
g++ -std=c++20 -Iinclude your_program.cpp -o your_program
```

Or use the provided Makefile to build the test:

```bash
mingw32-make test    # Build test/test.cpp
./build/test.exe      # Run tests
```

## API Reference

### DateTime Constructors

| Constructor | Description |
|-------------|-------------|
| `DateTime()` | Default — initializes to `MinValue` (0001-01-01), matching C# |
| `DateTime(std::time_t t)` | From Unix timestamp (Kind=Local) |
| `DateTime(int64_t ticks, DateTimeKind kind = Unspecified)` | From ticks and optional Kind |
| `DateTime(year, month, day, hour=0, minute=0, second=0, ms=0, kind=Unspecified)` | From components (throws on invalid values) |

### DateTime Static Methods

| Method | Description |
|--------|-------------|
| `DateTime::Now()` | Current local date and time |
| `DateTime::UtcNow()` | Current UTC date and time |
| `DateTime::Today()` | Today's date (time at 00:00:00) |
| `DateTime::MinValue()` | Minimum value (0001-01-01) |
| `DateTime::MaxValue()` | Maximum value (9999-12-31 23:59:59.9999999) |
| `DateTime::DaysInMonth(year, month)` | Number of days in the specified month |
| `DateTime::IsLeapYear(year)` | Whether the specified year is a leap year |
| `DateTime::Compare(dt1, dt2)` | Compares two DateTimes (-1 / 0 / 1) |
| `DateTime::Equals(dt1, dt2)` | Whether two DateTimes are equal |
| `DateTime::SpecifyKind(dt, kind)` | Returns new DateTime with specified Kind |

### DateTime Properties (Getters)

| Method | Description |
|--------|-------------|
| `GetYear()` / `GetMonth()` / `GetDay()` | Year / Month / Day |
| `GetHour()` / `GetMinute()` / `GetSecond()` | Hour / Minute / Second |
| `GetMillisecond()` | Millisecond component |
| `GetDayOfWeek()` | Day of week (0=Sunday, matching C#) |
| `GetDayOfYear()` | Day of year (1-based) |
| `GetDate()` | New DateTime with time set to 00:00:00 |
| `GetTimeOfDay()` | Time since midnight as TimeSpan |
| `GetTicks()` | Number of 100-nanosecond ticks since 0001-01-01 |
| `GetKind()` | DateTimeKind (Unspecified / Utc / Local) |
| `ToTime_t()` | Convert to Unix `time_t` |

### DateTime Arithmetic

| Method | Description |
|--------|-------------|
| `Add(TimeSpan)` | Add a TimeSpan |
| `AddDays(n)` | Add n days |
| `AddHours(n)` | Add n hours |
| `AddMinutes(n)` | Add n minutes |
| `AddSeconds(n)` | Add n seconds |
| `AddMilliseconds(n)` | Add n milliseconds |
| `AddMonths(n)` | Add n months (auto-clamps to month end) |
| `AddYears(n)` | Add n years (handles Feb 29 in leap years) |
| `Subtract(TimeSpan)` → DateTime | Subtract a TimeSpan |
| `Subtract(DateTime)` → TimeSpan | Subtract another DateTime, return TimeSpan |

### DateTime Conversion

| Method | Description |
|--------|-------------|
| `ToUniversalTime()` | Convert to UTC (updates Kind) |
| `ToLocalTime()` | Convert to local time (updates Kind) |

### ToString Format

Uses C#-style format strings, including both **predefined shorthand** (single character) and **custom** format specifiers.

#### Predefined Format Specifiers

| Specifier | Meaning | Expanded Format |
|-----------|---------|----------------|
| `d` | Short date | `MM/dd/yyyy` |
| `D` | Long date | `dddd, MMMM dd, yyyy` |
| `f` | Full date/time (short time) | `dddd, MMMM dd, yyyy HH:mm` |
| `F` | Full date/time (long time) | `dddd, MMMM dd, yyyy HH:mm:ss` |
| `g` | General (short time) | `MM/dd/yyyy HH:mm` |
| `G` | General (long time) | `MM/dd/yyyy HH:mm:ss` |
| `M` / `m` | Month/day | `MMMM dd` |
| `O` / `o` | Round-trip | `yyyy-MM-ddTHH:mm:ss.fffffff` |
| `R` / `r` | RFC1123 | `ddd, dd MMM yyyy HH:mm:ss 'GMT'` |
| `s` | Sortable | `yyyy-MM-ddTHH:mm:ss` |
| `t` | Short time | `HH:mm` |
| `T` | Long time | `HH:mm:ss` |
| `u` | Universal sortable | `yyyy-MM-dd HH:mm:ss'Z'` |
| `U` | Universal full | Converts to UTC, then uses `F` format |
| `Y` / `y` | Year/month | `yyyy MMMM` |

```cpp
dt.ToString("d");     // "06/07/2024"
dt.ToString("D");     // "Friday, June 07, 2024"
dt.ToString("o");     // "2024-06-07T14:30:45.7890000"
dt.ToString("s");     // "2024-06-07T14:30:45"
dt.ToString("U");     // "Friday, June 07, 2024 06:30:45" (UTC)
```

#### Custom Format Specifiers

| Specifier | Description | Example |
|-----------|-------------|---------|
| `y` / `yy` | Short year | `24` |
| `yyyy` | Full year | `2024` |
| `M` / `MM` | Numeric month | `6` / `06` |
| `MMM` | Abbreviated month name | `Jun` |
| `MMMM` | Full month name | `June` |
| `d` / `dd` | Numeric day | `7` / `07` |
| `ddd` | Abbreviated weekday | `Fri` |
| `dddd` | Full weekday | `Friday` |
| `H` / `HH` | 24-hour | `14` / `14` |
| `h` / `hh` | 12-hour | `2` / `02` |
| `m` / `mm` | Minute | `5` / `05` |
| `s` / `ss` | Second | `3` / `03` |
| `f` / `ff` / `fff` | Millisecond | `7` / `78` / `789` |
| `t` / `tt` | AM/PM indicator | `A` / `PM` |
| `'text'` | Literal text escape | `'hello'` → `hello` |

> **Note:** Due to `strftime` limitations, a single `t` displays the full AM/PM indicator (like `tt`) rather than just the first character.

```cpp
dt.ToString("yyyy-MM-dd HH:mm:ss");        // 2024-06-07 14:05:03
dt.ToString("hh:mm tt");                  // 02:05 PM
dt.ToString("dddd, MMMM d, yyyy");        // Friday, June 7, 2024
dt.ToString("yyyy'年'MM'月'dd'日'");      // 2024年06月07日
dt.ToString("yyyy-MM-dd HH:mm:ss.fff");   // 2024-06-07 14:05:03.789
dt.ToShortDateString();                   // 2024-06-07
dt.ToLongDateString();                    // Friday, June 07, 2024
dt.ToShortTimeString();                   // 14:05
dt.ToLongTimeString();                    // 14:05:03
```

### Parse / TryParse

```cpp
// Auto-detect common formats
DateTime dt = DateTime::Parse("2024-06-07 14:30:00");
DateTime dt = DateTime::Parse("06/07/2024");

// Parse with explicit format
DateTime dt = DateTime::Parse("2024/06/07 14:30:00", "yyyy/MM/dd HH:mm:ss");
DateTime dt = DateTime::Parse("06/07/2024 02:30:00 PM", "MM/dd/yyyy hh:mm:ss tt");

// Parse with predefined format shorthand
DateTime dt = DateTime::Parse("06/07/2024", "d");                // short date
DateTime dt = DateTime::Parse("2024-06-07T14:30:00", "s");       // sortable

// TryParse (safe version)
DateTime result;
if (DateTime::TryParse("2024-06-07", result)) {
    // Parsed successfully
}
```

### DateTime Operators

```cpp
dt1 + ts     // DateTime + TimeSpan → DateTime
ts + dt1    // TimeSpan + DateTime → DateTime
dt1 - ts    // DateTime - TimeSpan → DateTime
dt1 - dt2   // DateTime - DateTime → TimeSpan
dt1 == dt2  // equality
dt1 != dt2  // inequality
dt1 <  dt2  // less than
dt1 <= dt2  // less than or equal
dt1 >  dt2  // greater than
dt1 >= dt2  // greater than or equal
```

### Stream Operators

```cpp
DateTime dt = DateTime::Now();
std::cout << dt << std::endl;         // Outputs: 2026-06-07 18:24:20

std::cin >> dt;                        // Input: 2024-06-07 14:30:00
```

## TimeSpan

A complete `TimeSpan` class mirroring `System.TimeSpan`.

### TimeSpan Constructors

| Constructor | Description |
|-------------|-------------|
| `TimeSpan()` | Zero value |
| `TimeSpan(int64_t ticks)` | From ticks (100-ns intervals) |
| `TimeSpan(hours, minutes, seconds)` | From time components |
| `TimeSpan(days, hours, minutes, seconds)` | From day + time components |
| `TimeSpan(days, hours, minutes, seconds, milliseconds)` | Full constructor |

### TimeSpan Properties

| Method | Description |
|--------|-------------|
| `GetDays()` / `GetHours()` / `GetMinutes()` / `GetSeconds()` / `GetMilliseconds()` | Component values |
| `GetTicks()` | Total ticks |
| `GetTotalDays()` / `GetTotalHours()` / `GetTotalMinutes()` / `GetTotalSeconds()` / `GetTotalMilliseconds()` | Total value as double |

### TimeSpan Static Methods

```cpp
TimeSpan::Zero()          // TimeSpan(0)
TimeSpan::MinValue()      // Minimum possible TimeSpan
TimeSpan::MaxValue()      // Maximum possible TimeSpan
TimeSpan::FromDays(1.5)   // 1.5 days → TimeSpan
TimeSpan::FromHours(2.5)  // 2.5 hours → TimeSpan
// ... FromMinutes, FromSeconds, FromMilliseconds
```

### TimeSpan Arithmetic

```cpp
ts1 + ts2   // addition
ts1 - ts2   // subtraction
-ts1        // negation
ts1.Duration()  // absolute value
ts1.CompareTo(ts2)  // comparison
```

### TimeSpan Operators

```cpp
ts1 + ts2      // addition
ts1 - ts2      // subtraction
-ts1           // negation
ts1 == ts2     // equality
ts1 != ts2     // inequality
ts1 <  ts2     // less than
ts1 <= ts2     // less than or equal
ts1 >  ts2     // greater than
ts1 >= ts2     // greater than or equal
```

### TimeSpan Formatting

```cpp
TimeSpan ts(1, 2, 30, 0);    // 1 day, 2 hours, 30 minutes
std::cout << ts;             // Output: 1.02:30:00
```

## DateTimeKind

```cpp
enum class DateTimeKind {
    Unspecified = 0,   // Default
    Utc         = 1,
    Local       = 2
};
```

## Localization

By default, month and weekday names in `ToString` use the system language. To switch languages, call `setlocale` from `<locale.h>`:

```cpp
#include <locale.h>

// Windows - US English
setlocale(LC_ALL, "English_United States.1252");
// Windows - Simplified Chinese
setlocale(LC_ALL, "Chinese (Simplified)_China.UTF-8");
```

## Project Structure

```
DateTimeForCpp/
├── include/
│   └── GL_DateTime.hpp      # Main file
├── test/
│   └── test.cpp             # Test program
├── Makefile                 # Makefile
├── README.md                # English Document
├── README_cn.md             # Chinese Document
└── LICENSE
```

## Differences from C# DateTime

- The single `t` format specifier (for AM/PM) is treated as `tt` — always displays full AM/PM string
- Microsecond and tick precision are simulated (derived from milliseconds)
- `MaxValue` may throw on some platforms due to `localtime_s` limitations

## License

This project is open source under the MIT License. See [LICENSE](LICENSE).
