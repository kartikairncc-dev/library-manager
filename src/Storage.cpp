#include "Storage.h"
#include "DateUtils.h"

#include <fstream>
#include <sstream>
#include <set>
#include <cstdio>
#include <iostream>

Storage::Storage(std::string booksFile,
                 std::string membersFile,
                 std::string loansFile,
                 std::string seedFile)
    : booksFilePath_(std::move(booksFile)),
      membersFilePath_(std::move(membersFile)),
      loansFilePath_(std::move(loansFile)),
      seedFilePath_(std::move(seedFile)) {}

bool Storage::fileExists(const std::string& filepath) {
    std::ifstream f(filepath.c_str());
    return f.good();
}

std::vector<std::string> Storage::parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                current += '"';
                ++i; // Skip escaped quote
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            fields.push_back(current);
            current.clear();
        } else if (c != '\r') {
            current += c;
        }
    }
    fields.push_back(current);
    return fields;
}

bool Storage::atomicWrite(const std::string& targetPath,
                          const std::string& content,
                          std::string& errorMsg) {
    std::string tempPath = targetPath + ".tmp";

    // 1. Open temporary file using RAII (std::ofstream destructor closes file)
    {
        std::ofstream out(tempPath.c_str(), std::ios::out | std::ios::trunc);
        if (!out.is_open()) {
            errorMsg = "Unable to open temporary file for write: " + tempPath;
            return false;
        }

        out << content;
        if (!out.good()) {
            errorMsg = "Failed while writing content to temporary file: " + tempPath;
            out.close();
            std::remove(tempPath.c_str());
            return false;
        }
        out.flush();
    } // File stream is closed here via RAII

    // 2. Safely replace target with temp file
    // On Windows, std::rename fails if destination file exists, so remove it first.
    std::remove(targetPath.c_str());

    if (std::rename(tempPath.c_str(), targetPath.c_str()) != 0) {
        errorMsg = "Failed to rename temporary file '" + tempPath + "' to '" + targetPath + "'.";
        return false;
    }

    return true;
}

// ==========================================
// Book Persistence
// ==========================================

bool Storage::loadBooksCSV(const std::string& filepath,
                           std::vector<Book>& outBooks,
                           std::vector<std::string>& warnings,
                           std::string& errorMsg) const {
    std::ifstream in(filepath.c_str());
    if (!in.is_open()) {
        errorMsg = "Cannot open books file: " + filepath;
        return false;
    }

    std::string line;
    size_t lineNum = 0;
    std::set<std::string> seenIds;

    // Read header line
    if (std::getline(in, line)) {
        ++lineNum;
        // Verify or skip header
    }

    while (std::getline(in, line)) {
        ++lineNum;
        std::string trimmedLine = DateUtils::trim(line);
        if (trimmedLine.empty()) {
            continue; // Skip blank lines
        }

        std::vector<std::string> fields = parseCSVLine(trimmedLine);
        // Schema: book_id,title,author,category,year,status,borrower_id,due_date
        if (fields.size() < 6) {
            warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                               ": malformed row (expected at least 6 columns, got " +
                               std::to_string(fields.size()) + "). Skipping.");
            continue;
        }

        std::string bookId = DateUtils::trim(fields[0]);
        std::string title = DateUtils::trim(fields[1]);
        std::string author = DateUtils::trim(fields[2]);
        std::string category = DateUtils::trim(fields[3]);
        std::string yearStr = DateUtils::trim(fields[4]);
        std::string status = DateUtils::toLower(DateUtils::trim(fields[5]));
        std::string borrowerId = fields.size() > 6 ? DateUtils::trim(fields[6]) : "";
        std::string dueDate = fields.size() > 7 ? DateUtils::trim(fields[7]) : "";

        // Validate required fields
        if (bookId.empty()) {
            warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                               ": empty book ID. Skipping.");
            continue;
        }

        // Check for duplicate ID
        if (seenIds.find(bookId) != seenIds.end()) {
            warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                               ": duplicate book ID '" + bookId +
                               "' detected. Rejecting duplicate and keeping first entry.");
            continue;
        }

        int year = 0;
        try {
            year = std::stoi(yearStr);
        } catch (...) {
            warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                               ": invalid publication year '" + yearStr + "'. Skipping.");
            continue;
        }

        if (year <= 0 || year > 2100) {
            warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                               ": out-of-range publication year " + std::to_string(year) + ". Skipping.");
            continue;
        }

        if (status != "available" && status != "borrowed") {
            warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                               ": unrecognized status '" + status + "'. Skipping.");
            continue;
        }

        if (status == "borrowed") {
            if (!dueDate.empty() && !DateUtils::isValidDate(dueDate)) {
                warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                                   ": invalid due date format '" + dueDate + "' for borrowed book " + bookId + ".");
            }
        }

        seenIds.insert(bookId);
        outBooks.emplace_back(bookId, title, author, category, year, status, borrowerId, dueDate);
    }

    return true;
}

