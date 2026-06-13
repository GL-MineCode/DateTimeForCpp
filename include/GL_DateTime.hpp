#ifndef __INC_GL_DATETIME_
#define __INC_GL_DATETIME_

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <cstdint>
#include <climits>
#include <cstring>

/*
    I noticed that C# automatically sets the language for its standard libraries,
    so the DateTime type in C# always uses the system's default language.

    To make DateTime support other languages, call the setlocale function from locale.h.
    Below is an example (for Windows) to set the locale to Simplified Chinese with UTF-8 encoding:
    setlocale(LC_ALL, "Chinese (Simplified)_China.UTF-8");
*/
/*
    我注意到C#中似乎自动设置了标准库使用的语言，所以C#中的DateTime总是系统默认语言
    若要使DateTime支持其他语言，请调用locale.h里的setlocale函数，这是比如设置为简体中文UTF-8的例子(Windows平台):
    setlocale(LC_ALL, "Chinese (Simplified)_China.UTF-8");
*/

namespace detail
{
    constexpr int64_t TicksPerMillisecond = 10000;
    constexpr int64_t TicksPerSecond = TicksPerMillisecond * 1000;
    constexpr int64_t TicksPerMinute = TicksPerSecond * 60;
    constexpr int64_t TicksPerHour = TicksPerMinute * 60;
    constexpr int64_t TicksPerDay = TicksPerHour * 24;

    constexpr int DaysTo1970 = 719162;
    constexpr int DaysTo10000 = 3652059; // days from 1/1/0001 to 1/1/10000
    constexpr int64_t MinTicks = 0;
    constexpr int64_t MaxTicks = static_cast<int64_t>(DaysTo10000) * TicksPerDay - 1;
    constexpr int64_t UnixEpochTicks = static_cast<int64_t>(DaysTo1970) * TicksPerDay;

    static bool is_leap_year(int year)
    {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    static int days_in_month(int year, int month)
    {
        static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month == 2 && is_leap_year(year))
            return 29;
        return days[month - 1];
    }

