#ifndef DATE_UTILS_H
#define DATE_UTILS_H

#include <string>

/**
 * @namespace DateUtils
 * @brief Utility functions for ISO 8601 (YYYY-MM-DD) date validation,
 *        arithmetic, and common string formatting.
 */
namespace DateUtils {

    /**
     * @brief Checks if a given year is a leap year in the Gregorian calendar.
     * @param year Year to check (e.g., 2024).
     * @return true if leap year, false otherwise.
     */
    bool isLeapYear(int year);

    /**
     * @brief Gets the number of days in a specific month for a given year.
     * @param year The year (affects February in leap years).
     * @param month Month (1-12).
     * @return Number of days (28-31), or 0 if month is invalid.
     */
    int getDaysInMonth(int year, int month);

    /**
     * @brief Checks if string matches YYYY-MM-DD pattern (10 chars, digits and hyphens).
     * @param dateStr Date string.
     * @return true if pattern matches, false otherwise.
     */
    bool isValidDateFormat(const std::string& dateStr);

    /**
     * @brief Performs full calendar validation on a YYYY-MM-DD date string.
     *        Checks format, month boundaries (1-12), day boundaries (1-31),
     *        month lengths, and February leap year rules.
     * @param dateStr Date string to validate.
     * @return true if date is valid on the Gregorian calendar.
     */
    bool isValidDate(const std::string& dateStr);

    /**
     * @brief Compares two YYYY-MM-DD date strings.
     * @param d1 First date string.
     * @param d2 Second date string.
     * @return true if d1 <= d2 chronologically.
     */
    bool isDateLessOrEqual(const std::string& d1, const std::string& d2);

    /**
     * @brief Returns current system date formatted as YYYY-MM-DD.
     * @return Formatted date string.
     */
    std::string getTodayDate();

    /**
     * @brief Adds a specified number of days to a YYYY-MM-DD date string.
     * @param dateStr Base date string in YYYY-MM-DD format.
     * @param days Number of days to add (can be positive).
     * @return Resulting date string in YYYY-MM-DD format.
     */
    std::string addDaysToDate(const std::string& dateStr, int days);

    /**
     * @brief Trims leading and trailing whitespace from a string.
     * @param str Input string.
     * @return Trimmed string.
     */
    std::string trim(const std::string& str);

    /**
     * @brief Converts string to lowercase for case-insensitive comparisons.
     * @param str Input string.
     * @return Lowercase string.
     */
    std::string toLower(const std::string& str);

} // namespace DateUtils

#endif // DATE_UTILS_H
