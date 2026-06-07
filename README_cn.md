选择语言 | Select a language

[**English**](README.md)  
[**简体中文**](README_cn.md)  

# DateTime for C++

一个模仿 C# `DateTime` 类的 C++ 头文件库，提供近乎一致的功能和用法。

## 特性

- **单头文件** — 仅需包含 `GL_DateTime.hpp`，无额外依赖
- **C# 风格格式化** — 使用 `yyyy-MM-dd HH:mm:ss` 等格式字符串
- **全面的解析支持** — `Parse()` / `TryParse()` 自动推断常见格式或指定格式解析
- **日期时间运算** — 添加/减少 年、月、日、时、分、秒、毫秒
- **比较运算** — 完整的 `==`、`!=`、`<`、`<=`、`>`、`>=` 操作符
- **UTC 支持** — 获取 UTC 时间的全部属性
- **流操作符** — 支持 `<<` / `>>` 直接与 `cin`/`cout` 交互
- **C++20** — 需要 C++20 编译

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
    
    // 日期运算
    DateTime tomorrow = now.AddDays(1);
    
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

### 构造函数

| 构造函数 | 说明 |
|---------|------|
| `DateTime()` | 默认构造，初始化为当前本地时间 |
| `DateTime(std::time_t t)` | 从 `time_t` 构造 |
| `DateTime(year, month, day, hour=0, minute=0, second=0, ms=0)` | 从各分量构造（超出范围抛异常） |

### 静态方法

| 方法 | 说明 |
|------|------|
| `DateTime::Now()` | 当前本地时间 |
| `DateTime::UtcNow()` | 当前 UTC 时间 |
| `DateTime::Today()` | 当天日期（时分秒归零） |
| `DateTime::MinValue()` | 最小可表示时间 (1970-01-01) |
| `DateTime::MaxValue()` | 最大可表示时间 (9999-12-31) |

### 属性获取

| 方法 | 说明 |
|------|------|
| `GetYear()` / `GetMonth()` / `GetDay()` | 年/月/日 |
| `GetHour()` / `GetMinute()` / `GetSecond()` | 时/分/秒 |
| `GetMillisecond()` | 毫秒 |
| `GetDayOfWeek()` | 星期几 (0=周日) |
| `GetDayOfYear()` | 一年中的第几天 |
| `GetUtcYear()` ... `GetUtcSecond()` | UTC 时间各分量 |
| `ToTime_t()` | 转换为 `time_t` |

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
| `t` / `tt` | AM/PM 指示符 | `P` / `PM` |
| `'text'` | 文本引号 | `'年'` → `年` |

```cpp
dt.ToString("yyyy-MM-dd HH:mm:ss");     // 2024-06-07 14:05:03
dt.ToString("hh:mm tt");               // 02:05 PM
dt.ToString("dddd, MMMM d, yyyy");     // Friday, June 7, 2024
dt.ToString("yyyy'年'MM'月'dd'日'");   // 2024年06月07日
dt.ToShortDateString();                 // 2024-06-07
dt.ToLongDateString();                  // Friday, June 07, 2024
dt.ToShortTimeString();                 // 14:05
dt.ToLongTimeString();                  // 14:05:03
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

### 日期运算

| 方法 | 说明 |
|------|------|
| `AddDays(n)` | 添加 n 天 |
| `AddHours(n)` | 添加 n 小时 |
| `AddMinutes(n)` | 添加 n 分钟 |
| `AddSeconds(n)` | 添加 n 秒 |
| `AddMilliseconds(n)` | 添加 n 毫秒 |
| `AddMonths(n)` | 添加 n 月（自动处理月末对齐） |
| `AddYears(n)` | 添加 n 年（自动处理闰年2月29） |

### 比较运算

```cpp
dt1 == dt2    // 相等
dt1 != dt2    // 不等
dt1 <  dt2    // 小于
dt1 <= dt2    // 小于等于
dt1 >  dt2    // 大于
dt1 >= dt2    // 大于等于
```

### 时间差

```cpp
double days    = dt2.TotalDaysSince(dt1);
double hours   = dt2.TotalHoursSince(dt1);
double minutes = dt2.TotalMinutesSince(dt1);
double seconds = dt2.TotalSecondsSince(dt1);
```

### 流操作符

```cpp
DateTime dt = DateTime::Now();
std::cout << dt << std::endl;         // 输出: 2026-06-07 18:24:20

std::cin >> dt;                        // 输入: 2024-06-07 14:30:00
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
│   └── GL_DateTime.hpp      # 主头文件（仅此一个）
├── test/
│   └── test.cpp             # 测试程序
├── Makefile                 # 构建脚本
├── README.md                # 英文文档
└── README_cn.md             # 中文文档
```

## 与 C# DateTime 的差异

- 格式符 `t`（单 t）被视为 `tt`（双 t），始终显示完整 AM/PM 字符串
- 微秒和百纳秒精度为模拟值（基于毫秒推算）
- `MaxValue` 在部分平台上可能因 `localtime_s` 限制而抛出异常

## 许可证

本项目基于 MIT 许可证开源。详见 [LICENSE](LICENSE)。
