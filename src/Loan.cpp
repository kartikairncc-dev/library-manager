#include "Loan.h"
#include "DateUtils.h"
#include <sstream>

Loan::Loan(std::string bookId,
           std::string memberId,
           std::string issueDate,
           std::string dueDate,
           std::string returnDate)
    : bookId_(std::move(bookId)),
      memberId_(std::move(memberId)),
      issueDate_(std::move(issueDate)),
      dueDate_(std::move(dueDate)),
      returnDate_(std::move(returnDate)) {}

// Getters
const std::string& Loan::getBookId() const { return bookId_; }
const std::string& Loan::getMemberId() const { return memberId_; }
const std::string& Loan::getIssueDate() const { return issueDate_; }
const std::string& Loan::getDueDate() const { return dueDate_; }
const std::string& Loan::getReturnDate() const { return returnDate_; }

// Validated Setters
bool Loan::setBookId(const std::string& bookId) {
    std::string trimmed = DateUtils::trim(bookId);
    if (trimmed.empty()) {
        return false;
    }
    bookId_ = trimmed;
    return true;
}

bool Loan::setMemberId(const std::string& memberId) {
    std::string trimmed = DateUtils::trim(memberId);
    if (trimmed.empty()) {
        return false;
    }
    memberId_ = trimmed;
    return true;
}

bool Loan::setIssueDate(const std::string& issueDate) {
    std::string trimmed = DateUtils::trim(issueDate);
    if (!DateUtils::isValidDate(trimmed)) {
        return false;
    }
    issueDate_ = trimmed;
    return true;
}

bool Loan::setDueDate(const std::string& dueDate) {
    std::string trimmed = DateUtils::trim(dueDate);
    if (!DateUtils::isValidDate(trimmed)) {
        return false;
    }
    dueDate_ = trimmed;
    return true;
}

bool Loan::setReturnDate(const std::string& returnDate) {
    std::string trimmed = DateUtils::trim(returnDate);
    if (trimmed.empty()) {
        returnDate_.clear();
        return true;
    }
    if (!DateUtils::isValidDate(trimmed)) {
        return false;
    }
    returnDate_ = trimmed;
    return true;
}

// Business Methods
bool Loan::isReturned() const {
    return !returnDate_.empty();
}

bool Loan::isOverdue(const std::string& currentDate) const {
    if (isReturned()) {
        return false;
    }
    std::string checkDate = currentDate.empty() ? DateUtils::getTodayDate() : currentDate;
    // Overdue if checkDate strictly exceeds dueDate
    return checkDate > dueDate_;
}

void Loan::markReturned(const std::string& returnDate) {
    returnDate_ = returnDate;
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

std::string Loan::toCSV() const {
    std::ostringstream oss;
    oss << escapeForCSV(bookId_) << ","
        << escapeForCSV(memberId_) << ","
        << issueDate_ << ","
        << dueDate_ << ","
        << returnDate_;
    return oss.str();
}
