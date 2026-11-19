#include "Book.h"
#include "DateUtils.h"
#include <sstream>

Book::Book(std::string id,
           std::string title,
           std::string author,
           std::string category,
           int year,
           std::string status,
           std::string borrowerId,
           std::string dueDate)
    : id_(std::move(id)),
      title_(std::move(title)),
      author_(std::move(author)),
      category_(std::move(category)),
      year_(year),
      status_(std::move(status)),
      borrowerId_(std::move(borrowerId)),
      dueDate_(std::move(dueDate)) {
    // Normalize status string
    std::string lowerStatus = DateUtils::toLower(status_);
    if (lowerStatus == "borrowed") {
        status_ = "borrowed";
    } else {
        status_ = "available";
    }
}

// Getters
const std::string& Book::getId() const { return id_; }
const std::string& Book::getTitle() const { return title_; }
const std::string& Book::getAuthor() const { return author_; }
const std::string& Book::getCategory() const { return category_; }
int Book::getYear() const { return year_; }
const std::string& Book::getStatus() const { return status_; }
const std::string& Book::getBorrowerId() const { return borrowerId_; }
const std::string& Book::getDueDate() const { return dueDate_; }

// Validated Setters
bool Book::setId(const std::string& id) {
    std::string trimmed = DateUtils::trim(id);
    if (trimmed.empty()) {
        return false;
    }
    id_ = trimmed;
    return true;
}

bool Book::setTitle(const std::string& title) {
    std::string trimmed = DateUtils::trim(title);
    if (trimmed.empty()) {
        return false;
    }
    title_ = trimmed;
    return true;
}

bool Book::setAuthor(const std::string& author) {
    std::string trimmed = DateUtils::trim(author);
    if (trimmed.empty()) {
        return false;
    }
    author_ = trimmed;
    return true;
}

bool Book::setCategory(const std::string& category) {
    category_ = DateUtils::trim(category);
    return true;
}

bool Book::setYear(int year) {
    if (year <= 0 || year > 2100) {
        return false;
    }
    year_ = year;
    return true;
}

bool Book::setStatus(const std::string& status) {
    std::string lower = DateUtils::toLower(DateUtils::trim(status));
    if (lower != "available" && lower != "borrowed") {
        return false;
    }
    status_ = lower;
    return true;
}

bool Book::setBorrowerId(const std::string& borrowerId) {
    borrowerId_ = DateUtils::trim(borrowerId);
    return true;
}

bool Book::setDueDate(const std::string& dueDate) {
    std::string trimmed = DateUtils::trim(dueDate);
    if (trimmed.empty()) {
        dueDate_.clear();
        return true;
    }
    if (!DateUtils::isValidDate(trimmed)) {
        return false;
    }
    dueDate_ = trimmed;
    return true;
}

// Business State Methods
bool Book::isAvailable() const {
    return status_ == "available";
}

void Book::markIssued(const std::string& borrowerId, const std::string& dueDate) {
    status_ = "borrowed";
    borrowerId_ = borrowerId;
    dueDate_ = dueDate;
}

void Book::markReturned() {
    status_ = "available";
    borrowerId_.clear();
    dueDate_.clear();
}

static std::string escapeForCSV(const std::string& text) {
    if (text.find(',') != std::string::npos ||
        text.find('"') != std::string::npos ||
        text.find('\n') != std::string::npos) {
        std::string result = "\"";
        for (char c : text) {
            if (c == '"') {
                result += "\"\"";
            } else {
                result += c;
            }
        }
        result += "\"";
        return result;
    }
    return text;
}

std::string Book::toCSV() const {
    // Exactly matches CSV schema:
    // book_id,title,author,category,year,status,borrower_id,due_date
    std::ostringstream oss;
    oss << escapeForCSV(id_) << ","
        << escapeForCSV(title_) << ","
        << escapeForCSV(author_) << ","
        << escapeForCSV(category_) << ","
        << year_ << ","
        << status_ << ","
        << escapeForCSV(borrowerId_) << ","
        << dueDate_;
    return oss.str();
}
