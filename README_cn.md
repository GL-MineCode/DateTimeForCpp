选择语言 | Select a language

[**English**](README.md)  
[**简体中文**](README_cn.md)  

# DateTime for C++

一个模仿 C# `DateTime` 和 `TimeSpan` 类的 C++ 头文件库，提供近乎一致的功能和用法。

## 特性

- **单头文件** — 仅需包含 `GL_DateTime.hpp`，无额外依赖
- **C# 风格格式化** — 使用 `yyyy-MM-dd HH:mm:ss` 等格式字符串
- **全面的解析支持** — `Parse()` / `TryParse()` 自动推断常见格式或指定格式解析
- **日期时间运算** — 添加/减少 年、月、日、时、分、秒、毫秒
- **TimeSpan 支持** — 完整的 `TimeSpan` 类（算术、比较、格式化）
- **DateTime 与 TimeSpan 运算** — `DateTime ± TimeSpan = DateTime`，`DateTime - DateTime = TimeSpan`
- **UTC/本地转换** — `ToUniversalTime()` / `ToLocalTime()`，附带 `DateTimeKind` 追踪
- **比较运算** — 完整的 `==`、`!=`、`<`、`<=`、`>`、`>=` 操作符
- **流操作符** — 支持 `<<` / `>>` 直接与 `cin`/`cout` 交互
- **Ticks 存储** — 内部使用 100 纳秒间隔的 ticks（自 0001-01-01 起），与 C# 完全一致
- **完整日期范围** — 支持公元 1 年至 9999 年
- **C++20** — 使用 C++20 标准

## 快速开始

```cpp
#include "GL_DateTime.hpp"
#include <iostream>

int main() {
    // 当前时间
    DateTime now = DateTime::Now();
    std::cout << "Now: " << now << std::endl;

    // 构造指定时间
    DateTime dt(2024, 12, 25, 10, 30, 0, 123);
    std::cout << dt.ToString("yyyy-MM-dd HH:mm:ss.fff") << std::endl;

    // 解析字符串
    DateTime parsed = DateTime::Parse("2024-06-07 14:30:00");

    // 使用 TimeSpan 做日期运算
    DateTime tomorrow = now.AddDays(1);
    TimeSpan diff = tomorrow - now;
    std::cout << "距离明天还有 " << diff.GetTotalHours() << " 小时" << std::endl;

    return 0;
}
```

## 编译

本项目仅需包含头文件即可使用：

```bash
g++ -std=c++20 -Iinclude your_program.cpp -o your_program
```

也可使用 Makefile 编译测试：

```bash
mingw32-make test    # 编译 test/test.cpp
./build/test.exe      # 运行测试
```

## API 参考

### DateTime 构造函数

| 构造函数 | 说明 |
|---------|------|
| `DateTime()` | 默认构造，初始化为 `MinValue` (0001-01-01)，与 C# 一致 |
| `DateTime(std::time_t t)` | 从 Unix 时间戳构造 (Kind=Local) |
| `DateTime(int64_t ticks, DateTimeKind kind = Unspecified)` | 从 ticks 和可选 Kind 构造 |
| `DateTime(year, month, day, hour=0, minute=0, second=0, ms=0, kind=Unspecified)` | 从各分量构造（超出范围抛异常） |

### DateTime 静态方法

| 方法 | 说明 |
|------|------|
| `DateTime::Now()` | 当前本地时间 |
| `DateTime::UtcNow()` | 当前 UTC 时间 |
| `DateTime::Today()` | 当天日期（时分秒归零） |
| `DateTime::MinValue()` | 最小值 (0001-01-01) |
| `DateTime::MaxValue()` | 最大值 (9999-12-31 23:59:59.9999999) |
| `DateTime::DaysInMonth(year, month)` | 指定月份的天数 |
| `DateTime::IsLeapYear(year)` | 指定年份是否为闰年 |
| `DateTime::Compare(dt1, dt2)` | 比较两个 DateTime (-1 / 0 / 1) |
| `DateTime::Equals(dt1, dt2)` | 判断两个 DateTime 是否相等 |
| `DateTime::SpecifyKind(dt, kind)` | 返回具有指定 Kind 的新 DateTime |

