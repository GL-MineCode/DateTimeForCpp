#include <cstdio>
#include <iostream>
#include "GL_DateTime.hpp"

void test_constructors() {
    printf("===== Constructor Test =====\n");
    
    DateTime dt1;
    printf("Now:            %s\n", dt1.ToString().c_str());
    
    DateTime dt2(2024, 6, 7, 12, 30, 0, 456);
    printf("Custom:         %s (ms=%d)\n", dt2.ToString("yyyy-MM-dd HH:mm:ss.fff").c_str(), dt2.GetMillisecond());
    
    DateTime dt3 = DateTime::Today();
    printf("Today:          %s\n", dt3.ToString().c_str());
    
    DateTime dt4 = DateTime::UtcNow();
    printf("UtcNow:         %s\n", dt4.ToString().c_str());
}

void test_properties() {
    printf("\n===== Property Test =====\n");
    DateTime dt(2024, 12, 25, 10, 30, 45, 123);
    
    printf("Year:           %d\n", dt.GetYear());
    printf("Month:          %d\n", dt.GetMonth());
    printf("Day:            %d\n", dt.GetDay());
    printf("Hour:           %d\n", dt.GetHour());
    printf("Minute:         %d\n", dt.GetMinute());
    printf("Second:         %d\n", dt.GetSecond());
    printf("Millisecond:    %d\n", dt.GetMillisecond());
    printf("DayOfWeek:      %d (0=Sun)\n", dt.GetDayOfWeek());
    printf("DayOfYear:      %d\n", dt.GetDayOfYear());
}

void test_to_string() {
    printf("\n===== ToString Format Test =====\n");
    DateTime dt(2024, 6, 7, 14, 5, 3, 789);
    
    printf("Default:        %s\n", dt.ToString().c_str());
    printf("yyyy-MM-dd:     %s\n", dt.ToString("yyyy-MM-dd").c_str());
    printf("MM/dd/yyyy:     %s\n", dt.ToString("MM/dd/yyyy").c_str());
    printf("HH:mm:ss:       %s\n", dt.ToString("HH:mm:ss").c_str());
    printf("hh:mm tt:       %s\n", dt.ToString("hh:mm tt").c_str());
    printf("With ms:        %s\n", dt.ToString("yyyy-MM-dd HH:mm:ss.fff").c_str());
    printf("Custom text:    %s\n", dt.ToString("yyyy'年'MM'月'dd'日'").c_str());
    printf("ShortDate:      %s\n", dt.ToShortDateString().c_str());
    printf("LongDate:       %s\n", dt.ToLongDateString().c_str());
    printf("ShortTime:      %s\n", dt.ToShortTimeString().c_str());
    printf("LongTime:       %s\n", dt.ToLongTimeString().c_str());
}

void test_parse() {
    printf("\n===== Parse Test =====\n");
    
    auto test_parse = [](const std::string& str) {
        try {
            DateTime dt = DateTime::Parse(str);
            printf("Parse(\"%s\") -> %s\n", str.c_str(), dt.ToString().c_str());
        } catch (const std::exception& e) {
            printf("Parse(\"%s\") failed: %s\n", str.c_str(), e.what());
        }
    };
    
    test_parse("2024-06-07 14:30:00");
    test_parse("2024-06-07");
    test_parse("06/07/2024 14:30:00");
    test_parse("2024/06/07");
    
    // Parse with specified format
    try {
        DateTime dt = DateTime::Parse("2024年06月07日", "yyyy'年'MM'月'dd'日'");
        printf("Parse custom:   %s\n", dt.ToString().c_str());
    } catch (const std::exception& e) {
        printf("Parse custom failed: %s\n", e.what());
    }
}

