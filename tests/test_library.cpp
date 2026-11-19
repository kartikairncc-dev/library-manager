/**
 * @file test_library.cpp
 * @brief Comprehensive assert-based test suite for the Object-Oriented Library Manager.
 * 
 * Tests cover:
 *  1. Empty collection handling
 *  2. Duplicate ID rejection (Books and Members, plus CSV loader)
 *  3. Missing file handling
 *  4. Malformed CSV rows (skipped with warnings, never crashes)
 *  5. Boundary date validation (leap years, month ends, invalid formats)
 *  6. Issuing an unavailable book (rejection of double borrow)
 *  7. Exceeding borrow limits (rejection when patron hits limit)
 *  8. Returning a book not issued (rejection of invalid returns)
 *  9. Save then reload roundtrip (full state persistence verification)
 */

#include "Book.h"
#include "Member.h"
#include "Loan.h"
#include "Library.h"
#include "Storage.h"
#include "DateUtils.h"

#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>

// Simple test assertion helper with informative printing
#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[-] ASSERTION FAILED: " << msg << "\n" \
                      << "    File: " << __FILE__ << ", Line: " << __LINE__ << "\n"; \
            std::exit(1); \
        } \
    } while (0)

void testEmptyCollection() {
    std::cout << "[Test 1] Testing empty collection handling...\n";
    Library lib;

    TEST_ASSERT(lib.getBooks().empty(), "New library must have empty books");
    TEST_ASSERT(lib.getMembers().empty(), "New library must have empty members");
    TEST_ASSERT(lib.getLoans().empty(), "New library must have empty loans");
    TEST_ASSERT(lib.getActiveLoans().empty(), "New library must have no active loans");

    // Search on empty collection returns empty
    auto searchRes = lib.searchBooks("Pragmatic");
    TEST_ASSERT(searchRes.empty(), "Searching empty library should return empty list");

    // Lookups on empty collection return nullptr
    TEST_ASSERT(lib.findBook("B001") == nullptr, "findBook on empty should return nullptr");
    TEST_ASSERT(lib.findMember("M001") == nullptr, "findMember on empty should return nullptr");
    TEST_ASSERT(lib.findActiveLoan("B001") == nullptr, "findActiveLoan on empty should return nullptr");

    // Availability check on empty collection returns false with error
    bool isAvail = false;
    std::string details, err;
    bool found = lib.checkAvailability("B001", isAvail, details, err);
    TEST_ASSERT(!found, "checkAvailability on nonexistent book should fail");
    TEST_ASSERT(!err.empty(), "Error message should be set for nonexistent book");

    // Issuing or returning on empty collection fails gracefully
    bool issued = lib.issueBook("B001", "M001", "2026-01-01", "2026-01-15", err);
    TEST_ASSERT(!issued, "Cannot issue book from empty library");

    bool returned = lib.returnBook("B001", "2026-01-10", err);
    TEST_ASSERT(!returned, "Cannot return book in empty library");

    std::cout << "  -> Passed: Empty collection handled safely without crashes.\n";
}

void testDuplicateIDs() {
    std::cout << "[Test 2] Testing duplicate ID rejection...\n";
    Library lib;
    std::string err;

    Book b1("B001", "The Pragmatic Programmer", "Hunt & Thomas", "Software", 1999);
    Book b1Dup("B001", "Duplicate Book", "Another Author", "Design", 2020);

    TEST_ASSERT(lib.addBook(b1, err), "Adding first book B001 should succeed");
    TEST_ASSERT(!lib.addBook(b1Dup, err), "Adding duplicate book B001 should fail");
    TEST_ASSERT(err.find("already exists") != std::string::npos, "Error should report duplicate ID");
    TEST_ASSERT(lib.getBooks().size() == 1, "Only one book should exist in library");
    TEST_ASSERT(lib.getBooks()[0].getTitle() == "The Pragmatic Programmer", "Original book preserved");

    Member m1("M001", "Alice Smith", 3);
    Member m1Dup("M001", "Duplicate Alice", 5);

    TEST_ASSERT(lib.addMember(m1, err), "Adding first member M001 should succeed");
    TEST_ASSERT(!lib.addMember(m1Dup, err), "Adding duplicate member M001 should fail");
    TEST_ASSERT(err.find("already exists") != std::string::npos, "Error should report duplicate member ID");
    TEST_ASSERT(lib.getMembers().size() == 1, "Only one member should exist");
    TEST_ASSERT(lib.getMembers()[0].getName() == "Alice Smith", "Original member preserved");

    std::cout << "  -> Passed: Duplicate IDs rejected; original records preserved.\n";
}