    static int64_t date_to_ticks(int year, int month, int day)
    {
        if (year < 1 || year > 9999 || month < 1 || month > 12 || day < 1 || day > 31)
            throw std::runtime_error("Invalid date parameters");
        int max_day = days_in_month(year, month);
        if (day > max_day)
            throw std::runtime_error("Day is out of range for the month");
        int y = year - 1;
        int64_t numDays = (int64_t)y * 365 + y / 4 - y / 100 + y / 400;
        static const int d365[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
        static const int d366[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};
        numDays += is_leap_year(year) ? d366[month - 1] : d365[month - 1];
        numDays += day - 1;
        return numDays * TicksPerDay;
    }

    static void ticks_to_date(int64_t ticks, int &year, int &month, int &day)
    {
        if (ticks < 0)
            throw std::runtime_error("Ticks must be non-negative");
        int64_t days = ticks / TicksPerDay;
        int y400 = (int)(days / 146097);
        days -= y400 * 146097LL;
        int y100 = (int)(days / 36524);
        if (y100 == 4)
            y100 = 3;
        days -= y100 * 36524LL;
        int y4 = (int)(days / 1461);
        days -= y4 * 1461LL;
        int y1 = (int)(days / 365);
        if (y1 == 4)
            y1 = 3;
        days -= y1 * 365LL;
        year = y400 * 400 + y100 * 100 + y4 * 4 + y1 + 1;
        static const int d365[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
        static const int d366[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};
        const int *d2m = is_leap_year(year) ? d366 : d365;
        for (month = 1; month <= 12; month++)
        {
            if ((int)days < d2m[month])
            {
                day = (int)days - d2m[month - 1] + 1;
                return;
            }
        }
        month = 12;
        day = 31;
    }
}

/**
 * @brief 模仿 System.DateTimeKind，指示 DateTime 表示的是本地时间、UTC 时间还是未指定。
 *
 * - Unspecified: 时间未指定为本地或 UTC（默认）
 * - Utc: 协调世界时（UTC）
 * - Local: 本地时间
 */
enum class DateTimeKind
{
    Unspecified = 0,
    Utc = 1,
    Local = 2
};

class DateTime;

/**
 * @brief 模仿 System.TimeSpan 的时间间隔类。
 *
 * 表示一个时间间隔（以 100纳秒 为单位的刻度数），提供与 C# TimeSpan 一致的接口。
 *
 * 所有属性均使用 GetXxx() 命名（因为 C++ 没有属性语法）。
 *
 * 使用示例:
 * @code{.cpp}
 *   TimeSpan ts(1, 2, 30, 0);         // 1天2小时30分钟
 *   double days = ts.GetTotalDays();   // ≈ 1.104
 *   TimeSpan ts2 = TimeSpan::FromHours(3.5);
 * @endcode
 */
class TimeSpan
{
private:
    int64_t ticks_;

    static double ticks_to_double(int64_t ticks, int64_t ticksPerUnit)
    {
        return static_cast<double>(ticks) / static_cast<double>(ticksPerUnit);
    }

    static int64_t double_to_ticks(double value, double ticksPerUnit)
    {
        return static_cast<int64_t>(std::round(value * ticksPerUnit));
    }

public:
    /**
     * @brief 构造一个零值的 TimeSpan。
     */
    TimeSpan() : ticks_(0) {}

    /**
     * @brief 从指定的刻度数构造 TimeSpan。
     * @param ticks 时间间隔的刻度数（1 刻度 = 100 纳秒）
     */
    explicit TimeSpan(int64_t ticks) : ticks_(ticks) {}

    /**
     * @brief 从指定的小时、分钟和秒数构造 TimeSpan。
     * @param hours 小时（可正可负）
     * @param minutes 分钟
     * @param seconds 秒
     */
    TimeSpan(int hours, int minutes, int seconds)
        : ticks_(
              static_cast<int64_t>(hours) * detail::TicksPerHour +
              static_cast<int64_t>(minutes) * detail::TicksPerMinute +
              static_cast<int64_t>(seconds) * detail::TicksPerSecond) {}

    /**
     * @brief 从指定的天、小时、分钟和秒数构造 TimeSpan。
     * @param days 天
     * @param hours 小时
     * @param minutes 分钟
     * @param seconds 秒
     */
    TimeSpan(int days, int hours, int minutes, int seconds)
        : ticks_(
              static_cast<int64_t>(days) * detail::TicksPerDay +
              static_cast<int64_t>(hours) * detail::TicksPerHour +
              static_cast<int64_t>(minutes) * detail::TicksPerMinute +
              static_cast<int64_t>(seconds) * detail::TicksPerSecond) {}

    /**
     * @brief 从指定的天、小时、分钟、秒和毫秒数构造 TimeSpan。
     * @param days 天
     * @param hours 小时
     * @param minutes 分钟
     * @param seconds 秒
     * @param milliseconds 毫秒
     */
    TimeSpan(int days, int hours, int minutes, int seconds, int milliseconds)
        : ticks_(
              static_cast<int64_t>(days) * detail::TicksPerDay +
              static_cast<int64_t>(hours) * detail::TicksPerHour +
              static_cast<int64_t>(minutes) * detail::TicksPerMinute +
              static_cast<int64_t>(seconds) * detail::TicksPerSecond +
              static_cast<int64_t>(milliseconds) * detail::TicksPerMillisecond) {}

    /** @brief 获取此 TimeSpan 的刻度数（1 刻度 = 100 纳秒）。 */
    int64_t GetTicks() const { return ticks_; }
    /** @brief 获取此 TimeSpan 的天数部分。 */
    int GetDays() const { return static_cast<int>(ticks_ / detail::TicksPerDay); }
    /** @brief 获取此 TimeSpan 的小时数部分（0~23）。 */
    int GetHours() const { return static_cast<int>((ticks_ % detail::TicksPerDay) / detail::TicksPerHour); }
    /** @brief 获取此 TimeSpan 的分钟数部分（0~59）。 */
    int GetMinutes() const { return static_cast<int>((ticks_ % detail::TicksPerHour) / detail::TicksPerMinute); }
    /** @brief 获取此 TimeSpan 的秒数部分（0~59）。 */
    int GetSeconds() const { return static_cast<int>((ticks_ % detail::TicksPerMinute) / detail::TicksPerSecond); }
    /** @brief 获取此 TimeSpan 的毫秒数部分（0~999）。 */
    int GetMilliseconds() const { return static_cast<int>((ticks_ % detail::TicksPerSecond) / detail::TicksPerMillisecond); }

    /** @brief 获取以整天为单位表示的时间间隔值。 */
    double GetTotalDays() const { return ticks_to_double(ticks_, detail::TicksPerDay); }
    /** @brief 获取以整小时为单位表示的时间间隔值。 */
    double GetTotalHours() const { return ticks_to_double(ticks_, detail::TicksPerHour); }
    /** @brief 获取以整分钟为单位表示的时间间隔值。 */
    double GetTotalMinutes() const { return ticks_to_double(ticks_, detail::TicksPerMinute); }
    /** @brief 获取以整秒为单位表示的时间间隔值。 */
    double GetTotalSeconds() const { return ticks_to_double(ticks_, detail::TicksPerSecond); }
    /** @brief 获取以整毫秒为单位表示的时间间隔值。 */
    double GetTotalMilliseconds() const { return ticks_to_double(ticks_, detail::TicksPerMillisecond); }

    /** @brief 返回零值的 TimeSpan（静态属性）。 */
    static TimeSpan Zero() { return TimeSpan(0LL); }
    /** @brief 返回最小的 TimeSpan 可能值（静态属性）。 */
    static TimeSpan MinValue() { return TimeSpan(LLONG_MIN); }
    /** @brief 返回最大的 TimeSpan 可能值（静态属性）。 */
    static TimeSpan MaxValue() { return TimeSpan(LLONG_MAX); }

    /**
     * @brief 返回指定天数表示的 TimeSpan，精确到最接近的刻度。
     * @param d 天数（精确到微秒）
     */
    static TimeSpan FromDays(double d) { return TimeSpan(double_to_ticks(d, (double)detail::TicksPerDay)); }
    /**
     * @brief 返回指定小时数表示的 TimeSpan，精确到最接近的刻度。
     * @param h 小时数
     */
    static TimeSpan FromHours(double h) { return TimeSpan(double_to_ticks(h, (double)detail::TicksPerHour)); }
    /**
     * @brief 返回指定分钟数表示的 TimeSpan，精确到最接近的刻度。
     * @param m 分钟数
     */
    static TimeSpan FromMinutes(double m) { return TimeSpan(double_to_ticks(m, (double)detail::TicksPerMinute)); }
    /**
     * @brief 返回指定秒数表示的 TimeSpan，精确到最接近的刻度。
     * @param s 秒数
     */
    static TimeSpan FromSeconds(double s) { return TimeSpan(double_to_ticks(s, (double)detail::TicksPerSecond)); }
    /**
     * @brief 返回指定毫秒数表示的 TimeSpan，精确到最接近的刻度。
     * @param ms 毫秒数
     */
    static TimeSpan FromMilliseconds(double ms) { return TimeSpan(double_to_ticks(ms, (double)detail::TicksPerMillisecond)); }

    /**
     * @brief 比较两个 TimeSpan 值。
     * @param t1 第一个 TimeSpan
     * @param t2 第二个 TimeSpan
     * @return 如果 t1 < t2 则为 -1；如果相等则为 0；如果 t1 > t2 则为 1
     */
    static int Compare(const TimeSpan &t1, const TimeSpan &t2)
    {
        if (t1.ticks_ < t2.ticks_)
            return -1;
        if (t1.ticks_ > t2.ticks_)
            return 1;
        return 0;
    }

    /**
     * @brief 判断两个 TimeSpan 值是否相等。
     */
    static bool Equals(const TimeSpan &t1, const TimeSpan &t2) { return t1.ticks_ == t2.ticks_; }

    /**
     * @brief 将两个 TimeSpan 相加。
     * @param other 要添加的时间间隔
     * @return 当前对象与 other 之和
     */
    TimeSpan Add(const TimeSpan &other) const { return TimeSpan(ticks_ + other.ticks_); }
    /**
     * @brief 两个 TimeSpan 相减。
     * @param other 要减去的时间间隔
     * @return 当前对象减去 other 的结果
     */
    TimeSpan Subtract(const TimeSpan &other) const { return TimeSpan(ticks_ - other.ticks_); }

    /**
     * @brief 返回绝对值的新 TimeSpan。
     * @throw std::runtime_error 如果值为 MinValue（无法表示正值）
     */
    TimeSpan Duration() const
    {
        if (ticks_ == LLONG_MIN)
            throw std::runtime_error("Cannot compute Duration of TimeSpan::MinValue");
        return TimeSpan(ticks_ < 0 ? -ticks_ : ticks_);
    }

    /**
     * @brief 返回相反数的新 TimeSpan。
     * @throw std::runtime_error 如果值为 MinValue（无法表示正值）
     */
    TimeSpan Negate() const
    {
        if (ticks_ == LLONG_MIN)
            throw std::runtime_error("Cannot negate TimeSpan::MinValue");
        return TimeSpan(-ticks_);
    }

    /**
     * @brief 与另一个 TimeSpan 比较。
     * @return 如果当前实例 < other 则为 -1；相等则为 0；否则为 1
     */
    int CompareTo(const TimeSpan &other) const { return Compare(*this, other); }
    /**
     * @brief 判断当前 TimeSpan 与另一个是否相等。
     */
    bool Equals(const TimeSpan &other) const { return ticks_ == other.ticks_; }

    /**
     * @brief 将当前 TimeSpan 转换为标准字符串表示。
     *
     * 格式: [d.]hh:mm:ss[.fffffff]
     * - 当毫秒非零时显示毫秒部分
     * - 负值前面带有负号
     *
     * @return 格式化的时间间隔字符串
     */
    std::string ToString() const
    {
        if (ticks_ == LLONG_MIN)
            return "-10675199.02:48:05.4775808";
        if (ticks_ == LLONG_MAX)
            return "10675199.02:48:05.4775807";
        int64_t r = ticks_;
        std::string sign;
        if (r < 0)
        {
            sign = "-";
            r = -r;
        }
        int64_t d = r / detail::TicksPerDay;
        r %= detail::TicksPerDay;
        int h = (int)(r / detail::TicksPerHour);
        r %= detail::TicksPerHour;
        int m = (int)(r / detail::TicksPerMinute);
        r %= detail::TicksPerMinute;
        int s = (int)(r / detail::TicksPerSecond);
        r %= detail::TicksPerSecond;
        int ms = (int)(r / detail::TicksPerMillisecond);
        char buf[64];
        if (ms > 0)
            snprintf(buf, sizeof(buf), "%s%lld.%02d:%02d:%02d.%03d", sign.c_str(), (long long)d, h, m, s, ms);
        else
            snprintf(buf, sizeof(buf), "%s%lld.%02d:%02d:%02d", sign.c_str(), (long long)d, h, m, s);
        return std::string(buf);
    }

    /** @brief 两个 TimeSpan 相加。 */
    TimeSpan operator+(const TimeSpan &o) const { return Add(o); }
    /** @brief 两个 TimeSpan 相减。 */
    TimeSpan operator-(const TimeSpan &o) const { return Subtract(o); }
    /** @brief 返回相反数。 */
    TimeSpan operator-() const { return Negate(); }
    /** @brief 返回自身（一元正号）。 */
    TimeSpan operator+() const { return *this; }
    /** @brief 添加 TimeSpan 到当前实例。 */
    TimeSpan &operator+=(const TimeSpan &o)
    {
        ticks_ += o.ticks_;
        return *this;
    }
    /** @brief 从当前实例减去 TimeSpan。 */
    TimeSpan &operator-=(const TimeSpan &o)
    {
        ticks_ -= o.ticks_;
        return *this;
    }

    /** @brief 判断两个 TimeSpan 是否相等。 */
    bool operator==(const TimeSpan &o) const { return ticks_ == o.ticks_; }
    /** @brief 判断两个 TimeSpan 是否不等。 */
    bool operator!=(const TimeSpan &o) const { return ticks_ != o.ticks_; }
    /** @brief 判断当前 TimeSpan 是否小于另一个。 */
    bool operator<(const TimeSpan &o) const { return ticks_ < o.ticks_; }
    /** @brief 判断当前 TimeSpan 是否小于等于另一个。 */
    bool operator<=(const TimeSpan &o) const { return ticks_ <= o.ticks_; }
    /** @brief 判断当前 TimeSpan 是否大于另一个。 */
    bool operator>(const TimeSpan &o) const { return ticks_ > o.ticks_; }
    /** @brief 判断当前 TimeSpan 是否大于等于另一个。 */
    bool operator>=(const TimeSpan &o) const { return ticks_ >= o.ticks_; }

    /** @brief 将 TimeSpan 输出到流。 */
    friend std::ostream &operator<<(std::ostream &os, const TimeSpan &ts)
    {
        os << ts.ToString();
        return os;
    }
};

/**
 * @brief 模仿 System.DateTime 的日期时间类。
 *
 * 使用 int64_t ticks（100 纳秒间隔，自公元 1 年 1 月 1 日午夜起）作为内部存储，
 * 提供与 C# DateTime 近乎一致的接口和语义。
 *
 * 所有 C# 属性均映射为 GetXxx() 方法（因为 C++ 不支持属性语法）。
 *
 * 支持所有标准日期时间操作：
 * - 构造、格式化、解析
 * - 日期算术（AddDays/AddMonths/AddYears 等）
 * - 时间间隔运算（配合 TimeSpan）
 * - UTC / 本地时间转换
 * - 比较运算符
 *
 * 使用示例:
 * @code{.cpp}
 *   DateTime dt(2024, 6, 7, 14, 30, 0);
 *   std::cout << dt.ToString("yyyy-MM-dd HH:mm:ss"); // "2024-06-07 14:30:00"
 *   DateTime now = DateTime::Now();
 *   DateTime tomorrow = now.AddDays(1);
 * @endcode
 */
class DateTime
{
private:
    int64_t ticks_; // 100-nanosecond intervals since 1/1/0001
    DateTimeKind kind_;

    //DateTime ticks to Unix time_t
    std::time_t ticks_to_time_t(int64_t t) const
    {
        int64_t unix_ticks = t - detail::UnixEpochTicks;
        if (unix_ticks < 0)
            return 0;
        return static_cast<std::time_t>(unix_ticks / detail::TicksPerSecond);
    }

    int64_t time_t_to_ticks(std::time_t tt) const
    {
        return static_cast<int64_t>(tt) * detail::TicksPerSecond + detail::UnixEpochTicks;
    }

    int64_t time_of_day_ticks() const { return ticks_ % detail::TicksPerDay; }

    // Fallback: compute tm directly from ticks (handles any date 1/1/0001 - 12/31/9999)
    std::tm ticks_to_tm(int64_t t) const
    {
        std::tm tm = {};
        int y, m, d;
        detail::ticks_to_date(t, y, m, d);
        tm.tm_year = y - 1900;
        tm.tm_mon = m - 1;
        tm.tm_mday = d;
        int64_t tod = t % detail::TicksPerDay;
        tm.tm_hour = (int)(tod / detail::TicksPerHour);
        tod %= detail::TicksPerHour;
        tm.tm_min = (int)(tod / detail::TicksPerMinute);
        tod %= detail::TicksPerMinute;
        tm.tm_sec = (int)(tod / detail::TicksPerSecond);
        return tm;
    }

    std::tm get_tm_local() const
    {
        if (ticks_ < detail::UnixEpochTicks)
        {
            return ticks_to_tm(ticks_);
        }
        std::time_t tt = ticks_to_time_t(ticks_);
        std::tm tm;
#ifdef _WIN32
        if (localtime_s(&tm, &tt) != 0)
        {
            return ticks_to_tm(ticks_);
        }
#else
        if (localtime_r(&tt, &tm) == nullptr)
        {
            return ticks_to_tm(ticks_);
        }
#endif
        return tm;
    }

    std::tm get_tm_utc() const
    {
        if (ticks_ < detail::UnixEpochTicks)
        {
            return ticks_to_tm(ticks_);
        }
        std::time_t tt = ticks_to_time_t(ticks_);
        std::tm tm;
#ifdef _WIN32
        if (gmtime_s(&tm, &tt) != 0)
        {
            return ticks_to_tm(ticks_);
        }
#else
        if (gmtime_r(&tt, &tm) == nullptr)
        {
            return ticks_to_tm(ticks_);
        }
#endif
        return tm;
    }

    std::tm get_tm() const
    {
        return (kind_ == DateTimeKind::Utc) ? get_tm_utc() : get_tm_local();
    }

    static bool try_parse_csharp_format(const std::string &str, const std::string &format,
                                        int &year, int &month, int &day,
                                        int &hour, int &minute, int &second, int &millisecond,
                                        bool &is_pm)
    {
        // English weekday/month names for parsing text-based specifiers (ddd/dddd/MMM/MMMM)
        static const char *month_names_full[12] = {
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"};
        static const char *month_names_abbr[12] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        static const char *weekday_names_full[7] = {
            "Sunday", "Monday", "Tuesday", "Wednesday",
            "Thursday", "Friday", "Saturday"};
        static const char *weekday_names_abbr[7] = {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

        size_t str_idx = 0, fmt_idx = 0;

        auto match_name = [&](const char *const *names, int count, bool full) -> int {
            for (int i = 0; i < count; i++)
            {
                size_t len = strlen(names[i]);
                if (!full)
                    len = 3;
                if (str_idx + len <= str.size())
                {
                    bool ok = true;
                    for (size_t k = 0; k < len; k++)
                    {
                        if (std::toupper((unsigned char)str[str_idx + k]) !=
                            std::toupper((unsigned char)names[i][k]))
                        {
                            ok = false;
                            break;
                        }
                    }
                    if (ok)
                    {
                        str_idx += len;
                        return i;
                    }
                }
            }
            return -1;
        };
        is_pm = false;
        while (fmt_idx < format.length() && str_idx < str.length())
        {
            char fc = format[fmt_idx];
            if (fc == '\'')
            {
                fmt_idx++;
                size_t eq = format.find('\'', fmt_idx);
                if (eq == std::string::npos)
                    return false;
                while (fmt_idx < eq)
                {
                    if (str_idx >= str.size() || str[str_idx] != format[fmt_idx])
                        return false;
                    str_idx++;
                    fmt_idx++;
                }
                fmt_idx = eq + 1;
                continue;
            }
            if (fc == 'y' || fc == 'M' || fc == 'd' || fc == 'H' || fc == 'h' ||
                fc == 'm' || fc == 's' || fc == 'f' || fc == 't')
            {
                size_t cnt = 1;
                while (fmt_idx + cnt < format.size() && format[fmt_idx + cnt] == fc)
                    cnt++;
                if (str_idx >= str.size())
                    return false;

                // Handle text-based specifiers: ddd/dddd (weekday) and MMM/MMMM (month)
                if (fc == 'd' && cnt >= 3)
                {
                    int wd = match_name(weekday_names_abbr, 7, false);
                    if (wd < 0)
                        return false;
                    // For dddd (full name), try to match more; skip extra chars
                    if (cnt >= 4)
                    {
                        // Already matched 3 chars of abbreviation; try full match
                        str_idx -= 3; // backtrack to re-match full name
                        wd = match_name(weekday_names_full, 7, true);
                        if (wd < 0)
                            return false;
                    }
                    fmt_idx += cnt;
                    continue;
                }
                if (fc == 'M' && cnt >= 3)
                {
                    int mn = match_name(month_names_abbr, 12, false);
                    if (mn < 0)
                        return false;
                    if (cnt >= 4)
                    {
                        str_idx -= 3;
                        mn = match_name(month_names_full, 12, true);
                        if (mn < 0)
                            return false;
                    }
                    month = mn + 1;
                    fmt_idx += cnt;
                    continue;
                }

                if (fc != 't' && !std::isdigit((unsigned char)str[str_idx]))
                    return false;
                int val = 0;
                size_t dig = 0;
                size_t maxd = (fc == 'f') ? 7 : cnt;
                while (str_idx < str.size() && std::isdigit((unsigned char)str[str_idx]) && dig < maxd)
                {
                    val = val * 10 + (str[str_idx] - '0');
                    str_idx++;
                    dig++;
                }
                switch (fc)
                {
                case 'y':
                    if (cnt == 2)
                        year = (val < 30) ? 2000 + val : 1900 + val;
                    else if (cnt >= 4)
                    {
                        while (dig < 4)
                        {
                            val *= 10;
                            dig++;
                        }
                        year = val;
                    }
                    break;
                case 'M':
                    month = val;
                    if (month < 1 || month > 12)
                        return false;
                    break;
                case 'd':
                    day = val;
                    if (day < 1 || day > 31)
                        return false;
                    break;
                case 'H':
                    hour = val;
                    if (hour < 0 || hour > 23)
                        return false;
                    break;
                case 'h':
                    hour = val;
                    if (hour < 1 || hour > 12)
                        return false;
                    break;
                case 'm':
                    minute = val;
                    if (minute < 0 || minute > 59)
                        return false;
                    break;
                case 's':
                    second = val;
                    if (second < 0 || second > 59)
                        return false;
                    break;
                case 'f':
                    if (cnt <= 3)
                    {
                        while (dig < 3)
                        {
                            val *= 10;
                            dig++;
                        }
                        millisecond = val;
                    }
                    else if (cnt <= 6)
                    {
                        while (dig < 6)
                        {
                            val *= 10;
                            dig++;
                        }
                        millisecond = val / 1000;
                    }
                    else
                    {
                        while (dig < 7)
                        {
                            val *= 10;
                            dig++;
                        }
                        millisecond = val / 10000;
                    }
                    break;
                case 't':
                    if (cnt == 1 && str_idx < str.size())
                    {
                        char c = (char)std::toupper((unsigned char)str[str_idx]);
                        if (c == 'A')
                            is_pm = false;
                        else if (c == 'P')
                            is_pm = true;
                        else
                            return false;
                        str_idx++;
                    }
                    else if (cnt >= 2 && str_idx + 1 < str.size())
                    {
                        std::string ap = str.substr(str_idx, 2);
                        std::transform(ap.begin(), ap.end(), ap.begin(),
                                       [](unsigned char c)
                                       { return (char)std::toupper(c); });
                        if (ap == "AM")
                            is_pm = false;
                        else if (ap == "PM")
                            is_pm = true;
                        else
                            return false;
                        str_idx += 2;
                    }
                    break;
                }
                fmt_idx += cnt;
            }
            else
            {
                if (str_idx >= str.size() || str[str_idx] != fc)
                    return false;
                str_idx++;
                fmt_idx++;
            }
        }
        if (str_idx < str.size() || fmt_idx < format.size())
            return false;
        if (format.find('h') != std::string::npos && format.find('t') != std::string::npos)
        {
            if (is_pm && hour < 12)
                hour += 12;
            else if (!is_pm && hour == 12)
                hour = 0;
        }
        return true;
    }

    // 预定义格式字符串映射表（与 C# 一致）
    static std::string expand_predefined_format(const std::string &format)
    {
        if (format.size() != 1)
            return format;
        char c = format[0];
        switch (c)
        {
        case 'd': return "MM/dd/yyyy";
        case 'D': return "dddd, MMMM dd, yyyy";
        case 'f': return "dddd, MMMM dd, yyyy HH:mm";
        case 'F': return "dddd, MMMM dd, yyyy HH:mm:ss";
        case 'g': return "MM/dd/yyyy HH:mm";
        case 'G': return "MM/dd/yyyy HH:mm:ss";
        case 'm':
        case 'M': return "MMMM dd";
        case 'o':
        case 'O': return "yyyy-MM-ddTHH:mm:ss.fffffff";
        case 'r':
        case 'R': return "ddd, dd MMM yyyy HH:mm:ss 'GMT'";
        case 's': return "yyyy-MM-ddTHH:mm:ss";
        case 't': return "HH:mm";
        case 'T': return "HH:mm:ss";
        case 'u': return "yyyy-MM-dd HH:mm:ss'Z'";
        case 'U': return "dddd, MMMM dd, yyyy HH:mm:ss"; // 需要先 ToUniversalTime()
        case 'y':
        case 'Y': return "yyyy MMMM";
        default:  return format;
        }
    }

public:

    /**
     * @brief 默认构造函数。初始化为 DateTime::MinValue（0001-01-01），与 C# 一致。
     */
    DateTime() : ticks_(0), kind_(DateTimeKind::Unspecified) {}

    /**
     * @brief 从 std::time_t（Unix 时间戳）构造 DateTime。
     * @param t Unix 时间戳（自 1970-01-01 以来的秒数）
     * @note Kind 自动设为 Local。
     */
    explicit DateTime(std::time_t t)
        : ticks_(time_t_to_ticks(t)), kind_(DateTimeKind::Local) {}

    /**
     * @brief 从指定的 ticks 和 DateTimeKind 构造 DateTime。
     * @param ticks 自 0001-01-01 以来的 100 纳秒间隔数
     * @param kind DateTimeKind（默认为 Unspecified）
     * @throw std::runtime_error 如果 ticks 超出有效范围 [MinTicks, MaxTicks]
     */
    explicit DateTime(int64_t ticks, DateTimeKind kind = DateTimeKind::Unspecified)
        : ticks_(ticks), kind_(kind)
    {
        if (ticks_ < detail::MinTicks || ticks_ > detail::MaxTicks)
            throw std::runtime_error("Ticks out of valid DateTime range");
    }

    /**
     * @brief 从各时间分量构造 DateTime。
     * @param year 年（1~9999）
     * @param month 月（1~12）
     * @param day 日（1~31，会校验月份最大天数）
     * @param hour 时（0~23，默认 0）
     * @param minute 分（0~59，默认 0）
     * @param second 秒（0~59，默认 0）
     * @param millisecond 毫秒（0~999，默认 0）
     * @param kind DateTimeKind（默认为 Unspecified）
     * @throw std::runtime_error 如果任何参数超出有效范围
     */
    DateTime(int year, int month, int day,
             int hour = 0, int minute = 0, int second = 0, int millisecond = 0,
             DateTimeKind kind = DateTimeKind::Unspecified)
    {
        if (millisecond < 0 || millisecond > 999)
            throw std::runtime_error("Invalid datetime parameters: millisecond out of range");
        int64_t date_part = detail::date_to_ticks(year, month, day);
        int64_t time_part =
            (int64_t)hour * detail::TicksPerHour +
            (int64_t)minute * detail::TicksPerMinute +
            (int64_t)second * detail::TicksPerSecond +
            (int64_t)millisecond * detail::TicksPerMillisecond;
        if (time_part < 0 || time_part >= detail::TicksPerDay)
            throw std::runtime_error("Invalid datetime parameters: time out of range");
        ticks_ = date_part + time_part;
        kind_ = kind;
        if (ticks_ < detail::MinTicks || ticks_ > detail::MaxTicks)
            throw std::runtime_error("Invalid datetime parameters: result out of range");
    }

    /**
     * @brief 获取当前本地日期和时间（静态属性）。
     * @return 表示当前本地时间的 DateTime，Kind=Local。
     */
    static DateTime Now()
    {
        auto now = std::chrono::system_clock::now();
        auto tt = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        int64_t ticks = (int64_t)tt * detail::TicksPerSecond + detail::UnixEpochTicks + (ms % 1000) * detail::TicksPerMillisecond;
        return DateTime(ticks, DateTimeKind::Local);
    }

    /**
     * @brief 获取当前 UTC 日期和时间（静态属性）。
     * @return 表示当前 UTC 时间的 DateTime，Kind=Utc。
     */
    static DateTime UtcNow()
    {
        auto now = std::chrono::system_clock::now();
        auto tt = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        int64_t ticks = (int64_t)tt * detail::TicksPerSecond + detail::UnixEpochTicks + (ms % 1000) * detail::TicksPerMillisecond;
        return DateTime(ticks, DateTimeKind::Utc);
    }

    /**
     * @brief 获取当前日期（时间部分为 00:00:00）。
     * @return 表示当天日期的 DateTime，Kind=Local，时分秒为 0。
     */
    static DateTime Today()
    {
        DateTime n = Now();
        return DateTime(n.ticks_ - n.time_of_day_ticks(), DateTimeKind::Local);
    }

    /**
     * @brief 获取 DateTime 的最小可能值（0001-01-01）。
     */
    static DateTime MinValue() { return DateTime((int64_t)0, DateTimeKind::Unspecified); }
    /**
     * @brief 获取 DateTime 的最大可能值（9999-12-31 23:59:59.9999999）。
     */
    static DateTime MaxValue() { return DateTime(detail::MaxTicks, DateTimeKind::Unspecified); }

    /**
     * @brief 获取此 DateTime 的日期部分（时间部分设为 00:00:00）。
     * @return 一个新的 DateTime，其日期与当前实例相同，时间为午夜。
     */
    DateTime GetDate() const { return DateTime(ticks_ - time_of_day_ticks(), kind_); }
    /**
     * @brief 获取此 DateTime 当天的时间部分。
     * @return 一个 TimeSpan，表示自午夜以来的时间间隔。
     */
    TimeSpan GetTimeOfDay() const { return TimeSpan(time_of_day_ticks()); }
    /** @brief 获取此 DateTime 的刻度数（自 0001-01-01 以来的 100 纳秒间隔数）。 */
    int64_t GetTicks() const { return ticks_; }
    /** @brief 获取此 DateTime 的 DateTimeKind（Unspecified / Utc / Local）。 */
    DateTimeKind GetKind() const { return kind_; }

    /** @brief 获取年份分量（1~9999）。 */
    int GetYear() const
    {
        int y, m, d;
        detail::ticks_to_date(ticks_, y, m, d);
        return y;
    }
    /** @brief 获取月份分量（1~12）。 */
    int GetMonth() const
    {
        int y, m, d;
        detail::ticks_to_date(ticks_, y, m, d);
        return m;
    }
    /** @brief 获取日期分量（1~31）。 */
    int GetDay() const
    {
        int y, m, d;
        detail::ticks_to_date(ticks_, y, m, d);
        return d;
    }
    /** @brief 获取小时分量（0~23）。 */
    int GetHour() const { return (int)(time_of_day_ticks() / detail::TicksPerHour); }
    /** @brief 获取分钟分量（0~59）。 */
    int GetMinute() const
    {
        int64_t t = time_of_day_ticks() % detail::TicksPerHour;
        return (int)(t / detail::TicksPerMinute);
    }
    /** @brief 获取秒钟分量（0~59）。 */
    int GetSecond() const
    {
        int64_t t = time_of_day_ticks() % detail::TicksPerMinute;
        return (int)(t / detail::TicksPerSecond);
    }
    /** @brief 获取毫秒分量（0~999）。 */
    int GetMillisecond() const
    {
        int64_t t = time_of_day_ticks() % detail::TicksPerSecond;
        return (int)(t / detail::TicksPerMillisecond);
    }

    /**
     * @brief 获取此 DateTime 表示的星期几。
     * @return 0=Sunday, 1=Monday, ..., 6=Saturday（与 C# DayOfWeek 枚举一致）
     */
    int GetDayOfWeek() const
    {
        // 1/1/0001 = Monday = 1, DayOfWeek: Sunday=0
        int64_t days = ticks_ / detail::TicksPerDay;
        return (int)((days + 1) % 7);
    }

    /**
     * @brief 获取此 DateTime 在一年中的第几天。
     * @return 1 到 366 之间的整数（1 月 1 日为 1）
     */
    int GetDayOfYear() const
    {
        int y, m, d;
        detail::ticks_to_date(ticks_, y, m, d);
        for (int i = 1; i < m; i++)
            d += detail::days_in_month(y, i);
        return d;
    }

    /**
     * @brief 转换为 std::time_t（Unix 时间戳）。
     * @return 自 1970-01-01 00:00:00 UTC 以来的秒数。
     * @note 对于 1970 年之前的日期返回 0。
     */
    std::time_t ToTime_t() const { return ticks_to_time_t(ticks_); }

    /**
     * @brief 使用指定的格式将当前 DateTime 转换为字符串。
     *
     * 支持自定义格式说明符和预定义简写格式说明符（与 C# 一致）。
     *
     * ### 预定义格式简写（单字符）
     *
     * | 说明符 | 含义                | 展开格式                          |
     * |--------|---------------------|-----------------------------------|
     * | d      | 短日期              | MM/dd/yyyy                       |
     * | D      | 长日期              | dddd, MMMM dd, yyyy              |
     * | f      | 完整日期+短时间      | dddd, MMMM dd, yyyy HH:mm        |
     * | F      | 完整日期+长时间      | dddd, MMMM dd, yyyy HH:mm:ss     |
     * | g      | 常规日期+短时间      | MM/dd/yyyy HH:mm                 |
     * | G      | 常规日期+长时间      | MM/dd/yyyy HH:mm:ss              |
     * | M / m  | 月/日               | MMMM dd                          |
     * | O / o  | 往返日期/时间       | yyyy-MM-ddTHH:mm:ss.fffffff      |
     * | R / r  | RFC1123             | ddd, dd MMM yyyy HH:mm:ss 'GMT'  |
     * | s      | 可排序              | yyyy-MM-ddTHH:mm:ss              |
     * | t      | 短时间              | HH:mm                            |
     * | T      | 长时间              | HH:mm:ss                         |
     * | u      | 通用可排序          | yyyy-MM-dd HH:mm:ss'Z'           |
     * | U      | 通用完整            | 先 ToUniversalTime() 再使用 F 格式 |
     * | Y / y  | 年/月               | yyyy MMMM                        |
     *
     * 使用示例:
     * @code{.cpp}
     *   dt.ToString("d");  // "06/07/2024"
     *   dt.ToString("D");  // "Friday, June 07, 2024"
     *   dt.ToString("o");  // "2024-06-07T14:05:03.7890000"
     *   dt.ToString("s");  // "2024-06-07T14:05:03"
     * @endcode
     *
     * ### 自定义格式说明符
     * |--------|----------------|-------------------|
     * | y      | 短年份（不补零）| 24                |
     * | yy     | 短年份（补零）  | 24                |
     * | yyyy   | 完整年份        | 2024              |
     * | M      | 数字月（不补零）| 6                 |
     * | MM     | 数字月（补零）  | 06                |
     * | MMM    | 缩写月份名     | Jun               |
     * | MMMM   | 完整月份名     | June              |
     * | d      | 日（不补零）    | 7                 |
     * | dd     | 日（补零）      | 07                |
     * | ddd    | 缩写星期名     | Fri               |
     * | dddd   | 完整星期名     | Friday            |
     * | H      | 24小时（不补零）| 14                |
     * | HH     | 24小时（补零）  | 14                |
     * | h      | 12小时（不补零）| 2                 |
     * | hh     | 12小时（补零）  | 02                |
     * | m      | 分钟（不补零）  | 5                 |
     * | mm     | 分钟（补零）    | 05                |
     * | s      | 秒（不补零）    | 3                 |
     * | ss     | 秒（补零）      | 03                |
     * | f/ff/fff | 毫秒        | 7 / 78 / 789      |
     * | t      | AM/PM 首字符   | A/P  (注1)        |
     * | tt     | AM/PM 完整     | AM/PM             |
     * | 'text' | 转义文本       | '年' → 年         |
     *
     * 注1: 由于 strftime 限制，单个 't' 与 'tt' 行为相同（显示完整 AM/PM）。
     *
     * 使用示例:
     * @code{.cpp}
     *   DateTime dt(2024, 6, 7, 14, 5, 3, 789);
     *   dt.ToString("yyyy-MM-dd HH:mm:ss");    // "2024-06-07 14:05:03"
     *   dt.ToString("hh:mm tt");               // "02:05 PM"
     *   dt.ToString("dddd, MMMM d, yyyy");     // "Friday, June 7, 2024"
     *   dt.ToString("yyyy'年'MM'月'dd'日'");   // "2024年06月07日"
     *   dt.ToString("yyyy-MM-dd HH:mm:ss.fff"); // "2024-06-07 14:05:03.789"
     * @endcode
     *
     * @param format C# 风格的格式字符串（默认为 "yyyy-MM-dd HH:mm:ss"）
     * @return 格式化的日期时间字符串
     * @throw std::runtime_error 如果格式字符串包含未闭合的引号
     */
    std::string ToString(const std::string &format = "yyyy-MM-dd HH:mm:ss") const
    {
        if (format.empty())
            return ToString("yyyy-MM-dd HH:mm:ss");
        // 展开预定义格式简写
        std::string fmt = expand_predefined_format(format);
        // 特殊处理 "U"（Universal full）— 先转换 UTC 再使用 "F" 格式
        if (format.size() == 1 && format[0] == 'U')
        {
            return ToUniversalTime().ToString("F");
        }
        try
        {
            std::tm tm = get_tm();
            std::string result;
            for (size_t i = 0; i < fmt.size();)
            {
                char c = fmt[i];
                if (c == '\'')
                {
                    i++;
                    size_t eq = fmt.find('\'', i);
                    if (eq == std::string::npos)
                        throw std::runtime_error("Unclosed quote");
                    result += fmt.substr(i, eq - i);
                    i = eq + 1;
                }
                else if (c == 'y' || c == 'M' || c == 'd' || c == 'H' || c == 'h' || c == 'm' || c == 's' || c == 't')
                {
                    size_t cnt = 1;
                    while (i + cnt < fmt.size() && fmt[i + cnt] == c)
                        cnt++;
                    const char *strf = "";
                    switch (c)
                    {
                    case 'y':
                        strf = (cnt >= 4) ? "%Y" : "%y";
                        break;
                    case 'M':
                        strf = (cnt >= 4) ? "%B" : (cnt == 3 ? "%b" : "%m");
                        break;
                    case 'd':
                        strf = (cnt >= 4) ? "%A" : (cnt == 3 ? "%a" : "%d");
                        break;
                    case 'H':
                        strf = "%H";
                        break;
                    case 'h':
                        strf = "%I";
                        break;
                    case 'm':
                        strf = "%M";
                        break;
                    case 's':
                        strf = "%S";
                        break;
                    case 't':
                        strf = "%p";
                        break;
                    }
                    char buf[64] = {};
                    std::strftime(buf, sizeof(buf), strf, &tm);
                    result += buf;
                    i += cnt;
                }
                else if (c == 'f')
                {
                    size_t cnt = 1;
                    while (i + cnt < fmt.size() && fmt[i + cnt] == 'f')
                        cnt++;
                    int ms = GetMillisecond();
                    char buf[16] = {};
                    if (cnt <= 3)
                    {
                        if (cnt == 1)
                            snprintf(buf, sizeof(buf), "%d", ms / 100);
                        else if (cnt == 2)
                            snprintf(buf, sizeof(buf), "%02d", ms / 10);
                        else
                            snprintf(buf, sizeof(buf), "%03d", ms);
                    }
                    else if (cnt <= 6)
                    {
                        snprintf(buf, sizeof(buf), "%06d", ms * 1000);
                        result += std::string(buf).substr(0, cnt);
                        i += cnt;
                        continue;
                    }
                    else
                    {
                        snprintf(buf, sizeof(buf), "%07d", ms * 10000);
                        result += std::string(buf).substr(0, cnt);
                        i += cnt;
                        continue;
                    }
                    result += buf;
                    i += cnt;
                }
                else
                {
                    result += c;
                    i++;
                }
            }
            return result;
        }
        catch (...)
        {
            throw;
        }
    }

    /** @brief 返回短日期字符串（格式: yyyy-MM-dd）。 */
    std::string ToShortDateString() const { return ToString("yyyy-MM-dd"); }
    /** @brief 返回长日期字符串（格式: dddd, MMMM dd, yyyy，例如 "Friday, June 07, 2024"）。 */
    std::string ToLongDateString() const { return ToString("dddd, MMMM dd, yyyy"); }
    /** @brief 返回短时间字符串（格式: HH:mm）。 */
    std::string ToShortTimeString() const { return ToString("HH:mm"); }
    /** @brief 返回长时间字符串（格式: HH:mm:ss）。 */
    std::string ToLongTimeString() const { return ToString("HH:mm:ss"); }

    /**
     * @brief 将日期时间字符串解析为 DateTime，支持自动格式检测或指定格式。
     *
     * 自动检测的格式（当 format 为空时按顺序尝试）：
     * - yyyy-MM-dd HH:mm:ss
     * - yyyy-MM-dd HH:mm
     * - yyyy-MM-dd
     * - MM/dd/yyyy HH:mm:ss
     * - MM/dd/yyyy HH:mm
     * - MM/dd/yyyy
     * - dd/MM/yyyy HH:mm:ss
     * - dd/MM/yyyy HH:mm
     * - dd/MM/yyyy
     * - yyyy/MM/dd HH:mm:ss
     * - yyyy/MM/dd HH:mm
     * - yyyy/MM/dd
     * - yyyy-MM-ddTHH:mm:ss.fffffff (往返格式 o)
     * - yyyy-MM-ddTHH:mm:ss (可排序格式 s)
     * - yyyy-MM-dd HH:mm:ss'Z' (通用可排序 u)
     * - ddd, dd MMM yyyy HH:mm:ss 'GMT' (RFC1123 R)
     * - dddd, MMMM dd, yyyy HH:mm:ss (完整格式 F)
     * - dddd, MMMM dd, yyyy HH:mm (完整短时间 f)
     * - dddd, MMMM dd, yyyy (长日期 D)
     * - MMMM dd (月/日 M)
     * - yyyy MMMM (年/月 Y)
     *
     * 使用示例:
     * @code{.cpp}
     *   // 自动检测格式
     *   DateTime dt1 = DateTime::Parse("2024-06-07 14:30:00");
     *   DateTime dt2 = DateTime::Parse("06/07/2024");
     *
     *   // 指定格式
     *   DateTime dt3 = DateTime::Parse("2024年06月07日", "yyyy'年'MM'月'dd'日'");
     *   DateTime dt4 = DateTime::Parse("06/07/2024 02:30:00 PM", "MM/dd/yyyy hh:mm:ss tt");
     * @endcode
     *
     * @param str 要解析的日期时间字符串
     * @param format C# 风格格式字符串（为空时自动检测）
     * @return 解析后的 DateTime
     * @throw std::runtime_error 如果无法解析
     */
    static DateTime Parse(const std::string &str, const std::string &format = "")
    {
        // 展开预定义格式简写
        std::string fmt = expand_predefined_format(format);
        if (fmt.empty())
        {
            static const char *fmts[] = {
                "yyyy-MM-dd HH:mm:ss",
                "yyyy-MM-dd HH:mm",
                "yyyy-MM-dd",
                "MM/dd/yyyy HH:mm:ss",
                "MM/dd/yyyy HH:mm",
                "MM/dd/yyyy",
                "dd/MM/yyyy HH:mm:ss",
                "dd/MM/yyyy HH:mm",
                "dd/MM/yyyy",
                "yyyy/MM/dd HH:mm:ss",
                "yyyy/MM/dd HH:mm",
                "yyyy/MM/dd",
                "yyyy-MM-ddTHH:mm:ss.fffffff",
                "yyyy-MM-ddTHH:mm:ss",
                "yyyy-MM-dd HH:mm:ss'Z'",
                "ddd, dd MMM yyyy HH:mm:ss 'GMT'",
                "dddd, MMMM dd, yyyy HH:mm:ss",
                "dddd, MMMM dd, yyyy HH:mm",
                "dddd, MMMM dd, yyyy",
                "MMMM dd",
                "yyyy MMMM"};
            for (auto f : fmts)
            {
                try
                {
                    return Parse(str, f);
                }
                catch (...)
                {
                }
            }
            throw std::runtime_error("Unable to parse the datetime string: " + str);
        }
        int year = 1900, month = 1, day = 1, hour = 0, minute = 0, second = 0, ms = 0;
        bool is_pm = false;
        if (try_parse_csharp_format(str, fmt, year, month, day, hour, minute, second, ms, is_pm))
            return DateTime(year, month, day, hour, minute, second, ms);
        throw std::runtime_error("Unable to parse \"" + str + "\" with format \"" + fmt + "\"");
    }

    /**
     * @brief 安全地尝试解析日期时间字符串。
     *
     * 使用示例:
     * @code{.cpp}
     *   DateTime result;
     *   if (DateTime::TryParse("2024-06-07", result)) {
     *       // 解析成功
     *   } else {
     *       // 解析失败
     *   }
     * @endcode
     *
     * @param str 要解析的字符串
     * @param result [out] 解析成功时输出的 DateTime
     * @param format C# 风格格式字符串（为空时自动检测）
     * @return true 如果解析成功，false 否则
     */
    static bool TryParse(const std::string &str, DateTime &result, const std::string &format = "")
    {
        try
        {
            result = Parse(str, format);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    /**
     * @brief 将指定的 TimeSpan 添加到当前 DateTime。
     * @param value 要添加的时间间隔
     * @return 新的 DateTime
     * @throw std::runtime_error 如果结果超出有效范围
     */
    DateTime Add(const TimeSpan &value) const
    {
        int64_t nt = ticks_ + value.GetTicks();
        if (nt < detail::MinTicks || nt > detail::MaxTicks)
            throw std::runtime_error("DateTime arithmetic out of range");
        return DateTime(nt, kind_);
    }

    /** @brief 返回一个新的 DateTime，将当前对象加上指定天数。 */
    DateTime AddDays(int d) const { return Add(TimeSpan(d, 0, 0, 0)); }
    /** @brief 返回一个新的 DateTime，将当前对象加上指定小时数。 */
    DateTime AddHours(int h) const { return Add(TimeSpan(0, h, 0, 0)); }
    /** @brief 返回一个新的 DateTime，将当前对象加上指定分钟数。 */
    DateTime AddMinutes(int m) const { return Add(TimeSpan(0, 0, m, 0)); }
    /** @brief 返回一个新的 DateTime，将当前对象加上指定秒数。 */
    DateTime AddSeconds(int s) const { return Add(TimeSpan(0, 0, s)); }
    /** @brief 返回一个新的 DateTime，将当前对象加上指定毫秒数。 */
    DateTime AddMilliseconds(int ms) const { return Add(TimeSpan(0, 0, 0, 0, ms)); }

    /**
     * @brief 返回一个新的 DateTime，将当前对象加上指定月数。
     *
     * 如果结果日期超出目标月份的天数，则自动调整到月末。
     * 例如: 1月31日 + 1个月 = 2月28/29日。
     *
     * @param months 月数（可为负数）
     * @throw std::runtime_error 如果结果超出有效范围
     */
    DateTime AddMonths(int months) const
    {
        int y, m, d;
        detail::ticks_to_date(ticks_, y, m, d);
        m += months;
        while (m > 12)
        {
            m -= 12;
            y++;
        }
        while (m < 1)
        {
            m += 12;
            y--;
        }
        int md = detail::days_in_month(y, m);
        if (d > md)
            d = md;
        int64_t nt = detail::date_to_ticks(y, m, d) + time_of_day_ticks();
        if (nt < detail::MinTicks || nt > detail::MaxTicks)
            throw std::runtime_error("DateTime arithmetic out of range");
        return DateTime(nt, kind_);
    }

    /**
     * @brief 返回一个新的 DateTime，将当前对象加上指定年数。
     *
     * 如果当前日期为闰年 2 月 29 日，且目标年份不是闰年，则自动调整为 2 月 28 日。
     *
     * @param years 年数（可为负数）
     * @throw std::runtime_error 如果结果超出有效范围
     */
    DateTime AddYears(int years) const
    {
        int y, m, d;
        detail::ticks_to_date(ticks_, y, m, d);
        y += years;
        if (m == 2 && d == 29 && !detail::is_leap_year(y))
            d = 28;
        int64_t nt = detail::date_to_ticks(y, m, d) + time_of_day_ticks();
        if (nt < detail::MinTicks || nt > detail::MaxTicks)
            throw std::runtime_error("DateTime arithmetic out of range");
        return DateTime(nt, kind_);
    }

    /** @brief 从此 DateTime 减去一个 TimeSpan，返回新的 DateTime。 */
    DateTime Subtract(const TimeSpan &ts) const { return Add(TimeSpan(-ts.GetTicks())); }
    /** @brief 从此 DateTime 减去另一个 DateTime，返回 TimeSpan 差。 */
    TimeSpan Subtract(const DateTime &dt) const { return TimeSpan(ticks_ - dt.ticks_); }

    /**
     * @brief 将当前 DateTime 转换为 UTC 时间。
     *
     * - 如果 Kind 为 Utc，则返回自身。
     * - 如果 Kind 为 Local 或 Unspecified，假定当前值为本地时间并转换为 UTC。
     *
     * @return 表示 UTC 时间的 DateTime，Kind=Utc
     */
    DateTime ToUniversalTime() const
    {
        if (kind_ == DateTimeKind::Utc)
            return *this;
        std::tm ltm = get_tm_local();
        std::time_t tt = std::mktime(&ltm);
        if (tt == -1)
            return DateTime(ticks_, DateTimeKind::Utc);
#ifdef _WIN32
        std::tm utm;
        if (gmtime_s(&utm, &tt) != 0)
            return DateTime(ticks_, DateTimeKind::Utc);
#else
        std::tm utm;
        if (gmtime_r(&tt, &utm) == nullptr)
            return DateTime(ticks_, DateTimeKind::Utc);
#endif
        int64_t ut = detail::date_to_ticks(utm.tm_year + 1900, utm.tm_mon + 1, utm.tm_mday) + (int64_t)utm.tm_hour * detail::TicksPerHour + (int64_t)utm.tm_min * detail::TicksPerMinute + (int64_t)utm.tm_sec * detail::TicksPerSecond + (ticks_ % detail::TicksPerSecond);
        return DateTime(ut, DateTimeKind::Utc);
    }

    /**
     * @brief 将当前 DateTime 转换为本地时间。
     *
     * - 如果 Kind 为 Local，则返回自身。
     * - 如果 Kind 为 Utc 或 Unspecified，假定当前值为 UTC 时间并转换为本地时间。
     *
     * @return 表示本地时间的 DateTime，Kind=Local
     */
    DateTime ToLocalTime() const
    {
        if (kind_ == DateTimeKind::Local)
            return *this;
        std::tm utm = get_tm_utc();
        utm.tm_isdst = -1;
        std::time_t tt = std::mktime(&utm);
        if (tt == -1)
            return DateTime(ticks_, DateTimeKind::Local);
#ifdef _WIN32
        std::tm ltm;
        if (localtime_s(&ltm, &tt) != 0)
            return DateTime(ticks_, DateTimeKind::Local);
#else
        std::tm ltm;
        if (localtime_r(&tt, &ltm) == nullptr)
            return DateTime(ticks_, DateTimeKind::Local);
#endif
        int64_t lt = detail::date_to_ticks(ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday) + (int64_t)ltm.tm_hour * detail::TicksPerHour + (int64_t)ltm.tm_min * detail::TicksPerMinute + (int64_t)ltm.tm_sec * detail::TicksPerSecond + (ticks_ % detail::TicksPerSecond);
        return DateTime(lt, DateTimeKind::Local);
    }

    /**
     * @brief 比较两个 DateTime 值。
     * @return 如果 a < b 则为 -1；相等则为 0；如果 a > b 则为 1
     */
    static int Compare(const DateTime &a, const DateTime &b)
    {
        if (a.ticks_ < b.ticks_)
            return -1;
        if (a.ticks_ > b.ticks_)
            return 1;
        return 0;
    }
    /** @brief 判断两个 DateTime 是否相等。 */
    static bool Equals(const DateTime &a, const DateTime &b) { return a.ticks_ == b.ticks_; }
    /**
     * @brief 返回指定年份和月份的日期数。
     * @param year 年份
     * @param month 月份（1~12）
     * @return 该月的天数（28~31）
     * @throw std::runtime_error 如果月份不在 1~12 范围内
     */
    static int DaysInMonth(int year, int month)
    {
        if (month < 1 || month > 12)
            throw std::runtime_error("Month must be 1-12");
        return detail::days_in_month(year, month);
    }
    /**
     * @brief 判断指定年份是否为闰年。
     * @return true 如果为闰年
     */
    static bool IsLeapYear(int year) { return detail::is_leap_year(year); }
    /**
     * @brief 返回一个新的 DateTime，其 Kind 设置为指定的值（ticks 不变）。
     * @param v 原始 DateTime
     * @param k 新的 DateTimeKind
     * @return 具有相同 ticks 但 Kind 改变的新 DateTime
     */
    static DateTime SpecifyKind(const DateTime &v, DateTimeKind k) { return DateTime(v.ticks_, k); }

    /** @brief 与另一个 DateTime 比较。返回 -1、0 或 1。 */
    int CompareTo(const DateTime &o) const { return Compare(*this, o); }
    /** @brief 判断当前 DateTime 与另一个是否相等。 */
    bool Equals(const DateTime &o) const { return ticks_ == o.ticks_; }

    /** @brief 判断两个 DateTime 是否相等。 */
    bool operator==(const DateTime &o) const { return ticks_ == o.ticks_; }
    /** @brief 判断两个 DateTime 是否不等。 */
    bool operator!=(const DateTime &o) const { return ticks_ != o.ticks_; }
    /** @brief 判断当前 DateTime 是否小于另一个。 */
    bool operator<(const DateTime &o) const { return ticks_ < o.ticks_; }
    /** @brief 判断当前 DateTime 是否小于等于另一个。 */
    bool operator<=(const DateTime &o) const { return ticks_ <= o.ticks_; }
    /** @brief 判断当前 DateTime 是否大于另一个。 */
    bool operator>(const DateTime &o) const { return ticks_ > o.ticks_; }
    /** @brief 判断当前 DateTime 是否大于等于另一个。 */
    bool operator>=(const DateTime &o) const { return ticks_ >= o.ticks_; }

    /** @brief DateTime + TimeSpan，返回新的 DateTime。 */
    DateTime operator+(const TimeSpan &ts) const { return Add(ts); }
    /** @brief DateTime - TimeSpan，返回新的 DateTime。 */
    DateTime operator-(const TimeSpan &ts) const { return Subtract(ts); }
    /** @brief DateTime - DateTime，返回 TimeSpan 差。 */
    TimeSpan operator-(const DateTime &o) const { return Subtract(o); }
    /** @brief 将 TimeSpan 添加到当前 DateTime。 */
    DateTime &operator+=(const TimeSpan &ts)
    {
        *this = Add(ts);
        return *this;
    }
    /** @brief 从当前 DateTime 减去 TimeSpan。 */
    DateTime &operator-=(const TimeSpan &ts)
    {
        *this = Subtract(ts);
        return *this;
    }

    /** @brief 将 DateTime 输出到流（使用默认格式 yyyy-MM-dd HH:mm:ss）。 */
    friend std::ostream &operator<<(std::ostream &os, const DateTime &dt)
    {
        os << dt.ToString();
        return os;
    }

    /**
     * @brief 从输入流读取 DateTime 字符串。
     *
     * 使用 TryParse 进行自动格式检测。如果解析失败，设置流的 failbit。
     *
     * 使用示例:
     * @code{.cpp}
     *   DateTime dt;
     *   std::cin >> dt;  // 输入 "2024-06-07 14:30:00"
     * @endcode
     */
    friend std::istream &operator>>(std::istream &is, DateTime &dt)
    {
        std::string s;
        if (!(is >> s))
            return is;
        DateTime r;
        if (TryParse(s, r))
            dt = r;
        else
            is.setstate(std::ios::failbit);
        return is;
    }

    /** @brief TimeSpan + DateTime，返回新的 DateTime。 */
    friend DateTime operator+(const TimeSpan &ts, const DateTime &dt) { return dt.Add(ts); }
};

#endif // __INC_GL_DATETIME_
