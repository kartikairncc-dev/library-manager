#ifndef LIBRARY_H
#define LIBRARY_H

#include "Book.h"
#include "Member.h"
#include "Loan.h"

#include <vector>
#include <string>

/**
 * @class Library
 * @brief Domain manager class controlling all business logic and collection state.
 * 
 * Strict architectural rule:
 * - NO std::cin or std::cout within this class.
 * - All methods return status codes, booleans, and descriptive error messages.
 * - Owns internal containers for Book, Member, and Loan instances.
 */
class Library {
private:
    std::vector<Book> books_;     ///< Collection of library books
    std::vector<Member> members_; ///< Registered library members
    std::vector<Loan> loans_;     ///< All loan transactions (active and historical)

    // Helper lookup iterators
    std::vector<Book>::iterator findBookIter(const std::string& bookId);
    std::vector<Book>::const_iterator findBookIter(const std::string& bookId) const;

    std::vector<Member>::iterator findMemberIter(const std::string& memberId);
    std::vector<Member>::const_iterator findMemberIter(const std::string& memberId) const;

    std::vector<Loan>::iterator findActiveLoanIter(const std::string& bookId);
    std::vector<Loan>::const_iterator findActiveLoanIter(const std::string& bookId) const;

public:
    /**
     * @brief Default constructor.
     */
    Library() = default;

    /**
     * @brief Clears all internal collections (used on fresh reload or tests).
     */
    void clear();

    // ==========================================
    // Book Management
    // ==========================================

    /**
     * @brief Adds a new book to the library collection.
     * @param book Book entity to add.
     * @param errorMsg Output message on validation failure.
     * @return true if added, false if ID is duplicate or invalid.
     */
    bool addBook(const Book& book, std::string& errorMsg);

    /**
     * @brief Finds a book by ID (read-only).
     * @param bookId Unique book ID.
     * @return Pointer to Book, or nullptr if not found.
     */
    const Book* findBook(const std::string& bookId) const;

    /**
     * @brief Finds a book by ID (mutable).
     * @param bookId Unique book ID.
     * @return Pointer to Book, or nullptr if not found.
     */
    Book* findBook(const std::string& bookId);

    /**
     * @brief Gets read-only reference to all books.
     * @return Const vector reference.
     */
    const std::vector<Book>& getBooks() const;

    /**
     * @brief Searches books by title, author, or category (case-insensitive substring).
     * @param query Search query text.
     * @return Vector of matching books.
     */
    std::vector<Book> searchBooks(const std::string& query) const;

    /**
     * @brief Checks the availability status of a book.
     * @param bookId Book identifier.
     * @param isAvailable Output boolean flag.
     * @param statusDetails Output human-readable status explanation.
     * @param errorMsg Output error message if book does not exist.
     * @return true if book exists, false if book not found.
     */
    bool checkAvailability(const std::string& bookId,
                           bool& isAvailable,
                           std::string& statusDetails,
                           std::string& errorMsg) const;

    // ==========================================
    // Member Management
    // ==========================================

    /**
     * @brief Registers a new member in the library.
     * @param member Member entity to add.
     * @param errorMsg Output message on validation failure.
     * @return true if added, false if ID is duplicate or invalid.
     */
    bool addMember(const Member& member, std::string& errorMsg);

    /**
     * @brief Finds a member by ID (read-only).
     * @param memberId Unique member ID.
     * @return Pointer to Member, or nullptr if not found.
     */
    const Member* findMember(const std::string& memberId) const;

    /**
     * @brief Finds a member by ID (mutable).
     * @param memberId Unique member ID.
     * @return Pointer to Member, or nullptr if not found.
     */
    Member* findMember(const std::string& memberId);

    /**
     * @brief Gets read-only reference to all members.
     * @return Const vector reference.
     */
    const std::vector<Member>& getMembers() const;

    // ==========================================
    // Loan & Borrowing Operations
    // ==========================================

    /**
     * @brief Issues a book to a member following all library business rules.
     * Rules enforced:
     *  1. Book must exist.
     *  2. Book must be available.
     *  3. Member must exist.
     *  4. Member borrow limit must not be exceeded.
     *  5. issueDate and dueDate must be valid YYYY-MM-DD.
     *  6. dueDate must be >= issueDate.
     * 
     * @param bookId ID of book to issue.
     * @param memberId ID of member borrowing the book.
     * @param issueDate Start date (YYYY-MM-DD).
     * @param dueDate Due date (YYYY-MM-DD).
     * @param errorMsg Output error message if any rule fails.
     * @return true if loan created successfully, false otherwise.
     */
    bool issueBook(const std::string& bookId,
                   const std::string& memberId,
                   const std::string& issueDate,
                   const std::string& dueDate,
                   std::string& errorMsg);

    /**
     * @brief Returns an issued book and closes its active loan.
     * Rules enforced:
     *  1. Book must exist.
     *  2. An active loan must exist for this book.
     *  3. returnDate must be valid YYYY-MM-DD.
     * 
     * @param bookId ID of book being returned.
     * @param returnDate Actual return date (YYYY-MM-DD).
     * @param errorMsg Output error message if any rule fails.
     * @return true if returned successfully, false otherwise.
     */
    bool returnBook(const std::string& bookId,
                    const std::string& returnDate,
                    std::string& errorMsg);

    /**
     * @brief Finds the active (unreturned) loan for a book.
     * @param bookId Book identifier.
     * @return Pointer to Loan, or nullptr if not currently on loan.
     */
    const Loan* findActiveLoan(const std::string& bookId) const;

    /**
     * @brief Finds the active (unreturned) loan for a book (mutable).
     * @param bookId Book identifier.
     * @return Pointer to Loan, or nullptr if not currently on loan.
     */
    Loan* findActiveLoan(const std::string& bookId);

    /**
     * @brief Retrieves all currently active (unreturned) loans.
     * @return Vector of active loans.
     */
    std::vector<Loan> getActiveLoans() const;

    /**
     * @brief Retrieves all loans (both active and returned).
     * @return Const vector reference.
     */
    const std::vector<Loan>& getLoans() const;

    // Direct helper for storage loader to reconstruct loan history
    void rawAddLoan(const Loan& loan);
};

#endif // LIBRARY_H