void testMissingFile() {
    std::cout << "[Test 3] Testing missing file handling...\n";
    Storage storage("non_existent_books_99999.csv",
                    "non_existent_members_99999.csv",
                    "non_existent_loans_99999.csv",
                    "non_existent_seed_99999.csv");

    Library lib;
    std::vector<std::string> warnings;
    std::string errorMsg;

    // Loading from nonexistent files should not crash; it starts fresh
    bool result = storage.load(lib, warnings, errorMsg);
    TEST_ASSERT(result, "Loading missing files should succeed by starting fresh");
    TEST_ASSERT(lib.getBooks().empty(), "Library should be empty when files missing");
    TEST_ASSERT(!warnings.empty(), "Warnings should note that data/seed files were missing");

    // Direct book CSV load on missing file returns false gracefully
    std::vector<Book> books;
    bool loadDirect = storage.loadBooksCSV("missing_file_xyz.csv", books, warnings, errorMsg);
    TEST_ASSERT(!loadDirect, "Direct load on missing file should return false");
    TEST_ASSERT(!errorMsg.empty(), "Error message should be populated");

    std::cout << "  -> Passed: Missing files handled gracefully.\n";
}

void testMalformedRows() {
    std::cout << "[Test 4] Testing malformed CSV rows handling...\n";
    std::string testCsvPath = "data/test_malformed.csv";

    // Create a CSV with various malformed rows:
    // 1. Valid header
    // 2. Valid book 1
    // 3. Row with only 3 columns (too few)
    // 4. Row with invalid year (non-numeric string)
    // 5. Row with negative year
    // 6. Row with empty book ID
    // 7. Row with duplicate book ID
    // 8. Row with unrecognized status
    // 9. Valid book 2
    {
        std::ofstream out(testCsvPath);
        out << "book_id,title,author,category,year,status,borrower_id,due_date\n"
            << "B101,Valid Book One,Author A,Fiction,2020,available,,\n"
            << "B102,Too Few Columns,Author B\n"
            << "B103,Bad Year Book,Author C,Science,NOT_A_YEAR,available,,\n"
            << "B104,Negative Year,Author D,History,-500,available,,\n"
            << ",Empty ID Book,Author E,Tech,2021,available,,\n"
            << "B101,Duplicate ID Book,Author F,Tech,2022,available,,\n"
            << "B105,Invalid Status,Author G,Tech,2022,lost,,\n"
            << "B106,Valid Book Two,Author H,Non-Fiction,2023,available,,\n";
    }

    Storage storage;
    std::vector<Book> loadedBooks;
    std::vector<std::string> warnings;
    std::string err;

    bool ok = storage.loadBooksCSV(testCsvPath, loadedBooks, warnings, err);
    TEST_ASSERT(ok, "loadBooksCSV should return true despite malformed rows");
    TEST_ASSERT(loadedBooks.size() == 2, "Only the 2 valid books should be loaded");
    TEST_ASSERT(loadedBooks[0].getId() == "B101", "First loaded book must be B101");
    TEST_ASSERT(loadedBooks[1].getId() == "B106", "Second loaded book must be B106");
    TEST_ASSERT(warnings.size() >= 5, "Warnings must be recorded for each malformed/duplicate row");

    // Clean up temporary test file
    std::remove(testCsvPath.c_str());

    std::cout << "  -> Passed: Malformed rows safely skipped with descriptive warnings.\n";
}