### DateTime 属性获取

| 方法 | 说明 |
|------|------|
| `GetYear()` / `GetMonth()` / `GetDay()` | 年/月/日 |
| `GetHour()` / `GetMinute()` / `GetSecond()` | 时/分/秒 |
| `GetMillisecond()` | 毫秒 |
| `GetDayOfWeek()` | 星期几 (0=周日，与 C# 一致) |
| `GetDayOfYear()` | 一年中的第几天（从 1 开始） |
| `GetDate()` | 返回日期部分（时间归零）的新 DateTime |
| `GetTimeOfDay()` | 返回当天时间作为 TimeSpan |
| `GetTicks()` | 自 0001-01-01 以来的 100 纳秒间隔数 |
| `GetKind()` | DateTimeKind (Unspecified / Utc / Local) |
| `ToTime_t()` | 转换为 Unix `time_t` |

### DateTime 日期运算

| 方法 | 说明 |
|------|------|
| `Add(TimeSpan)` | 添加时间间隔 |
| `AddDays(n)` | 添加 n 天 |
| `AddHours(n)` | 添加 n 小时 |
| `AddMinutes(n)` | 添加 n 分钟 |
| `AddSeconds(n)` | 添加 n 秒 |
| `AddMilliseconds(n)` | 添加 n 毫秒 |
| `AddMonths(n)` | 添加 n 月（自动处理月末对齐） |
| `AddYears(n)` | 添加 n 年（自动处理闰年2月29） |
| `Subtract(TimeSpan)` → DateTime | 减去一个 TimeSpan |
| `Subtract(DateTime)` → TimeSpan | 减去另一个 DateTime，返回 TimeSpan 差 |

### DateTime 转换

| 方法 | 说明 |
|------|------|
| `ToUniversalTime()` | 转换为 UTC（更新 Kind） |
| `ToLocalTime()` | 转换为本地时间（更新 Kind） |

### ToString 格式

使用 C# 风格的格式字符串：

| 格式符 | 说明 | 示例 |
|--------|------|------|
| `y` / `yy` | 短年份 | `24` |
| `yyyy` | 完整年份 | `2024` |
| `M` / `MM` | 数字月份 | `6` / `06` |
| `MMM` | 缩写月份名 | `Jun` |
| `MMMM` | 完整月份名 | `June` |
| `d` / `dd` | 数字日期 | `7` / `07` |
| `ddd` | 缩写星期名 | `Fri` |
| `dddd` | 完整星期名 | `Friday` |
| `H` / `HH` | 24小时制 | `14` / `14` |
| `h` / `hh` | 12小时制 | `2` / `02` |
| `m` / `mm` | 分钟 | `5` / `05` |
| `s` / `ss` | 秒钟 | `3` / `03` |
| `f` / `ff` / `fff` | 毫秒 | `7` / `78` / `789` |
| `t` / `tt` | AM/PM 指示符 | `A` / `PM` |
| `'text'` | 文本引号 | `'年'` → `年` |

> **注意：** 由于 `strftime` 的限制，单个 `t` 的行为与 `tt` 相同（显示完整 AM/PM），而非仅显示首字符。

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
// 自动推断常见格式
DateTime dt = DateTime::Parse("2024-06-07 14:30:00");
DateTime dt = DateTime::Parse("06/07/2024");

// 指定格式解析
DateTime dt = DateTime::Parse("2024年06月07日", "yyyy'年'MM'月'dd'日'");
DateTime dt = DateTime::Parse("06/07/2024 02:30:00 PM", "MM/dd/yyyy hh:mm:ss tt");

// TryParse（安全版本）
DateTime result;
if (DateTime::TryParse("2024-06-07", result)) {
    // 解析成功
}
```

### DateTime 运算符

```cpp
dt1 + ts     // DateTime + TimeSpan → DateTime
ts + dt1    // TimeSpan + DateTime → DateTime
dt1 - ts    // DateTime - TimeSpan → DateTime
dt1 - dt2   // DateTime - DateTime → TimeSpan
dt1 == dt2  // 相等
dt1 != dt2  // 不等
dt1 <  dt2  // 小于
dt1 <= dt2  // 小于等于
dt1 >  dt2  // 大于
dt1 >= dt2  // 大于等于
```

### 流操作符

```cpp
DateTime dt = DateTime::Now();
std::cout << dt << std::endl;         // 输出: 2026-06-07 18:24:20

std::cin >> dt;                        // 输入: 2024-06-07 14:30:00
```

## TimeSpan

完整的 `TimeSpan` 类，与 `System.TimeSpan` 一致。

### TimeSpan 构造函数

| 构造函数 | 说明 |
|---------|------|
| `TimeSpan()` | 零值 |
| `TimeSpan(int64_t ticks)` | 从刻度数构造（100 纳秒间隔） |
| `TimeSpan(hours, minutes, seconds)` | 从时间分量构造 |
| `TimeSpan(days, hours, minutes, seconds)` | 从天 + 时间分量构造 |
| `TimeSpan(days, hours, minutes, seconds, milliseconds)` | 完整构造函数 |

### TimeSpan 属性

| 方法 | 说明 |
|------|------|
| `GetDays()` / `GetHours()` / `GetMinutes()` / `GetSeconds()` / `GetMilliseconds()` | 各分量值 |
| `GetTicks()` | 总刻度数 |
| `GetTotalDays()` / `GetTotalHours()` / `GetTotalMinutes()` / `GetTotalSeconds()` / `GetTotalMilliseconds()` | 以 double 表示的总值 |

### TimeSpan 静态方法

```cpp
TimeSpan::Zero()          // TimeSpan(0)
TimeSpan::MinValue()      // 最小可能的 TimeSpan
TimeSpan::MaxValue()      // 最大可能的 TimeSpan
TimeSpan::FromDays(1.5)   // 1.5 天 → TimeSpan
TimeSpan::FromHours(2.5)  // 2.5 小时 → TimeSpan
// ... FromMinutes, FromSeconds, FromMilliseconds
```

### TimeSpan 算术运算

```cpp
ts1 + ts2   // 加法
ts1 - ts2   // 减法
-ts1        // 取反
ts1.Duration()  // 绝对值
ts1.CompareTo(ts2)  // 比较
```

### TimeSpan 运算符

```cpp
ts1 + ts2      // 加法
ts1 - ts2      // 减法
-ts1           // 取反
ts1 == ts2     // 相等
ts1 != ts2     // 不等
ts1 <  ts2     // 小于
ts1 <= ts2     // 小于等于
ts1 >  ts2     // 大于
ts1 >= ts2     // 大于等于
```

### TimeSpan 格式化

```cpp
TimeSpan ts(1, 2, 30, 0);    // 1天2小时30分钟
std::cout << ts;             // 输出: 1.02:30:00
```

## DateTimeKind

```cpp
enum class DateTimeKind {
    Unspecified = 0,   // 默认
    Utc         = 1,
    Local       = 2
};
```

## 本地化

默认情况下，`ToString` 中的月份名和星期名使用系统语言。若要切换语言，调用 `<locale.h>` 的 `setlocale` 函数：

```cpp
#include <locale.h>

// Windows - 简体中文
setlocale(LC_ALL, "Chinese (Simplified)_China.UTF-8");
// Windows - 美国英语
setlocale(LC_ALL, "English_United States.1252");
```

## 项目结构

```
DateTimeForCpp/
├── include/
│   └── GL_DateTime.hpp      # 主文件
├── test/
│   └── test.cpp             # 测试程序
├── Makefile                 # 构建配置
├── README.md                # 英文文档
├── README_cn.md             # 中文文档
└── LICENSE
```

## 与 C# DateTime 的差异

- 格式符 `t`（单 t）被视为 `tt`（双 t），始终显示完整 AM/PM 字符串
- 微秒和百纳秒精度为模拟值（基于毫秒推算）

## 许可证

本项目基于 MIT 许可证开源。详见 [LICENSE](LICENSE)。
