#include "DateUtils.h"

#include <cctype>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdio>

namespace DateUtils {

    bool isLeapYear(int year) {
        // A year is a leap year if it is divisible by 4,
        // except end-of-century years, which must also be divisible by 400.
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    int getDaysInMonth(int year, int month) {
        if (month < 1 || month > 12) {
            return 0;
        }
        switch (month) {
            case 1:  return 31; // January
            case 2:  return isLeapYear(year) ? 29 : 28; // February
            case 3:  return 31; // March
            case 4:  return 30; // April
            case 5:  return 31; // May
            case 6:  return 30; // June
            case 7:  return 31; // July
            case 8:  return 31; // August
            case 9:  return 30; // September
            case 10: return 31; // October
            case 11: return 30; // November
            case 12: return 31; // December
            default: return 0;
        }
    }

    bool isValidDateFormat(const std::string& dateStr) {
        if (dateStr.length() != 10) {
            return false;
        }
        if (dateStr[4] != '-' || dateStr[7] != '-') {
            return false;
        }
        for (size_t i = 0; i < 10; ++i) {
            if (i == 4 || i == 7) continue;
            if (!std::isdigit(static_cast<unsigned char>(dateStr[i]))) {
                return false;
            }
        }
        return true;
    }

    bool isValidDate(const std::string& dateStr) {
        if (!isValidDateFormat(dateStr)) {
            return false;
        }

        int year = 0;
        int month = 0;
        int day = 0;

        try {
            year = std::stoi(dateStr.substr(0, 4));
            month = std::stoi(dateStr.substr(5, 2));
            day = std::stoi(dateStr.substr(8, 2));
        } catch (...) {
            return false;
        }

        if (year < 1000 || year > 9999) {
            return false;
        }
        if (month < 1 || month > 12) {
            return false;
        }

        int maxDays = getDaysInMonth(year, month);
        if (day < 1 || day > maxDays) {
            return false;
        }

        return true;
    }

    bool isDateLessOrEqual(const std::string& d1, const std::string& d2) {
        // Since YYYY-MM-DD strings are fixed-width and formatted chronologically,
        // lexicographical comparison correctly matches chronological comparison.
        return d1 <= d2;
    }

    std::string getTodayDate() {
        std::time_t t = std::time(nullptr);
        std::tm tm{};
#if defined(_MSC_VER)
        localtime_s(&tm, &t);
#else
        std::tm* ptm = std::localtime(&t);
        if (ptm != nullptr) {
            tm = *ptm;
        }
#endif
        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d",
                      tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
        return std::string(buffer);
    }

    std::string addDaysToDate(const std::string& dateStr, int days) {
        if (!isValidDate(dateStr)) {
            return dateStr;
        }

        int year = std::stoi(dateStr.substr(0, 4));
        int month = std::stoi(dateStr.substr(5, 2));
        int day = std::stoi(dateStr.substr(8, 2));

        std::tm tm{};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day + days;
        tm.tm_isdst = -1;

        std::mktime(&tm);

        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d",
                      tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
        return std::string(buffer);
    }

    std::string trim(const std::string& str) {
        const std::string whitespace = " \t\r\n";
        const auto start = str.find_first_not_of(whitespace);
        if (start == std::string::npos) {
            return "";
        }
        const auto end = str.find_last_not_of(whitespace);
        return str.substr(start, end - start + 1);
    }

    std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return result;
    }

} // namespace DateUtils