void testBoundaryDates() {
    std::cout << "[Test 5] Testing boundary date validation (leap years, month ends)...\n";

    // Leap Year checks
    TEST_ASSERT(DateUtils::isLeapYear(2024), "2024 is divisible by 4 -> leap year");
    TEST_ASSERT(DateUtils::isLeapYear(2000), "2000 is divisible by 400 -> leap year");
    TEST_ASSERT(!DateUtils::isLeapYear(1900), "1900 is century not div by 400 -> not leap");
    TEST_ASSERT(!DateUtils::isLeapYear(2023), "2023 is not leap");

    // February boundary dates
    TEST_ASSERT(DateUtils::isValidDate("2024-02-29"), "2024-02-29 is valid leap day");
    TEST_ASSERT(DateUtils::isValidDate("2000-02-29"), "2000-02-29 is valid leap day");
    TEST_ASSERT(!DateUtils::isValidDate("2023-02-29"), "2023-02-29 is invalid (not leap)");
    TEST_ASSERT(!DateUtils::isValidDate("1900-02-29"), "1900-02-29 is invalid (1900 not leap)");
    TEST_ASSERT(!DateUtils::isValidDate("2024-02-30"), "2024-02-30 is impossible");

    // Month ends (30 vs 31 days)
    TEST_ASSERT(DateUtils::isValidDate("2026-04-30"), "April 30 is valid");
    TEST_ASSERT(!DateUtils::isValidDate("2026-04-31"), "April 31 is invalid (April has 30)");
    TEST_ASSERT(DateUtils::isValidDate("2026-01-31"), "January 31 is valid");
    TEST_ASSERT(DateUtils::isValidDate("2026-03-31"), "March 31 is valid");
    TEST_ASSERT(!DateUtils::isValidDate("2026-06-31"), "June 31 is invalid (June has 30)");
    TEST_ASSERT(DateUtils::isValidDate("2026-12-31"), "December 31 is valid");
    TEST_ASSERT(!DateUtils::isValidDate("2026-12-32"), "December 32 is invalid");

    // Boundary months (0, 13)
    TEST_ASSERT(!DateUtils::isValidDate("2026-00-15"), "Month 00 is invalid");
    TEST_ASSERT(!DateUtils::isValidDate("2026-13-01"), "Month 13 is invalid");

    // Format & malformed checks
    TEST_ASSERT(!DateUtils::isValidDate(""), "Empty date is invalid");
    TEST_ASSERT(!DateUtils::isValidDate("2026-1-1"), "Short date is invalid");
    TEST_ASSERT(!DateUtils::isValidDate("2026/05/10"), "Slash separators are invalid");
    TEST_ASSERT(!DateUtils::isValidDate("abcd-ef-gh"), "Letters are invalid");
    TEST_ASSERT(!DateUtils::isValidDate("2026-05-1a"), "Non-digit character is invalid");

    // Chronological order helper
    TEST_ASSERT(DateUtils::isDateLessOrEqual("2026-01-01", "2026-01-02"), "Jan 1 <= Jan 2");
    TEST_ASSERT(DateUtils::isDateLessOrEqual("2026-01-01", "2026-01-01"), "Jan 1 <= Jan 1");
    TEST_ASSERT(!DateUtils::isDateLessOrEqual("2026-02-01", "2026-01-31"), "Feb 1 not <= Jan 31");

    std::cout << "  -> Passed: Boundary dates and calendar constraints thoroughly verified.\n";
}

void testIssuingUnavailableBook() {
    std::cout << "[Test 6] Testing issuing an unavailable book...\n";
    Library lib;
    std::string err;

    Book b("B001", "The Pragmatic Programmer", "Hunt & Thomas", "Software", 1999);
    Member m1("M001", "Alice", 3);
    Member m2("M002", "Bob", 3);

    lib.addBook(b, err);
    lib.addMember(m1, err);
    lib.addMember(m2, err);

    // Issue to Alice succeeds
    bool ok1 = lib.issueBook("B001", "M001", "2026-05-01", "2026-05-15", err);
    TEST_ASSERT(ok1, "First issue of available book must succeed");
    TEST_ASSERT(!lib.findBook("B001")->isAvailable(), "Book status must now be borrowed");
    TEST_ASSERT(lib.findBook("B001")->getBorrowerId() == "M001", "Borrower must be Alice");

    // Attempt to issue already borrowed book to Bob must fail
    bool ok2 = lib.issueBook("B001", "M002", "2026-05-02", "2026-05-16", err);
    TEST_ASSERT(!ok2, "Issuing an unavailable book must fail");
    TEST_ASSERT(err.find("already borrowed") != std::string::npos, "Error must indicate book is already borrowed");
    TEST_ASSERT(m2.getBorrowedCount() == 0, "Bob's borrow count should remain 0");

    std::cout << "  -> Passed: Issuing unavailable book rejected with proper error message.\n";
}

