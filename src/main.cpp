/**
 * @file main.cpp
 * @brief Terminal user interface for Object-Oriented Library Manager in C++17.
 * 
 * Strict separation of concerns:
 * - main.cpp handles only terminal I/O, user prompts, and visual formatting.
 * - Business rules and domain logic reside exclusively in Library, Book, Member, and Loan.
 * - Persistence operations are delegated strictly to Storage.
 * - All user input is read using std::getline with robust parsing and graceful EOF handling.
 */

#include "Library.h"
#include "Storage.h"
#include "DateUtils.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

namespace UI {

    void printHeader(const std::string& title) {
        std::cout << "\n========================================================================================\n";
        std::cout << "  " << title << "\n";
        std::cout << "========================================================================================\n";
    }

    void printMenu() {
        std::cout << "\n+--------------------------------------------------------------------------------------+\n";
        std::cout << "|                         OBJECT-ORIENTED LIBRARY MANAGER                              |\n";
        std::cout << "+--------------------------------------------------------------------------------------+\n";
        std::cout << "  [1] List All Books\n";
        std::cout << "  [2] Search Books (Title / Author / Category)\n";
        std::cout << "  [3] Add New Book\n";
        std::cout << "  [4] List All Members\n";
        std::cout << "  [5] Add New Member\n";
        std::cout << "  [6] Issue a Book\n";
        std::cout << "  [7] Return a Book\n";
        std::cout << "  [8] Check Book Availability\n";
        std::cout << "  [9] List Active Loans\n";
        std::cout << "  [0] Exit Application\n";
        std::cout << "+--------------------------------------------------------------------------------------+\n";
        std::cout << "Enter your selection (0-9): ";
    }

    bool getLineInput(std::string& output) {
        if (!std::getline(std::cin, output)) {
            // EOF encountered (Ctrl+D / Ctrl+Z / end of piped script)
            return false;
        }
        output = DateUtils::trim(output);
        return true;
    }

    std::string promptNonEmpty(const std::string& promptText) {
        std::string input;
        while (true) {
            std::cout << promptText;
            if (!getLineInput(input)) {
                return "";
            }
            if (!input.empty()) {
                return input;
            }
            std::cout << "  [!] Input cannot be blank. Please try again.\n";
        }
    }

    void displayBooksTable(const std::vector<Book>& books) {
        if (books.empty()) {
            std::cout << "  (No books in library)\n";
            return;
        }

        std::cout << std::left
                  << std::setw(8)  << "ID"
                  << std::setw(32) << "Title"
                  << std::setw(26) << "Author"
                  << std::setw(16) << "Category"
                  << std::setw(6)  << "Year"
                  << std::setw(12) << "Status"
                  << std::setw(10) << "Borrower"
                  << std::setw(12) << "Due Date" << "\n";
        std::cout << std::string(122, '-') << "\n";

        for (const auto& b : books) {
            std::string title = b.getTitle();
            if (title.length() > 30) title = title.substr(0, 27) + "...";

            std::string author = b.getAuthor();
            if (author.length() > 24) author = author.substr(0, 21) + "...";

            std::string cat = b.getCategory();
            if (cat.length() > 14) cat = cat.substr(0, 12) + "...";

            std::cout << std::left
                      << std::setw(8)  << b.getId()
                      << std::setw(32) << title
                      << std::setw(26) << author
                      << std::setw(16) << cat
                      << std::setw(6)  << b.getYear()
                      << std::setw(12) << (b.isAvailable() ? "Available" : "Borrowed")
                      << std::setw(10) << (b.getBorrowerId().empty() ? "-" : b.getBorrowerId())
                      << std::setw(12) << (b.getDueDate().empty() ? "-" : b.getDueDate())
                      << "\n";
        }
        std::cout << std::string(122, '-') << "\n";
        std::cout << "Total Books: " << books.size() << "\n";
    }

    void displayMembersTable(const std::vector<Member>& members) {
        if (members.empty()) {
            std::cout << "  (No registered members)\n";
            return;
        }

        std::cout << std::left
                  << std::setw(10) << "Member ID"
                  << std::setw(28) << "Name"
                  << std::setw(12) << "Borrowed"
                  << std::setw(10) << "Max Limit"
                  << "Borrowed Book IDs\n";
        std::cout << std::string(80, '-') << "\n";

        for (const auto& m : members) {
            std::string booksList;
            for (size_t i = 0; i < m.getBorrowedBookIds().size(); ++i) {
                if (i > 0) booksList += ", ";
                booksList += m.getBorrowedBookIds()[i];
            }
            if (booksList.empty()) booksList = "(none)";

            std::cout << std::left
                      << std::setw(10) << m.getId()
                      << std::setw(28) << m.getName()
                      << std::setw(12) << (std::to_string(m.getBorrowedCount()) + " books")
                      << std::setw(10) << m.getMaxBorrowLimit()
                      << booksList << "\n";
        }
        std::cout << std::string(80, '-') << "\n";
        std::cout << "Total Members: " << members.size() << "\n";
    }

