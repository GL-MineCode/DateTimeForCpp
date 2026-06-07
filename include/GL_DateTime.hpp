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

/**
 * @brief A imitation of the C# DateTime class, with nearly identical functionality.
 * @note One of the few differences: the format specifier single 't' (for AM/PM indicator) is not supported in ToString(). This class treats a single 't' as two 't's, which displays AM for morning and PM for afternoon.
*/
class DateTime {
private:
    std::chrono::system_clock::time_point time_point_;
    
    static std::chrono::system_clock::time_point tm_to_time_point(const std::tm& tm) {
        std::time_t tt = std::mktime(const_cast<std::tm*>(&tm));
        if (tt == -1) {
            throw std::runtime_error("Invalid time structure");
        }
        return std::chrono::system_clock::from_time_t(tt);
    }
    
    std::tm get_tm() const {
        std::time_t tt = std::chrono::system_clock::to_time_t(time_point_);
        std::tm tm;
#ifdef _WIN32
        if (localtime_s(&tm, &tt) != 0) {
            throw std::runtime_error("Failed to convert to local time");
        }
#else
        if (localtime_r(&tt, &tm) == nullptr) {
            throw std::runtime_error("Failed to convert to local time");
        }
#endif
        return tm;
    }
    
    std::tm get_tm_utc() const {
        std::time_t tt = std::chrono::system_clock::to_time_t(time_point_);
        std::tm tm;
#ifdef _WIN32
        if (gmtime_s(&tm, &tt) != 0) {
            throw std::runtime_error("Failed to convert to UTC time");
        }
#else
        if (gmtime_r(&tt, &tm) == nullptr) {
            throw std::runtime_error("Failed to convert to UTC time");
        }
#endif
        return tm;
    }
    