bool Storage::saveBooksCSV(const std::string& filepath,
                           const std::vector<Book>& books,
                           std::string& errorMsg) const {
    std::ostringstream oss;
    oss << "book_id,title,author,category,year,status,borrower_id,due_date\n";
    for (const auto& book : books) {
        oss << book.toCSV() << "\n";
    }
    return atomicWrite(filepath, oss.str(), errorMsg);
}

// ==========================================
// Member Persistence
// ==========================================

bool Storage::loadMembersCSV(const std::string& filepath,
                             std::vector<Member>& outMembers,
                             std::vector<std::string>& warnings,
                             std::string& errorMsg) const {
    std::ifstream in(filepath.c_str());
    if (!in.is_open()) {
        errorMsg = "Cannot open members file: " + filepath;
        return false;
    }

    std::string line;
    size_t lineNum = 0;
    std::set<std::string> seenIds;

    if (std::getline(in, line)) {
        ++lineNum; // Skip header
    }

    while (std::getline(in, line)) {
        ++lineNum;
        std::string trimmed = DateUtils::trim(line);
        if (trimmed.empty()) continue;

        std::vector<std::string> fields = parseCSVLine(trimmed);
        if (fields.size() < 2) {
            warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                               ": malformed member row. Skipping.");
            continue;
        }

        std::string id = DateUtils::trim(fields[0]);
        std::string name = DateUtils::trim(fields[1]);
        int limit = 3;
        if (fields.size() > 2) {
            try {
                limit = std::stoi(DateUtils::trim(fields[2]));
                if (limit <= 0) limit = 3;
            } catch (...) {
                limit = 3;
            }
        }

        if (id.empty() || name.empty()) {
            warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                               ": missing member ID or name. Skipping.");
            continue;
        }

        if (seenIds.find(id) != seenIds.end()) {
            warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                               ": duplicate member ID '" + id +
                               "' detected. Rejecting duplicate and keeping first entry.");
            continue;
        }

        seenIds.insert(id);
        outMembers.emplace_back(id, name, limit);
    }
    return true;
}

bool Storage::saveMembersCSV(const std::string& filepath,
                             const std::vector<Member>& members,
                             std::string& errorMsg) const {
    std::ostringstream oss;
    oss << "member_id,name,max_borrow_limit\n";
    for (const auto& member : members) {
        oss << member.toCSV() << "\n";
    }
    return atomicWrite(filepath, oss.str(), errorMsg);
}

// ==========================================
// Loan Persistence
// ==========================================

bool Storage::loadLoansCSV(const std::string& filepath,
                           std::vector<Loan>& outLoans,
                           std::vector<std::string>& warnings,
                           std::string& errorMsg) const {
    std::ifstream in(filepath.c_str());
    if (!in.is_open()) {
        errorMsg = "Cannot open loans file: " + filepath;
        return false;
    }

    std::string line;
    size_t lineNum = 0;

    if (std::getline(in, line)) {
        ++lineNum; // Skip header
    }

    while (std::getline(in, line)) {
        ++lineNum;
        std::string trimmed = DateUtils::trim(line);
        if (trimmed.empty()) continue;

        std::vector<std::string> fields = parseCSVLine(trimmed);
        if (fields.size() < 4) {
            warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                               ": malformed loan row. Skipping.");
            continue;
        }

        std::string bookId = DateUtils::trim(fields[0]);
        std::string memberId = DateUtils::trim(fields[1]);
        std::string issueDate = DateUtils::trim(fields[2]);
        std::string dueDate = DateUtils::trim(fields[3]);
        std::string returnDate = fields.size() > 4 ? DateUtils::trim(fields[4]) : "";

        if (bookId.empty() || memberId.empty()) {
            warnings.push_back("File '" + filepath + "', line " + std::to_string(lineNum) +
                               ": empty bookId or memberId in loan record. Skipping.");
            continue;
        }

        outLoans.emplace_back(bookId, memberId, issueDate, dueDate, returnDate);
    }
    return true;
}

