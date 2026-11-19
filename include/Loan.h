#ifndef LOAN_H
#define LOAN_H

#include <string>

/**
 * @class Loan
 * @brief Represents a loan transaction connecting a Book and a Member.
 * 
 * Tracks:
 * - bookId: Which book was borrowed
 * - memberId: Who borrowed it
 * - issueDate: When the book was issued (YYYY-MM-DD)
 * - dueDate: When the book is due back (YYYY-MM-DD)
 * - returnDate: When the book was actually returned (YYYY-MM-DD, empty if active)
 */
class Loan {
private:
    std::string bookId_;      ///< ID of borrowed book
    std::string memberId_;    ///< ID of borrowing member
    std::string issueDate_;   ///< Date issued (YYYY-MM-DD)
    std::string dueDate_;     ///< Due date (YYYY-MM-DD)
    std::string returnDate_;  ///< Date returned (empty if currently active)

public:
    /**
     * @brief Default constructor.
     */
    Loan() = default;

    /**
     * @brief Parameterized constructor.
     * @param bookId Book identifier.
     * @param memberId Member identifier.
     * @param issueDate Date loan started (YYYY-MM-DD).
     * @param dueDate Date loan is due (YYYY-MM-DD).
     * @param returnDate Date book was returned (empty if still active).
     */
    Loan(std::string bookId,
         std::string memberId,
         std::string issueDate,
         std::string dueDate,
         std::string returnDate = "");

    // --- Getters ---
    const std::string& getBookId() const;
    const std::string& getMemberId() const;
    const std::string& getIssueDate() const;
    const std::string& getDueDate() const;
    const std::string& getReturnDate() const;

    // --- Validated Setters ---
    bool setBookId(const std::string& bookId);
    bool setMemberId(const std::string& memberId);
    bool setIssueDate(const std::string& issueDate);
    bool setDueDate(const std::string& dueDate);
    bool setReturnDate(const std::string& returnDate);

    // --- Business Methods ---

    /**
     * @brief Checks if the loan has been closed/returned.
     * @return true if returnDate is non-empty.
     */
    bool isReturned() const;

    /**
     * @brief Checks if the loan is overdue relative to a given date.
     *        If currentDate is empty, checks against today's date.
     *        A returned loan is not considered active/overdue.
     * @param currentDate Reference date in YYYY-MM-DD format (or empty for today).
     * @return true if active loan and currentDate > dueDate.
     */
    bool isOverdue(const std::string& currentDate = "") const;

    /**
     * @brief Closes the loan by recording a return date.
     * @param returnDate Return date in YYYY-MM-DD format.
     */
    void markReturned(const std::string& returnDate);

    /**
     * @brief Serializes loan into CSV row: book_id,member_id,issue_date,due_date,return_date
     */
    std::string toCSV() const;
};

#endif // LOAN_H