    static bool try_parse_csharp_format(const std::string& str, const std::string& format, 
                                        int& year, int& month, int& day, 
                                        int& hour, int& minute, int& second, int& millisecond,
                                        bool& is_pm) {
        size_t str_idx = 0;
        size_t fmt_idx = 0;
        is_pm = false;
        
        while (fmt_idx < format.length() && str_idx < str.length()) {
            char fmt_char = format[fmt_idx];
            
            if (fmt_char == '\'') {
                fmt_idx++;
                size_t end_quote = format.find('\'', fmt_idx);
                if (end_quote == std::string::npos) {
                    return false;
                }
                
                for (size_t i = fmt_idx; i < end_quote; i++) {
                    if (str_idx >= str.length() || str[str_idx] != format[i]) {
                        return false;
                    }
                    str_idx++;
                }
                fmt_idx = end_quote + 1;
                continue;
            }
            
            if (fmt_char == 'y' || fmt_char == 'M' || fmt_char == 'd' || 
                fmt_char == 'H' || fmt_char == 'h' || fmt_char == 'm' ||
                fmt_char == 's' || fmt_char == 'f' || fmt_char == 't') {
                
                size_t count = 1;
                while (fmt_idx + count < format.length() && format[fmt_idx + count] == fmt_char) {
                    count++;
                }
                
                if (str_idx >= str.length()) {
                    return false;
                }
                // 't' (AM/PM) reads letters, not digits
                if (fmt_char != 't' && !std::isdigit(static_cast<unsigned char>(str[str_idx]))) {
                    return false;
                }
                
                int value = 0;
                size_t digits = 0;
                
                size_t max_digits = (fmt_char == 'f') ? 7 : count;
                
                while (str_idx < str.length() && std::isdigit(str[str_idx]) && digits < max_digits) {
                    value = value * 10 + (str[str_idx] - '0');
                    str_idx++;
                    digits++;
                }
                
                switch (fmt_char) {
                    case 'y':
                        if (count == 2) {
                            year = (value < 30) ? 2000 + value : 1900 + value;
                        } else if (count >= 4) {
                            if (digits == 4) {
                                year = value;
                            } else {
                                while (digits < 4) {
                                    value *= 10;
                                    digits++;
                                }
                                year = value;
                            }
                        }
                        break;
                        
                    case 'M':
                        month = value;
                        if (month < 1 || month > 12) return false;
                        break;
                        
                    case 'd':
                        day = value;
                        if (day < 1 || day > 31) return false;
                        break;
                        
                    case 'H':
                        hour = value;
                        if (hour < 0 || hour > 23) return false;
                        break;
                        
                    case 'h':
                        hour = value;
                        if (hour < 1 || hour > 12) return false;
                        break;
                        
                    case 'm':
                        minute = value;
                        if (minute < 0 || minute > 59) return false;
                        break;
                        
                    case 's':
                        second = value;
                        if (second < 0 || second > 59) return false;
                        break;
                        
                    case 'f':
                        if (count <= 3) {
                            for (size_t i = digits; i < 3; i++) {
                                value *= 10;
                            }
                            millisecond = value;
                        } else if (count <= 6) {
                            for (size_t i = digits; i < 6; i++) {
                                value *= 10;
                            }
                            millisecond = value / 1000;
                        } else {
                            for (size_t i = digits; i < 7; i++) {
                                value *= 10;
                            }
                            millisecond = value / 10000;
                        }
                        break;
                        
                    case 't':
                        if (count == 1) {
                            if (str_idx < str.length()) {
                                char ampm = std::toupper(str[str_idx]);
                                if (ampm == 'A') {
                                    is_pm = false;
                                } else if (ampm == 'P') {
                                    is_pm = true;
                                } else {
                                    return false;
                                }
                                str_idx++;
                            }
                        } else if (count == 2) {
                            if (str_idx + 1 < str.length()) {
                                std::string ampm = str.substr(str_idx, 2);
                                std::transform(ampm.begin(), ampm.end(), ampm.begin(), ::toupper);
                                if (ampm == "AM") {
                                    is_pm = false;
                                } else if (ampm == "PM") {
                                    is_pm = true;
                                } else {
                                    return false;
                                }
                                str_idx += 2;
                            }
                        }
                        break;
                }
                
                fmt_idx += count;
            } else {
                
                if (str_idx >= str.length() || str[str_idx] != fmt_char) {
                    return false;
                }
                str_idx++;
                fmt_idx++;
            }
        }
        
        if (str_idx < str.length() || fmt_idx < format.length()) {
            return false;
        }
        
        if (format.find('h') != std::string::npos) {
            if (format.find('t') != std::string::npos) {
                if (is_pm && hour < 12) {
                    hour += 12;
                } else if (!is_pm && hour == 12) {
                    hour = 0;
                }
            }
        }
        
        return true;
    }
    
    static std::string csharp_to_strftime_format(const std::string& csharp_format) {
        std::string result;
        size_t i = 0;
        
        while (i < csharp_format.length()) {
            char c = csharp_format[i];
            
            if (c == '\'') {
                i++;
                size_t end_quote = csharp_format.find('\'', i);
                if (end_quote == std::string::npos) {
                    throw std::runtime_error("Unclosed quote in format string");
                }
                result += csharp_format.substr(i, end_quote - i);
                i = end_quote + 1;
                continue;
            }
            
            if (c == 'y' || c == 'M' || c == 'd' || c == 'H' || 
                c == 'h' || c == 'm' || c == 's' || c == 't') {
                
                size_t count = 1;
                while (i + count < csharp_format.length() && csharp_format[i + count] == c) {
                    count++;
                }
                
                switch (c) {
                    case 'y':
                        if (count >= 4) {
                            result += "%Y";
                        } else {
                            result += "%y";
                        }
                        break;
                    case 'M':
                        if (count == 1) {
                            result += "%m";
                        } else {
                            result += "%m";
                        }
                        break;
                    case 'd':
                        if (count == 1) {
                            result += "%d";
                        } else if (count == 2) {
                            result += "%d";
                        } else if (count == 3) {
                            result += "%a";
                        } else if (count >= 4) {
                            result += "%A";
                        }
                        break;
                    case 'H':
                        result += "%H";
                        break;
                    case 'h':
                        result += "%I";
                        break;
                    case 'm':
                        result += "%M";
                        break;
                    case 's':
                        result += "%S";
                        break;
                    case 't':
                        if (count == 1) {
                            result += "%p"; //因为strftime的限制，所以只能使用tt格式的上下午
                        } else {
                            result += "%p";
                        }
                        break;
                }
                
                i += count;
            } else {
                result.push_back(c);
                i++;
            }
        }
        
        return result;
    }

public:
    