void testExceedingBorrowLimit() {
    std::cout << "[Test 7] Testing exceeding the borrow limit...\n";
    Library lib;
    std::string err;

    // Member with max limit of 3
    Member m("M001", "Charlie", 3);
    lib.addMember(m, err);

    Book b1("B001", "Book One", "Author 1", "Cat", 2020);
    Book b2("B002", "Book Two", "Author 2", "Cat", 2021);
    Book b3("B003", "Book Three", "Author 3", "Cat", 2022);
    Book b4("B004", "Book Four", "Author 4", "Cat", 2023);

    lib.addBook(b1, err);
    lib.addBook(b2, err);
    lib.addBook(b3, err);
    lib.addBook(b4, err);

    TEST_ASSERT(lib.issueBook("B001", "M001", "2026-06-01", "2026-06-15", err), "Borrow 1 should succeed");
    TEST_ASSERT(lib.issueBook("B002", "M001", "2026-06-01", "2026-06-15", err), "Borrow 2 should succeed");
    TEST_ASSERT(lib.issueBook("B003", "M001", "2026-06-01", "2026-06-15", err), "Borrow 3 should succeed");

    const Member* cm = lib.findMember("M001");
    TEST_ASSERT(cm->getBorrowedCount() == 3, "Member should have 3 books borrowed");
    TEST_ASSERT(!cm->canBorrow(), "Member canBorrow should return false at limit");

    // Attempting to borrow 4th book must fail
    bool ok4 = lib.issueBook("B004", "M001", "2026-06-01", "2026-06-15", err);
    TEST_ASSERT(!ok4, "Borrowing beyond limit must fail");
    TEST_ASSERT(err.find("maximum borrow limit") != std::string::npos, "Error must cite borrow limit");

    TEST_ASSERT(lib.findBook("B004")->isAvailable(), "4th book must remain available");
    TEST_ASSERT(cm->getBorrowedCount() == 3, "Member borrow count must remain 3");

    std::cout << "  -> Passed: Max borrow limit strictly enforced.\n";
}

void testReturningBookNotIssued() {
    std::cout << "[Test 8] Testing returning a book that was not issued...\n";
    Library lib;
    std::string err;

    Book b("B001", "Available Book", "Author", "Cat", 2020);
    lib.addBook(b, err);

    // Book is currently available; attempting to return it must fail
    bool retAvailable = lib.returnBook("B001", "2026-07-01", err);
    TEST_ASSERT(!retAvailable, "Returning an available book must fail");
    TEST_ASSERT(err.find("No active loan found") != std::string::npos, "Error must indicate no active loan");

    // Returning a completely nonexistent book must fail
    bool retNonexistent = lib.returnBook("B999", "2026-07-01", err);
    TEST_ASSERT(!retNonexistent, "Returning nonexistent book must fail");
    TEST_ASSERT(err.find("does not exist") != std::string::npos, "Error must indicate book does not exist");

    // Now issue and return properly, then try returning again
    Member m("M001", "Dana", 3);
    lib.addMember(m, err);
    lib.issueBook("B001", "M001", "2026-07-01", "2026-07-15", err);

    bool firstReturn = lib.returnBook("B001", "2026-07-10", err);
    TEST_ASSERT(firstReturn, "First return of issued book must succeed");
    TEST_ASSERT(lib.findBook("B001")->isAvailable(), "Book must now be available");

    // Second return on now-closed loan must fail
    bool secondReturn = lib.returnBook("B001", "2026-07-11", err);
    TEST_ASSERT(!secondReturn, "Returning already returned book must fail");
    TEST_ASSERT(err.find("No active loan found") != std::string::npos, "Error must indicate no active loan");

    std::cout << "  -> Passed: Returning unissued or already-returned book rejected safely.\n";
}

