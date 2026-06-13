#include <cstdio>
#include <iostream>
#include <sstream>
#include "GL_DateTime.hpp"

void test_constructors() {
    printf("===== Constructor Test =====\n");
    
    DateTime dt1 = DateTime::Now();
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
    
    TimeSpan diff = dt2 - dt1;
    printf("dt2 - dt1 = %.0f day(s)\n", diff.GetTotalDays());
    printf("dt2 - dt1 = %.0f hour(s)\n", diff.GetTotalHours());
    printf("dt2 - dt1 = %.0f minute(s)\n", diff.GetTotalMinutes());
    printf("dt2 - dt1 = %lld ticks\n", (long long)diff.GetTicks());
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

void test_new_features() {
    printf("\n===== New Feature Tests =====\n");

    DateTime dt(2024, 6, 7, 10, 30, 45, 123);

    // Ticks
    printf("Ticks:          %lld\n", (long long)dt.GetTicks());

    // Date (time of day at 00:00:00)
    DateTime dateOnly = dt.GetDate();
    printf("Date:           %s\n", dateOnly.ToString().c_str());
    printf("Date.Hour:      %d\n", dateOnly.GetHour());

    // TimeOfDay (TimeSpan)
    TimeSpan tod = dt.GetTimeOfDay();
    printf("TimeOfDay:      %s\n", tod.ToString().c_str());
    printf("TimeOfDay hrs:  %.2f\n", tod.GetTotalHours());

    // Kind
    DateTime local = DateTime::Now();
    DateTime utc = DateTime::UtcNow();
    printf("Kind Local:     %d\n", (int)local.GetKind());
    printf("Kind Utc:       %d\n", (int)utc.GetKind());

    // ToUniversalTime / ToLocalTime
    DateTime conv = local.ToUniversalTime();
    printf("Local->UTC:     %s (kind=%d)\n", conv.ToString().c_str(), (int)conv.GetKind());

    // Subtract(DateTime) -> TimeSpan
    DateTime dtA(2024, 6, 10, 12, 0, 0);
    DateTime dtB(2024, 6, 7, 12, 0, 0);
    TimeSpan span = dtA.Subtract(dtB);
    printf("Sub(dt):        %.0f days\n", span.GetTotalDays());

    // Subtract(TimeSpan) -> DateTime
    DateTime dtC = dtA.Subtract(TimeSpan(3, 0, 0, 0));
    printf("Sub(ts):        %s\n", dtC.ToString().c_str());

    // Add(TimeSpan)
    DateTime dtD = dtB.Add(TimeSpan(5, 0, 0, 0));
    printf("Add(ts):        %s\n", dtD.ToString().c_str());

    // Operator+ (TimeSpan + DateTime)
    DateTime dtE = TimeSpan(1, 0, 0, 0) + dtB;
    printf("ts+dt:          %s\n", dtE.ToString().c_str());

    // Static methods
    printf("DaysInMonth(2024,2): %d\n", DateTime::DaysInMonth(2024, 2));
    printf("IsLeapYear(2024):   %s\n", DateTime::IsLeapYear(2024) ? "true" : "false");
    printf("IsLeapYear(2023):   %s\n", DateTime::IsLeapYear(2023) ? "true" : "false");

    // Default constructor = MinValue
    DateTime def;
    printf("Default():      %s\n", def.ToString().c_str());

    // SpecifyKind
    DateTime spec = DateTime::SpecifyKind(def, DateTimeKind::Utc);
    printf("SpecifyKind:    kind=%d\n", (int)spec.GetKind());
}

void test_predefined_formats() {
    printf("\n===== Predefined Format Tests =====\n");
    DateTime dt(2024, 6, 7, 14, 30, 45, 789);

    printf("ToString(\"d\"):  %s\n", dt.ToString("d").c_str());
    printf("ToString(\"D\"):  %s\n", dt.ToString("D").c_str());
    printf("ToString(\"f\"):  %s\n", dt.ToString("f").c_str());
    printf("ToString(\"F\"):  %s\n", dt.ToString("F").c_str());
    printf("ToString(\"g\"):  %s\n", dt.ToString("g").c_str());
    printf("ToString(\"G\"):  %s\n", dt.ToString("G").c_str());
    printf("ToString(\"M\"):  %s\n", dt.ToString("M").c_str());
    printf("ToString(\"o\"):  %s\n", dt.ToString("o").c_str());
    printf("ToString(\"R\"):  %s\n", dt.ToString("R").c_str());
    printf("ToString(\"s\"):  %s\n", dt.ToString("s").c_str());
    printf("ToString(\"t\"):  %s\n", dt.ToString("t").c_str());
    printf("ToString(\"T\"):  %s\n", dt.ToString("T").c_str());
    printf("ToString(\"u\"):  %s\n", dt.ToString("u").c_str());
    printf("ToString(\"U\"):  %s\n", dt.ToString("U").c_str());
    printf("ToString(\"Y\"):  %s\n", dt.ToString("Y").c_str());

    // Parse with predefined format
    try {
        DateTime parsed = DateTime::Parse("06/07/2024", "d");
        printf("Parse(\"06/07/2024\",\"d\"): %s\n", parsed.ToString().c_str());
    } catch (const std::exception& e) {
        printf("Parse with d failed: %s\n", e.what());
    }

    try {
        DateTime parsed = DateTime::Parse("2024-06-07T14:30:45", "s");
        printf("Parse(\"2024-06-07T14:30:45\",\"s\"): %s\n", parsed.ToString().c_str());
    } catch (const std::exception& e) {
        printf("Parse with s failed: %s\n", e.what());
    }

    try {
        DateTime parsed = DateTime::Parse("Fri, 07 Jun 2024 22:30:45 GMT", "R");
        printf("Parse(\"Fri, 07 Jun 2024 22:30:45 GMT\",\"R\"): %s\n", parsed.ToString().c_str());
    } catch (const std::exception& e) {
        printf("Parse with R failed: %s\n", e.what());
    }
}

void test_roundtrip() {
    printf("\n===== Round-trip Tests (ToString → Parse) =====\n");
    DateTime dt(2024, 6, 7, 14, 30, 45, 789);
    bool all_ok = true;

    // All predefined format specifiers that produce complete date+time
    struct { const char* name; const char* fmt; } tests[] = {
        {"default",  "yyyy-MM-dd HH:mm:ss"},
        {"d", "d"}, {"D", "D"}, {"f", "f"}, {"F", "F"},
        {"g", "g"}, {"G", "G"}, {"M", "M"},
        {"o", "o"}, {"s", "s"}, {"u", "u"},
        {"Y", "Y"}, {"R", "R"},
    };

    for (auto& t : tests) {
        std::string out = dt.ToString(t.fmt);
        DateTime parsed;
        bool ok = DateTime::TryParse(out, parsed);
        if (!ok) {
            printf("FAIL [%s]: \"%s\" -> could not parse\n", t.name, out.c_str());
            all_ok = false;
        }
    }

    // Test shortcut methods
    struct { const char* name; std::string (*func)(const DateTime&); } shortcuts[] = {
        {"ToShortDateString",  [](const DateTime& d){ return d.ToShortDateString(); }},
        {"ToLongDateString",   [](const DateTime& d){ return d.ToLongDateString(); }},
        {"operator<< (default)",[](const DateTime& d){ std::ostringstream ss; ss << d; return ss.str(); }},
    };
    for (auto& s : shortcuts) {
        std::string out = s.func(dt);
        DateTime parsed;
        bool ok = DateTime::TryParse(out, parsed);
        if (!ok) {
            printf("FAIL [%s]: \"%s\" -> could not parse\n", s.name, out.c_str());
            all_ok = false;
        }
    }

    if (all_ok)
        printf("All round-trip tests passed.\n");
}

void test_timespan() {
    printf("\n===== TimeSpan Tests =====\n");

    // Constructors
    TimeSpan ts1;
    printf("Default:        %s\n", ts1.ToString().c_str());

    TimeSpan ts2(1, 2, 3, 4, 567);
    printf("(1,2,3,4,567):  %s\n", ts2.ToString().c_str());

    TimeSpan ts3(10, 30, 45);
    printf("(10,30,45):     %s\n", ts3.ToString().c_str());

    // Properties
    printf("Days:           %d\n", ts2.GetDays());
    printf("Hours:          %d\n", ts2.GetHours());
    printf("Minutes:        %d\n", ts2.GetMinutes());
    printf("Seconds:        %d\n", ts2.GetSeconds());
    printf("Milliseconds:   %d\n", ts2.GetMilliseconds());
    printf("Ticks:          %lld\n", (long long)ts2.GetTicks());
    printf("TotalDays:      %f\n", ts2.GetTotalDays());
    printf("TotalHours:     %f\n", ts2.GetTotalHours());
    printf("TotalMinutes:   %f\n", ts2.GetTotalMinutes());
    printf("TotalSeconds:   %f\n", ts2.GetTotalSeconds());
    printf("TotalMs:        %f\n", ts2.GetTotalMilliseconds());

    // Static methods
    TimeSpan fromDays = TimeSpan::FromDays(1.5);
    printf("FromDays(1.5):  %s\n", fromDays.ToString().c_str());

    TimeSpan fromHours = TimeSpan::FromHours(2.5);
    printf("FromHours(2.5): %s\n", fromHours.ToString().c_str());

    // Arithmetic
    TimeSpan tsA(2, 0, 0, 0);
    TimeSpan tsB(1, 12, 0, 0);
    printf("tsA+tsB:        %s\n", (tsA + tsB).ToString().c_str());
    printf("tsA-tsB:        %s\n", (tsA - tsB).ToString().c_str());
    printf("Negate tsA:     %s\n", (-tsA).ToString().c_str());
    printf("Duration tsA:   %s\n", tsA.Duration().ToString().c_str());

    // Comparison
    TimeSpan tsC(1, 0, 0, 0);
    TimeSpan tsD(2, 0, 0, 0);
    printf("tsC==tsD:       %s\n", (tsC == tsD) ? "true" : "false");
    printf("tsC<tsD:        %s\n", (tsC < tsD) ? "true" : "false");

    // Static Zero/MinValue/MaxValue
    printf("Zero:           %s\n", TimeSpan::Zero().ToString().c_str());
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
    test_new_features();
    test_predefined_formats();
    test_roundtrip();
    test_timespan();

    printf("\n===== All tests completed =====\n");
    return 0;
}