bool Storage::saveLoansCSV(const std::string& filepath,
                           const std::vector<Loan>& loans,
                           std::string& errorMsg) const {
    std::ostringstream oss;
    oss << "book_id,member_id,issue_date,due_date,return_date\n";
    for (const auto& loan : loans) {
        oss << loan.toCSV() << "\n";
    }
    return atomicWrite(filepath, oss.str(), errorMsg);
}

// ==========================================
// High-Level Library Operations
// ==========================================

bool Storage::load(Library& lib,
                   std::vector<std::string>& warnings,
                   std::string& errorMsg) const {
    lib.clear();

    bool dataFileExists = fileExists(booksFilePath_);
    std::string loadPath = booksFilePath_;

    if (!dataFileExists) {
        // Runtime data file missing - check for seed CSV
        if (fileExists(seedFilePath_)) {
            warnings.push_back("Runtime data file '" + booksFilePath_ +
                               "' not found. Importing seed data from '" + seedFilePath_ + "'...");
            loadPath = seedFilePath_;
        } else {
            warnings.push_back("No runtime data file or seed file found. Starting with a fresh library.");
            return true;
        }
    }

    // 1. Load Books
    std::vector<Book> books;
    if (!loadBooksCSV(loadPath, books, warnings, errorMsg)) {
        return false;
    }

    for (const auto& b : books) {
        std::string err;
        lib.addBook(b, err);
    }

    // 2. Load Members
    std::vector<Member> members;
    if (fileExists(membersFilePath_)) {
        loadMembersCSV(membersFilePath_, members, warnings, errorMsg);
        for (const auto& m : members) {
            std::string err;
            lib.addMember(m, err);
        }
    }

    // 3. Load Loans
    std::vector<Loan> loans;
    if (fileExists(loansFilePath_)) {
        loadLoansCSV(loansFilePath_, loans, warnings, errorMsg);
        for (const auto& l : loans) {
            lib.rawAddLoan(l);
        }
    }

    // 4. Reconcile Seed or missing Member/Loan state:
    // If books are marked borrowed (e.g. from seed CSV or initial import), ensure member exists
    // and active loan is tracked.
    for (const auto& book : lib.getBooks()) {
        if (!book.isAvailable() && !book.getBorrowerId().empty()) {
            std::string memberId = book.getBorrowerId();
            Member* member = lib.findMember(memberId);
            if (member == nullptr) {
                // Auto-create patron record for existing borrower in seed data
                std::string memberName = "Member " + memberId;
                Member newMember(memberId, memberName, 3);
                std::string err;
                lib.addMember(newMember, err);
                member = lib.findMember(memberId);
            }

            if (member != nullptr && !member->hasBorrowed(book.getId())) {
                member->borrowBook(book.getId());
            }

            // If loan record does not exist for this active borrow, synthesize it
            if (lib.findActiveLoan(book.getId()) == nullptr) {
                std::string issue = book.getDueDate().empty() ? DateUtils::getTodayDate()
                                                              : DateUtils::addDaysToDate(book.getDueDate(), -14);
                Loan synLoan(book.getId(), memberId, issue, book.getDueDate(), "");
                lib.rawAddLoan(synLoan);
            }
        }
    }

    // If we just loaded from seed data, save immediately to create the runtime data files
    if (!dataFileExists && fileExists(seedFilePath_)) {
        std::string saveErr;
        if (!save(lib, saveErr)) {
            warnings.push_back("Warning: Failed to create initial runtime data files: " + saveErr);
        }
    }

    return true;
}

bool Storage::save(const Library& lib, std::string& errorMsg) const {
    if (!saveBooksCSV(booksFilePath_, lib.getBooks(), errorMsg)) {
        return false;
    }
    if (!saveMembersCSV(membersFilePath_, lib.getMembers(), errorMsg)) {
        return false;
    }
    if (!saveLoansCSV(loansFilePath_, lib.getLoans(), errorMsg)) {
        return false;
    }
    return true;
}