    void displayLoansTable(const std::vector<Loan>& loans, const std::string& today) {
        if (loans.empty()) {
            std::cout << "  (No active loans)\n";
            return;
        }

        std::cout << std::left
                  << std::setw(10) << "Book ID"
                  << std::setw(12) << "Member ID"
                  << std::setw(14) << "Issue Date"
                  << std::setw(14) << "Due Date"
                  << "Status\n";
        std::cout << std::string(65, '-') << "\n";

        for (const auto& l : loans) {
            std::string status = "Active (On Time)";
            if (l.isOverdue(today)) {
                status = "[!] OVERDUE";
            }

            std::cout << std::left
                      << std::setw(10) << l.getBookId()
                      << std::setw(12) << l.getMemberId()
                      << std::setw(14) << l.getIssueDate()
                      << std::setw(14) << l.getDueDate()
                      << status << "\n";
        }
        std::cout << std::string(65, '-') << "\n";
        std::cout << "Total Active Loans: " << loans.size() << "\n";
    }

} // namespace UI

int main() {
    Library library;
    Storage storage;

    // Load persisted state or seed data on startup
    std::vector<std::string> warnings;
    std::string loadError;
    if (!storage.load(library, warnings, loadError)) {
        std::cerr << "CRITICAL ERROR: Failed to load library data: " << loadError << "\n";
        return 1;
    }

    // Display any non-fatal startup warnings
    if (!warnings.empty()) {
        std::cout << "[Startup Notices]\n";
        for (const auto& w : warnings) {
            std::cout << "  * " << w << "\n";
        }
    }

    bool running = true;
    while (running) {
        UI::printMenu();
        std::string choice;
        if (!UI::getLineInput(choice)) {
            std::cout << "\n[EOF detected] Exiting Library Manager safely.\n";
            break;
        }

        if (choice == "1") {
            UI::printHeader("BOOKS CATALOG");
            UI::displayBooksTable(library.getBooks());
        }
        else if (choice == "2") {
            UI::printHeader("SEARCH BOOKS");
            std::cout << "Enter title, author, or category substring: ";
            std::string query;
            if (!UI::getLineInput(query)) break;

            std::vector<Book> results = library.searchBooks(query);
            std::cout << "\nSearch results for \"" << query << "\":\n";
            UI::displayBooksTable(results);
        }
        else if (choice == "3") {
            UI::printHeader("ADD NEW BOOK");
            std::string id = UI::promptNonEmpty("Enter Book ID (e.g. B007): ");
            if (id.empty()) break;

            std::string title = UI::promptNonEmpty("Enter Title: ");
            if (title.empty()) break;

            std::string author = UI::promptNonEmpty("Enter Author: ");
            if (author.empty()) break;

            std::cout << "Enter Category/Genre: ";
            std::string category;
            if (!UI::getLineInput(category)) break;

            int year = 0;
            while (true) {
                std::cout << "Enter Publication Year (e.g. 2024): ";
                std::string yearStr;
                if (!UI::getLineInput(yearStr)) break;
                try {
                    year = std::stoi(yearStr);
                    if (year > 0 && year <= 2100) break;
                    std::cout << "  [!] Year must be between 1 and 2100.\n";
                } catch (...) {
                    std::cout << "  [!] Invalid numeric format for year.\n";
                }
            }

            Book newBook(id, title, author, category, year);
            std::string errorMsg;
            if (library.addBook(newBook, errorMsg)) {
                std::cout << "  [+] Book added successfully!\n";
                std::string saveErr;
                if (storage.save(library, saveErr)) {
                    std::cout << "  [Disk] Changes persisted to storage.\n";
                } else {
                    std::cout << "  [!] Warning: Failed to save changes: " << saveErr << "\n";
                }
            } else {
                std::cout << "  [-] Error: " << errorMsg << "\n";
            }
        }
        else if (choice == "4") {
            UI::printHeader("REGISTERED MEMBERS");
            UI::displayMembersTable(library.getMembers());
        }
        else if (choice == "5") {
            UI::printHeader("ADD NEW MEMBER");
            std::string id = UI::promptNonEmpty("Enter Member ID (e.g. M001): ");
            if (id.empty()) break;

            std::string name = UI::promptNonEmpty("Enter Member Full Name: ");
            if (name.empty()) break;

            int limit = 3;
            std::cout << "Enter Max Borrow Limit (default 3, press Enter to keep default): ";
            std::string limitStr;
            if (UI::getLineInput(limitStr) && !limitStr.empty()) {
                try {
                    int val = std::stoi(limitStr);
                    if (val > 0) limit = val;
                } catch (...) {}
            }

            Member newMember(id, name, limit);
            std::string errorMsg;
            if (library.addMember(newMember, errorMsg)) {
                std::cout << "  [+] Member '" << name << "' registered successfully!\n";
                std::string saveErr;
                if (storage.save(library, saveErr)) {
                    std::cout << "  [Disk] Changes persisted to storage.\n";
                } else {
                    std::cout << "  [!] Warning: Failed to save changes: " << saveErr << "\n";
                }
            } else {
                std::cout << "  [-] Error: " << errorMsg << "\n";
            }
        }
        else if (choice == "6") {
            UI::printHeader("ISSUE BOOK LOAN");
            std::string bookId = UI::promptNonEmpty("Enter Book ID to issue: ");
            if (bookId.empty()) break;

            std::string memberId = UI::promptNonEmpty("Enter Member ID: ");
            if (memberId.empty()) break;

            std::string today = DateUtils::getTodayDate();
            std::cout << "Enter Issue Date (YYYY-MM-DD) [press Enter for today: " << today << "]: ";
            std::string issueDate;
            if (!UI::getLineInput(issueDate)) break;
            if (issueDate.empty()) {
                issueDate = today;
            }

            std::string defaultDue = DateUtils::addDaysToDate(issueDate, 14);
            std::cout << "Enter Due Date (YYYY-MM-DD) [press Enter for 14-day default: " << defaultDue << "]: ";
            std::string dueDate;
            if (!UI::getLineInput(dueDate)) break;
            if (dueDate.empty()) {
                dueDate = defaultDue;
            }

            std::string errorMsg;
            if (library.issueBook(bookId, memberId, issueDate, dueDate, errorMsg)) {
                std::cout << "  [+] Book '" << bookId << "' successfully issued to member '" << memberId << "'!\n";
                std::cout << "  Due Date: " << dueDate << "\n";
                std::string saveErr;
                if (storage.save(library, saveErr)) {
                    std::cout << "  [Disk] Changes persisted to storage.\n";
                } else {
                    std::cout << "  [!] Warning: Failed to save changes: " << saveErr << "\n";
                }
            } else {
                std::cout << "  [-] Issue Failed: " << errorMsg << "\n";
            }
        }
        else if (choice == "7") {
            UI::printHeader("RETURN BOOK");
            std::string bookId = UI::promptNonEmpty("Enter Book ID to return: ");
            if (bookId.empty()) break;

            std::string today = DateUtils::getTodayDate();
            std::cout << "Enter Return Date (YYYY-MM-DD) [press Enter for today: " << today << "]: ";
            std::string returnDate;
            if (!UI::getLineInput(returnDate)) break;
            if (returnDate.empty()) {
                returnDate = today;
            }

            std::string errorMsg;
            if (library.returnBook(bookId, returnDate, errorMsg)) {
                std::cout << "  [+] Book '" << bookId << "' successfully returned on " << returnDate << "!\n";
                std::string saveErr;
                if (storage.save(library, saveErr)) {
                    std::cout << "  [Disk] Changes persisted to storage.\n";
                } else {
                    std::cout << "  [!] Warning: Failed to save changes: " << saveErr << "\n";
                }
            } else {
                std::cout << "  [-] Return Failed: " << errorMsg << "\n";
            }
        }
        else if (choice == "8") {
            UI::printHeader("CHECK BOOK AVAILABILITY");
            std::string bookId = UI::promptNonEmpty("Enter Book ID: ");
            if (bookId.empty()) break;

            bool isAvailable = false;
            std::string statusDetails;
            std::string errorMsg;

            if (library.checkAvailability(bookId, isAvailable, statusDetails, errorMsg)) {
                const Book* b = library.findBook(bookId);
                std::cout << "\n  Book: \"" << b->getTitle() << "\" by " << b->getAuthor() << "\n";
                std::cout << "  Status: " << (isAvailable ? "[AVAILABLE]" : "[BORROWED]") << "\n";
                std::cout << "  Details: " << statusDetails << "\n";
            } else {
                std::cout << "  [-] " << errorMsg << "\n";
            }
        }
        else if (choice == "9") {
            UI::printHeader("ACTIVE LOANS");
            std::string today = DateUtils::getTodayDate();
            UI::displayLoansTable(library.getActiveLoans(), today);
        }
        else if (choice == "0") {
            std::cout << "\nThank you for using the Object-Oriented Library Manager. Goodbye!\n";
            running = false;
        }
        else {
            std::cout << "  [!] Invalid selection '" << choice << "'. Please enter a number between 0 and 9.\n";
        }
    }

    return 0;
}