void testSaveReloadRoundtrip() {
    std::cout << "[Test 9] Testing save then reload roundtrip...\n";

    std::string testBooks = "data/roundtrip_books.csv";
    std::string testMembers = "data/roundtrip_members.csv";
    std::string testLoans = "data/roundtrip_loans.csv";
    std::string testSeed = "data/non_existent_seed.csv";

    // Clean up any stale files from previous runs
    std::remove(testBooks.c_str());
    std::remove(testMembers.c_str());
    std::remove(testLoans.c_str());

    // 1. Setup original library with known state
    Library libOriginal;
    std::string err;

    Book b1("B001", "Design Patterns", "Gang of Four", "Software", 1994);
    Book b2("B002", "Refactoring", "Martin Fowler", "Software", 1999);
    Book b3("B003", "Clean Architecture", "Robert C Martin", "Software", 2017);

    libOriginal.addBook(b1, err);
    libOriginal.addBook(b2, err);
    libOriginal.addBook(b3, err);

    Member m1("M001", "Alice", 3);
    Member m2("M002", "Bob", 3);

    libOriginal.addMember(m1, err);
    libOriginal.addMember(m2, err);

    // Issue B002 to Bob
    libOriginal.issueBook("B002", "M002", "2026-08-01", "2026-08-15", err);

    // 2. Save state via Storage
    Storage storage(testBooks, testMembers, testLoans, testSeed);
    TEST_ASSERT(storage.save(libOriginal, err), "Saving original library must succeed");

    // Verify temp files were cleanly renamed to target files
    TEST_ASSERT(Storage::fileExists(testBooks), "testBooks file must exist on disk");
    TEST_ASSERT(Storage::fileExists(testMembers), "testMembers file must exist on disk");
    TEST_ASSERT(Storage::fileExists(testLoans), "testLoans file must exist on disk");
    TEST_ASSERT(!Storage::fileExists(testBooks + ".tmp"), "Temp book file must not linger");

    // 3. Reload into fresh second Library instance
    Library libReloaded;
    std::vector<std::string> warnings;
    TEST_ASSERT(storage.load(libReloaded, warnings, err), "Loading persisted files must succeed");

    // 4. Verify roundtrip equality
    // Books
    TEST_ASSERT(libReloaded.getBooks().size() == 3, "Reloaded library must have 3 books");
    const Book* rb1 = libReloaded.findBook("B001");
    const Book* rb2 = libReloaded.findBook("B002");
    const Book* rb3 = libReloaded.findBook("B003");

    TEST_ASSERT(rb1 != nullptr && rb1->isAvailable(), "B001 must be available");
    TEST_ASSERT(rb1->getTitle() == "Design Patterns", "B001 title must match");
    TEST_ASSERT(rb1->getYear() == 1994, "B001 year must match");

    TEST_ASSERT(rb2 != nullptr && !rb2->isAvailable(), "B002 must be borrowed");
    TEST_ASSERT(rb2->getBorrowerId() == "M002", "B002 borrower must be M002");
    TEST_ASSERT(rb2->getDueDate() == "2026-08-15", "B002 due date must match");

    TEST_ASSERT(rb3 != nullptr && rb3->isAvailable(), "B003 must be available");

    // Members
    TEST_ASSERT(libReloaded.getMembers().size() == 2, "Reloaded library must have 2 members");
    const Member* rm1 = libReloaded.findMember("M001");
    const Member* rm2 = libReloaded.findMember("M002");

    TEST_ASSERT(rm1 != nullptr && rm1->getBorrowedCount() == 0, "Alice must have 0 borrowed books");
    TEST_ASSERT(rm2 != nullptr && rm2->getBorrowedCount() == 1, "Bob must have 1 borrowed book");
    TEST_ASSERT(rm2->hasBorrowed("B002"), "Bob must have borrowed B002");

    // Loans
    TEST_ASSERT(libReloaded.getActiveLoans().size() == 1, "Reloaded library must have 1 active loan");
    const Loan* rloan = libReloaded.findActiveLoan("B002");
    TEST_ASSERT(rloan != nullptr, "Active loan for B002 must exist");
    TEST_ASSERT(rloan->getMemberId() == "M002", "Loan member must be M002");
    TEST_ASSERT(rloan->getDueDate() == "2026-08-15", "Loan due date must match");

    // Clean up temporary test files
    std::remove(testBooks.c_str());
    std::remove(testMembers.c_str());
    std::remove(testLoans.c_str());

    std::cout << "  -> Passed: Save and reload roundtrip preserved 100% of state.\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "  RUNNING OBJECT-ORIENTED LIBRARY MANAGER TEST SUITE\n";
    std::cout << "====================================================\n\n";

    testEmptyCollection();
    testDuplicateIDs();
    testMissingFile();
    testMalformedRows();
    testBoundaryDates();
    testIssuingUnavailableBook();
    testExceedingBorrowLimit();
    testReturningBookNotIssued();
    testSaveReloadRoundtrip();

    std::cout << "\n====================================================\n";
    std::cout << "  ALL 9 TEST SUITES PASSED SUCCESSFULLY! (100% OK)\n";
    std::cout << "====================================================\n";

    return 0;
}