void test_arithmetic() {
    printf("\n===== Arithmetic Test =====\n");
    DateTime dt(2024, 6, 7, 12, 0, 0);
    printf("Original:       %s\n", dt.ToString().c_str());
    printf("+10 days:       %s\n", dt.AddDays(10).ToString().c_str());
    printf("-5 days:        %s\n", dt.AddDays(-5).ToString().c_str());
    printf("+2 hours:       %s\n", dt.AddHours(2).ToString().c_str());
    printf("+30 minutes:    %s\n", dt.AddMinutes(30).ToString().c_str());
    printf("+45 seconds:    %s\n", dt.AddSeconds(45).ToString().c_str());
    printf("+1 month:       %s\n", dt.AddMonths(1).ToString().c_str());
    printf("+1 year:        %s\n", dt.AddYears(1).ToString().c_str());
    
    // Month/year boundary test
    DateTime dt2(2024, 1, 31, 0, 0, 0);
    printf("Jan 31 +1 mon:  %s\n", dt2.AddMonths(1).ToString().c_str()); // expected Feb 28/29
    printf("Jan 31 +12 mon: %s\n", dt2.AddMonths(12).ToString().c_str());
}

void test_comparison() {
    printf("\n===== Comparison Test =====\n");
    DateTime dt1(2024, 6, 7, 12, 0, 0);
    DateTime dt2(2024, 6, 8, 12, 0, 0);
    DateTime dt3(2024, 6, 7, 12, 0, 0);
    
    printf("dt1: %s\n", dt1.ToString().c_str());
    printf("dt2: %s\n", dt2.ToString().c_str());
    printf("dt1 == dt2: %s\n", (dt1 == dt2) ? "true" : "false");
    printf("dt1 == dt3: %s\n", (dt1 == dt3) ? "true" : "false");
    printf("dt1 <  dt2: %s\n", (dt1 < dt2) ? "true" : "false");
    printf("dt1 >  dt2: %s\n", (dt1 > dt2) ? "true" : "false");
    
    printf("dt2 - dt1 = %.0f day(s)\n", dt2.TotalDaysSince(dt1));
    printf("dt2 - dt1 = %.0f hour(s)\n", dt2.TotalHoursSince(dt1));
    printf("dt2 - dt1 = %.0f minute(s)\n", dt2.TotalMinutesSince(dt1));
}

void test_stream() {
    printf("\n===== Stream Operator Test (operator<<) =====\n");
    DateTime dt(2024, 12, 25, 8, 30, 0);
    std::cout << "cout output:    " << dt << std::endl;
    std::cout << "Inline:         Now is " << DateTime::Now() << std::endl;
    
    printf("\n===== Stream Operator Test (operator>>) =====\n");
    printf("Enter a datetime (e.g. 2024-06-07 14:30:00):\n");
    
    // std::string input_line;
    // std::getline(std::cin, input_line);
    std::string input_line = "2026-06-06 00:00:00";
    
    // Simulate >> with stringstream
    std::istringstream iss(input_line);
    DateTime parsed_dt;
    if (iss >> parsed_dt) {
        std::cout << "Parsed:         " << parsed_dt << std::endl;
    } else {
        printf("Parse failed, please check format\n");
    }
}

void test_edge_cases() {
    printf("\n===== Edge Case Test =====\n");
    
    // Leap year
    DateTime leap(2024, 2, 29, 12, 0, 0); // 2024 is a leap year
    printf("Leap Feb 29:    %s\n", leap.ToString().c_str());
    printf("Leap +1 year:   %s\n", leap.AddYears(1).ToString().c_str()); // should be Feb 28
    
    // Min/Max value
    printf("MinValue:       %s\n", DateTime::MinValue().ToString().c_str());
    try {
        printf("MaxValue:       %s\n", DateTime::MaxValue().ToString().c_str());
    } catch (const std::exception& e) {
        printf("MaxValue:       (throws: %s)\n", e.what());
    }
    
    // 12-hour clock parsing
    try {
        DateTime dt_ampm = DateTime::Parse("06/07/2024 02:30:00 PM", "MM/dd/yyyy hh:mm:ss tt");
        printf("PM parse:       %s\n", dt_ampm.ToString().c_str());
    } catch (const std::exception& e) {
        printf("PM parse failed: %s\n", e.what());
    }
    
    // TryParse
    DateTime try_dt;
    bool ok = DateTime::TryParse("invalid", try_dt);
    printf("TryParse bad:   %s\n", ok ? "succeeded (unexpected)" : "failed (expected)");
}

int main() {
    test_constructors();
    test_properties();
    test_to_string();
    test_parse();
    test_arithmetic();
    test_comparison();
    test_stream();
    test_edge_cases();
    
    printf("\n===== All tests completed =====\n");
    return 0;
}