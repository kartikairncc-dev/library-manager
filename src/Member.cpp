#include "Member.h"
#include "DateUtils.h"
#include <algorithm>
#include <sstream>

Member::Member(std::string id, std::string name, int maxBorrowLimit)
    : id_(std::move(id)),
      name_(std::move(name)),
      maxBorrowLimit_(maxBorrowLimit > 0 ? maxBorrowLimit : 3) {}

// Getters
const std::string& Member::getId() const { return id_; }
const std::string& Member::getName() const { return name_; }
const std::vector<std::string>& Member::getBorrowedBookIds() const { return borrowedBookIds_; }
int Member::getMaxBorrowLimit() const { return maxBorrowLimit_; }
size_t Member::getBorrowedCount() const { return borrowedBookIds_.size(); }

// Validated Setters
bool Member::setId(const std::string& id) {
    std::string trimmed = DateUtils::trim(id);
    if (trimmed.empty()) {
        return false;
    }
    id_ = trimmed;
    return true;
}

bool Member::setName(const std::string& name) {
    std::string trimmed = DateUtils::trim(name);
    if (trimmed.empty()) {
        return false;
    }
    name_ = trimmed;
    return true;
}

bool Member::setMaxBorrowLimit(int limit) {
    if (limit <= 0) {
        return false;
    }
    maxBorrowLimit_ = limit;
    return true;
}

// Borrowing Rules & State
bool Member::canBorrow() const {
    return borrowedBookIds_.size() < static_cast<size_t>(maxBorrowLimit_);
}

bool Member::borrowBook(const std::string& bookId) {
    if (!canBorrow()) {
        return false;
    }
    if (hasBorrowed(bookId)) {
        return false; // Already borrowed by this member
    }
    borrowedBookIds_.push_back(bookId);
    return true;
}

bool Member::returnBook(const std::string& bookId) {
    auto it = std::find(borrowedBookIds_.begin(), borrowedBookIds_.end(), bookId);
    if (it != borrowedBookIds_.end()) {
        borrowedBookIds_.erase(it);
        return true;
    }
    return false;
}

bool Member::hasBorrowed(const std::string& bookId) const {
    return std::find(borrowedBookIds_.begin(), borrowedBookIds_.end(), bookId) != borrowedBookIds_.end();
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

std::string Member::toCSV() const {
    std::ostringstream oss;
    oss << escapeForCSV(id_) << ","
        << escapeForCSV(name_) << ","
        << maxBorrowLimit_;
    return oss.str();
}
