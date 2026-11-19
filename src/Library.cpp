#include "Library.h"
#include "DateUtils.h"
#include <algorithm>

void Library::clear() {
    books_.clear();
    members_.clear();
    loans_.clear();
}

// ==========================================
// Internal Iterator Helpers
// ==========================================

std::vector<Book>::iterator Library::findBookIter(const std::string& bookId) {
    return std::find_if(books_.begin(), books_.end(), [&](const Book& b) {
        return b.getId() == bookId;
    });
}

std::vector<Book>::const_iterator Library::findBookIter(const std::string& bookId) const {
    return std::find_if(books_.begin(), books_.end(), [&](const Book& b) {
        return b.getId() == bookId;
    });
}

std::vector<Member>::iterator Library::findMemberIter(const std::string& memberId) {
    return std::find_if(members_.begin(), members_.end(), [&](const Member& m) {
        return m.getId() == memberId;
    });
}

std::vector<Member>::const_iterator Library::findMemberIter(const std::string& memberId) const {
    return std::find_if(members_.begin(), members_.end(), [&](const Member& m) {
        return m.getId() == memberId;
    });
}

std::vector<Loan>::iterator Library::findActiveLoanIter(const std::string& bookId) {
    return std::find_if(loans_.begin(), loans_.end(), [&](const Loan& l) {
        return l.getBookId() == bookId && !l.isReturned();
    });
}

std::vector<Loan>::const_iterator Library::findActiveLoanIter(const std::string& bookId) const {
    return std::find_if(loans_.begin(), loans_.end(), [&](const Loan& l) {
        return l.getBookId() == bookId && !l.isReturned();
    });
}

// ==========================================
// Book Management
// ==========================================

bool Library::addBook(const Book& book, std::string& errorMsg) {
    if (book.getId().empty()) {
        errorMsg = "Book ID cannot be empty.";
        return false;
    }
    if (findBook(book.getId()) != nullptr) {
        errorMsg = "Book with ID '" + book.getId() + "' already exists.";
        return false;
    }
    if (book.getTitle().empty()) {
        errorMsg = "Book title cannot be empty.";
        return false;
    }
    if (book.getAuthor().empty()) {
        errorMsg = "Book author cannot be empty.";
        return false;
    }
    if (book.getYear() <= 0) {
        errorMsg = "Book publication year must be a positive number.";
        return false;
    }

    books_.push_back(book);
    return true;
}

const Book* Library::findBook(const std::string& bookId) const {
    auto it = findBookIter(bookId);
    if (it != books_.end()) {
        return &(*it);
    }
    return nullptr;
}

Book* Library::findBook(const std::string& bookId) {
    auto it = findBookIter(bookId);
    if (it != books_.end()) {
        return &(*it);
    }
    return nullptr;
}

const std::vector<Book>& Library::getBooks() const {
    return books_;
}

std::vector<Book> Library::searchBooks(const std::string& query) const {
    std::string q = DateUtils::toLower(DateUtils::trim(query));
    if (q.empty()) {
        return books_;
    }

    std::vector<Book> matches;
    for (const auto& book : books_) {
        std::string title = DateUtils::toLower(book.getTitle());
        std::string author = DateUtils::toLower(book.getAuthor());
        std::string category = DateUtils::toLower(book.getCategory());

        if (title.find(q) != std::string::npos ||
            author.find(q) != std::string::npos ||
            category.find(q) != std::string::npos) {
            matches.push_back(book);
        }
    }
    return matches;
}

bool Library::checkAvailability(const std::string& bookId,
                                bool& isAvailable,
                                std::string& statusDetails,
                                std::string& errorMsg) const {
    const Book* book = findBook(bookId);
    if (book == nullptr) {
        errorMsg = "Book with ID '" + bookId + "' was not found.";
        return false;
    }

    isAvailable = book->isAvailable();
    if (isAvailable) {
        statusDetails = "Available on shelf.";
    } else {
        const Loan* loan = findActiveLoan(bookId);
        std::string borrowerInfo = book->getBorrowerId();
        std::string dueInfo = book->getDueDate();
        
        statusDetails = "Borrowed by member " + (borrowerInfo.empty() ? "Unknown" : borrowerInfo);
        if (!dueInfo.empty()) {
            statusDetails += ", due on " + dueInfo;
            if (loan != nullptr && loan->isOverdue()) {
                statusDetails += " [OVERDUE]";
            }
        }
    }
    return true;
}

// ==========================================
// Member Management
// ==========================================