    DateTime() : time_point_(std::chrono::system_clock::now()) {}
    
    explicit DateTime(std::time_t t) : time_point_(std::chrono::system_clock::from_time_t(t)) {}
    
    DateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, int millisecond = 0) {
        if (month < 1 || month > 12 || day < 1 || day > 31 || 
            hour < 0 || hour > 23 || minute < 0 || minute > 59 || 
            second < 0 || second > 59 || millisecond < 0 || millisecond > 999) {
            throw std::runtime_error("Invalid datetime parameters");
        }
        
        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        tm.tm_isdst = -1;
        
        time_point_ = tm_to_time_point(tm);
        if (millisecond > 0) {
            time_point_ += std::chrono::milliseconds(millisecond);
        }
    }
    
    static DateTime Now() {
        return DateTime();
    }
    
    static DateTime UtcNow() {
        auto now = std::chrono::system_clock::now();
        return DateTime(std::chrono::system_clock::to_time_t(now));
    }
    
    static DateTime Today() {
        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
#ifdef _WIN32
        if (localtime_s(&tm, &tt) != 0) {
            throw std::runtime_error("Failed to get local time");
        }
#else
        if (localtime_r(&tt, &tm) == nullptr) {
            throw std::runtime_error("Failed to get local time");
        }
#endif
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        return DateTime(std::mktime(&tm));
    }

    //C++没有属性，Get函数凑合凑合吧
    
    int GetYear() const { return get_tm().tm_year + 1900; }
    int GetMonth() const { return get_tm().tm_mon + 1; }
    int GetDay() const { return get_tm().tm_mday; }
    int GetHour() const { return get_tm().tm_hour; }
    int GetMinute() const { return get_tm().tm_min; }
    int GetSecond() const { return get_tm().tm_sec; }
    
    int GetMillisecond() const {
        auto duration = time_point_.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration - seconds);
        return milliseconds.count();
    }

    int GetDayOfWeek() const {
        std::tm tm = get_tm();
        return tm.tm_wday;
    }
    
    int GetDayOfYear() const {
        std::tm tm = get_tm();
        return tm.tm_yday + 1;
    }

    int GetUtcYear() const { return get_tm_utc().tm_year + 1900; }
    int GetUtcMonth() const { return get_tm_utc().tm_mon + 1; }
    int GetUtcDay() const { return get_tm_utc().tm_mday; }
    int GetUtcHour() const { return get_tm_utc().tm_hour; }
    int GetUtcMinute() const { return get_tm_utc().tm_min; }
    int GetUtcSecond() const { return get_tm_utc().tm_sec; }
    
    std::time_t ToTime_t() const {
        return std::chrono::system_clock::to_time_t(time_point_);
    }
    
    // Use C#-style format strings for date/time formatting
    std::string ToString(const std::string& format = "yyyy-MM-dd HH:mm:ss") const {
        if (format.empty()) {
            return ToString("yyyy-MM-dd HH:mm:ss");
        }
        
        try {
            std::time_t tt = std::chrono::system_clock::to_time_t(time_point_);
            std::tm tm;
#ifdef _WIN32
            if (localtime_s(&tm, &tt) != 0) {
                throw std::runtime_error("Failed to convert to local time");
            }
#else
            if (localtime_r(&tt, &tm) == nullptr) {
                throw std::runtime_error("Failed to convert to local time");
            }
#endif
            
            std::string result;
            size_t i = 0;
            
            while (i < format.length()) {
                char c = format[i];
                
                if (c == '\'') {
                    // Literal text between quotes — append directly (avoids strftime UTF-8 issues on Windows)
                    i++;
                    size_t end_quote = format.find('\'', i);
                    if (end_quote == std::string::npos) {
                        throw std::runtime_error("Unclosed quote in format string");
                    }
                    result += format.substr(i, end_quote - i);
                    i = end_quote + 1;
                } else if (c == 'y' || c == 'M' || c == 'd' || c == 'H' || 
                           c == 'h' || c == 'm' || c == 's' || c == 't') {
                    // Format specifier — use strftime for a single segment
                    size_t count = 1;
                    while (i + count < format.length() && format[i + count] == c) {
                        count++;
                    }
                    
                    std::string strf;
                    switch (c) {
                        case 'y': strf = (count >= 4) ? "%Y" : "%y"; break;
                        case 'M':
                            if (count >= 4)      strf = "%B";  // full month name
                            else if (count == 3) strf = "%b";  // abbreviated month name
                            else                 strf = "%m";
                            break;
                        case 'd':
                            if (count >= 4)      strf = "%A";
                            else if (count == 3) strf = "%a";
                            else                 strf = "%d";
                            break;
                        case 'H': strf = "%H"; break;
                        case 'h': strf = "%I"; break;
                        case 'm': strf = "%M"; break;
                        case 's': strf = "%S"; break;
                        case 't': strf = "%p"; break;
                    }
                    
                    char buf[64];
                    if (std::strftime(buf, sizeof(buf), strf.c_str(), &tm) > 0) {
                        result += buf;
                    }
                    i += count;
                } else if (c == 'f') {
                    // Fractional seconds (milliseconds)
                    size_t count = 1;
                    while (i + count < format.length() && format[i + count] == 'f') {
                        count++;
                    }
                    
                    int ms = GetMillisecond();
                    char ms_buf[16] = {};
                    
                    if (count <= 3) {
                        if (count == 1) {
                            snprintf(ms_buf, sizeof(ms_buf), "%d", ms / 100);
                        } else if (count == 2) {
                            snprintf(ms_buf, sizeof(ms_buf), "%02d", ms / 10);
                        } else {
                            snprintf(ms_buf, sizeof(ms_buf), "%03d", ms);
                        }
                        result += ms_buf;
                    } else if (count <= 6) {
                        snprintf(ms_buf, sizeof(ms_buf), "%06d", ms * 1000);
                        result += std::string(ms_buf).substr(0, count);
                    } else {
                        snprintf(ms_buf, sizeof(ms_buf), "%07d", ms * 10000);
                        result += std::string(ms_buf).substr(0, count);
                    }
                    i += count;
                } else {
                    // Regular literal character
                    result += c;
                    i++;
                }
            }
            
            return result;
        } catch (...) {
            throw;
        }
    }
    
    std::string ToShortDateString() const {
        return ToString("yyyy-MM-dd");
    }
    
    std::string ToLongDateString() const {
        return ToString("dddd, MMMM d, yyyy");
    }
    
    std::string ToShortTimeString() const {
        return ToString("HH:mm");
    }
    
    std::string ToLongTimeString() const {
        return ToString("HH:mm:ss");
    }
    
    //Parse with C# style DateTime stringify format
    static DateTime Parse(const std::string& str, const std::string& format = "") {
        if (format.empty()) {
            
            std::vector<std::string> common_formats = {
                "yyyy-MM-dd HH:mm:ss",
                "yyyy-MM-dd",
                "MM/dd/yyyy HH:mm:ss",
                "MM/dd/yyyy",
                "dd/MM/yyyy HH:mm:ss",
                "dd/MM/yyyy",
                "yyyy/MM/dd HH:mm:ss",
                "yyyy/MM/dd"
            };
            
            for (const auto& fmt : common_formats) {
                try {
                    return Parse(str, fmt);
                } catch (...) {
                    // 继续尝试下一个格式
                }
            }
            
            throw std::runtime_error("Unable to parse the datetime string:" + str);
        }
        
        int year = 1900, month = 1, day = 1;
        int hour = 0, minute = 0, second = 0, millisecond = 0;
        bool is_pm = false;
        
        if (try_parse_csharp_format(str, format, year, month, day, hour, minute, second, millisecond, is_pm)) {
            return DateTime(year, month, day, hour, minute, second, millisecond);
        }
        
        throw std::runtime_error("Unable to parse the datetime \"" + str +"\" with format \"" + format + "\"");
    }
    
    //Parse with C# style DateTime stringify format
    static bool TryParse(const std::string& str, DateTime& result, const std::string& format = "") {
        try {
            result = Parse(str, format);
            return true;
        } catch (...) {
            return false;
        }
    }
    
    DateTime AddDays(int days) const {
        DateTime result = *this;
        result.time_point_ += std::chrono::hours(24 * days);
        return result;
    }
    
    DateTime AddHours(int hours) const {
        DateTime result = *this;
        result.time_point_ += std::chrono::hours(hours);
        return result;
    }
    
    DateTime AddMinutes(int minutes) const {
        DateTime result = *this;
        result.time_point_ += std::chrono::minutes(minutes);
        return result;
    }
    
    DateTime AddSeconds(int seconds) const {
        DateTime result = *this;
        result.time_point_ += std::chrono::seconds(seconds);
        return result;
    }
    
    DateTime AddMilliseconds(int milliseconds) const {
        DateTime result = *this;
        result.time_point_ += std::chrono::milliseconds(milliseconds);
        return result;
    }
    
    DateTime AddMonths(int months) {
        std::tm tm = get_tm();
        tm.tm_mon += months;
        
        while (tm.tm_mon > 11) {
            tm.tm_mon -= 12;
            tm.tm_year++;
        }
        while (tm.tm_mon < 0) {
            tm.tm_mon += 12;
            tm.tm_year--;
        }
        
        int max_day = days_in_month(tm.tm_year + 1900, tm.tm_mon + 1);
        if (tm.tm_mday > max_day) {
            tm.tm_mday = max_day;
        }
        
        DateTime result(std::mktime(&tm));
        result.time_point_ += std::chrono::milliseconds(GetMillisecond());
        return result;
    }
    
    DateTime AddYears(int years) {
        std::tm tm = get_tm();
        tm.tm_year += years;
        
        if (tm.tm_mon == 1 && tm.tm_mday == 29) {
            if (!is_leap_year(tm.tm_year + 1900)) {
                tm.tm_mday = 28;
            }
        }
        
        DateTime result(std::mktime(&tm));
        result.time_point_ += std::chrono::milliseconds(GetMillisecond());
        return result;
    }

    private:

    static bool is_leap_year(int year) {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }
    
    static int days_in_month(int year, int month) {
        static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month == 2 && is_leap_year(year)) {
            return 29;
        }
        return days[month - 1];
    }

    public:
    
    double TotalDaysSince(const DateTime& other) const {
        auto diff = time_point_ - other.time_point_;
        auto hours = std::chrono::duration_cast<std::chrono::hours>(diff);
        return hours.count() / 24.0;
    }
    
    double TotalHoursSince(const DateTime& other) const {
        auto diff = time_point_ - other.time_point_;
        auto hours = std::chrono::duration_cast<std::chrono::hours>(diff);
        return hours.count();
    }
    
    double TotalMinutesSince(const DateTime& other) const {
        auto diff = time_point_ - other.time_point_;
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(diff);
        return minutes.count();
    }
    
    double TotalSecondsSince(const DateTime& other) const {
        auto diff = time_point_ - other.time_point_;
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(diff);
        return seconds.count();
    }
    
    bool operator==(const DateTime& other) const { return time_point_ == other.time_point_; }
    bool operator!=(const DateTime& other) const { return time_point_ != other.time_point_; }
    bool operator<(const DateTime& other) const { return time_point_ < other.time_point_; }
    bool operator<=(const DateTime& other) const { return time_point_ <= other.time_point_; }
    bool operator>(const DateTime& other) const { return time_point_ > other.time_point_; }
    bool operator>=(const DateTime& other) const { return time_point_ >= other.time_point_; }
    
    static DateTime MinValue() {
        return DateTime(0);
    }
    
    static DateTime MaxValue() {
        return DateTime(253402300799);
    }
    
    friend std::ostream& operator<<(std::ostream& os, const DateTime& dt) {
        os << dt.ToString();
        return os;
    }
    
    friend std::istream& operator>>(std::istream& is, DateTime& dt) {
        std::string str;
        if (!(is >> str)) {
            return is;
        }
        DateTime result;
        if (DateTime::TryParse(str, result)) {
            dt = result;
        } else {
            is.setstate(std::ios::failbit);
        }
        return is;
    }
};

#endif
