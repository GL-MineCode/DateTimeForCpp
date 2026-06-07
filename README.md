Select a language | 选择语言

[**简体中文**](README_cn.md)  
[**English**](README.md)  

# DateTime for C++

A C++ header-only library that mimics the C# `DateTime` class, providing nearly identical functionality and usage.

## Features

- **Single header** — Just `#include "GL_DateTime.hpp"`, no extra dependencies
- **C#-style formatting** — Use familiar format strings like `yyyy-MM-dd HH:mm:ss`
- **Comprehensive parsing** — `Parse()` / `TryParse()` with automatic format detection or explicit format specification
- **Date/time arithmetic** — Add/subtract years, months, days, hours, minutes, seconds, milliseconds
- **Comparison operators** — Full set: `==`, `!=`, `<`, `<=`, `>`, `>=`
- **UTC support** — All properties available in UTC
- **Stream operators** — Direct `<<` / `>>` support for `cin`/`cout`
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
    
    // Date arithmetic
    DateTime tomorrow = now.AddDays(1);
    
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

### Constructors

| Constructor | Description |
|-------------|-------------|
| `DateTime()` | Default constructor — initializes to current local time |
| `DateTime(std::time_t t)` | Construct from `time_t` |
| `DateTime(year, month, day, hour=0, minute=0, second=0, ms=0)` | Construct from components (throws on invalid values) |

### Static Methods

| Method | Description |
|--------|-------------|
| `DateTime::Now()` | Current local date and time |
| `DateTime::UtcNow()` | Current UTC date and time |
| `DateTime::Today()` | Today's date (time at 00:00:00) |
| `DateTime::MinValue()` | Minimum representable time (1970-01-01) |
| `DateTime::MaxValue()` | Maximum representable time (9999-12-31) |

### Property Getters

| Method | Description |
|--------|-------------|
| `GetYear()` / `GetMonth()` / `GetDay()` | Year / Month / Day |
| `GetHour()` / `GetMinute()` / `GetSecond()` | Hour / Minute / Second |
| `GetMillisecond()` | Millisecond component |
| `GetDayOfWeek()` | Day of week (0=Sunday) |
| `GetDayOfYear()` | Day of year (1-based) |
| `GetUtcYear()` ... `GetUtcSecond()` | UTC component values |
| `ToTime_t()` | Convert to `time_t` |

### ToString Format

Uses C#-style format strings:

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
| `t` / `tt` | AM/PM indicator | `P` / `PM` |
| `'text'` | Literal text escape | `'hello'` → `hello` |

```cpp
dt.ToString("yyyy-MM-dd HH:mm:ss");     // 2024-06-07 14:05:03
dt.ToString("hh:mm tt");               // 02:05 PM
dt.ToString("dddd, MMMM d, yyyy");     // Friday, June 7, 2024
dt.ToShortDateString();                 // 2024-06-07
dt.ToLongDateString();                  // Friday, June 07, 2024
dt.ToShortTimeString();                 // 14:05
dt.ToLongTimeString();                  // 14:05:03
```

### Parse / TryParse

```cpp
// Auto-detect common formats
DateTime dt = DateTime::Parse("2024-06-07 14:30:00");
DateTime dt = DateTime::Parse("06/07/2024");

// Parse with explicit format
DateTime dt = DateTime::Parse("2024/06/07 14:30:00", "yyyy/MM/dd HH:mm:ss");
DateTime dt = DateTime::Parse("06/07/2024 02:30:00 PM", "MM/dd/yyyy hh:mm:ss tt");

// TryParse (safe version)
DateTime result;
if (DateTime::TryParse("2024-06-07", result)) {
    // Parsed successfully
}
```

### Date Arithmetic

| Method | Description |
|--------|-------------|
| `AddDays(n)` | Add n days |
| `AddHours(n)` | Add n hours |
| `AddMinutes(n)` | Add n minutes |
| `AddSeconds(n)` | Add n seconds |
| `AddMilliseconds(n)` | Add n milliseconds |
| `AddMonths(n)` | Add n months (auto-clamps to month end) |
| `AddYears(n)` | Add n years (handles Feb 29 in leap years) |

### Comparison

```cpp
dt1 == dt2    // equals
dt1 != dt2    // not equals
dt1 <  dt2    // less than
dt1 <= dt2    // less than or equal
dt1 >  dt2    // greater than
dt1 >= dt2    // greater than or equal
```

### Time Difference

```cpp
double days    = dt2.TotalDaysSince(dt1);
double hours   = dt2.TotalHoursSince(dt1);
double minutes = dt2.TotalMinutesSince(dt1);
double seconds = dt2.TotalSecondsSince(dt1);
```

### Stream Operators

```cpp
DateTime dt = DateTime::Now();
std::cout << dt << std::endl;         // Outputs: 2026-06-07 18:24:20

std::cin >> dt;                        // Input: 2024-06-07 14:30:00
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
│   └── GL_DateTime.hpp      # Single header — the whole library
├── test/
│   └── test.cpp             # Test program
├── Makefile                 # Build script
├── README.md                # English documentation
└── README_cn.md             # Chinese documentation
```

## Differences from C# DateTime

- The single `t` format specifier (for AM/PM) is treated as `tt` — always displays full AM/PM string
- Microsecond and tick precision are simulated (derived from milliseconds)
- `MaxValue` may throw on some platforms due to `localtime_s` limitations

## License

This project is open source under the MIT License. See [LICENSE](LICENSE).