bool Library::addMember(const Member& member, std::string& errorMsg) {
    if (member.getId().empty()) {
        errorMsg = "Member ID cannot be empty.";
        return false;
    }
    if (findMember(member.getId()) != nullptr) {
        errorMsg = "Member with ID '" + member.getId() + "' already exists.";
        return false;
    }
    if (member.getName().empty()) {
        errorMsg = "Member name cannot be empty.";
        return false;
    }

    members_.push_back(member);
    return true;
}

const Member* Library::findMember(const std::string& memberId) const {
    auto it = findMemberIter(memberId);
    if (it != members_.end()) {
        return &(*it);
    }
    return nullptr;
}

Member* Library::findMember(const std::string& memberId) {
    auto it = findMemberIter(memberId);
    if (it != members_.end()) {
        return &(*it);
    }
    return nullptr;
}

const std::vector<Member>& Library::getMembers() const {
    return members_;
}

// ==========================================
// Loan & Borrowing Operations
// ==========================================

bool Library::issueBook(const std::string& bookId,
                        const std::string& memberId,
                        const std::string& issueDate,
                        const std::string& dueDate,
                        std::string& errorMsg) {
    // Rule 1: Book must exist
    Book* book = findBook(bookId);
    if (book == nullptr) {
        errorMsg = "Book with ID '" + bookId + "' does not exist.";
        return false;
    }

    // Rule 2: Book must be available
    if (!book->isAvailable()) {
        errorMsg = "Book '" + book->getTitle() + "' (ID: " + bookId + ") is already borrowed.";
        return false;
    }

    // Rule 3: Member must exist
    Member* member = findMember(memberId);
    if (member == nullptr) {
        errorMsg = "Member with ID '" + memberId + "' does not exist.";
        return false;
    }

    // Rule 4: Member must not exceed borrow limit
    if (!member->canBorrow()) {
        errorMsg = "Member '" + member->getName() + "' (ID: " + memberId +
                   ") has reached the maximum borrow limit of " +
                   std::to_string(member->getMaxBorrowLimit()) + " books.";
        return false;
    }

    // Rule 5: Dates must be valid calendar dates
    if (!DateUtils::isValidDate(issueDate)) {
        errorMsg = "Invalid issue date '" + issueDate + "'. Must be valid YYYY-MM-DD.";
        return false;
    }
    if (!DateUtils::isValidDate(dueDate)) {
        errorMsg = "Invalid due date '" + dueDate + "'. Must be valid YYYY-MM-DD.";
        return false;
    }
    if (!DateUtils::isDateLessOrEqual(issueDate, dueDate)) {
        errorMsg = "Due date (" + dueDate + ") cannot be earlier than issue date (" + issueDate + ").";
        return false;
    }

    // Apply state changes atomically
    book->markIssued(memberId, dueDate);
    member->borrowBook(bookId);
    loans_.emplace_back(bookId, memberId, issueDate, dueDate, "");

    return true;
}

bool Library::returnBook(const std::string& bookId,
                         const std::string& returnDate,
                         std::string& errorMsg) {
    // Rule 1: Book must exist
    Book* book = findBook(bookId);
    if (book == nullptr) {
        errorMsg = "Book with ID '" + bookId + "' does not exist.";
        return false;
    }

    // Rule 2: Active loan must exist for this book
    Loan* loan = findActiveLoan(bookId);
    if (loan == nullptr) {
        errorMsg = "No active loan found for book '" + book->getTitle() + "' (ID: " + bookId + ").";
        return false;
    }

    // Rule 3: Return date must be valid
    if (!DateUtils::isValidDate(returnDate)) {
        errorMsg = "Invalid return date '" + returnDate + "'. Must be valid YYYY-MM-DD.";
        return false;
    }

    // Apply state changes
    loan->markReturned(returnDate);
    book->markReturned();

    Member* member = findMember(loan->getMemberId());
    if (member != nullptr) {
        member->returnBook(bookId);
    }

    return true;
}

const Loan* Library::findActiveLoan(const std::string& bookId) const {
    auto it = findActiveLoanIter(bookId);
    if (it != loans_.end()) {
        return &(*it);
    }
    return nullptr;
}

Loan* Library::findActiveLoan(const std::string& bookId) {
    auto it = findActiveLoanIter(bookId);
    if (it != loans_.end()) {
        return &(*it);
    }
    return nullptr;
}

std::vector<Loan> Library::getActiveLoans() const {
    std::vector<Loan> active;
    for (const auto& loan : loans_) {
        if (!loan.isReturned()) {
            active.push_back(loan);
        }
    }
    return active;
}

const std::vector<Loan>& Library::getLoans() const {
    return loans_;
}

void Library::rawAddLoan(const Loan& loan) {
    loans_.push_back(loan);
}
