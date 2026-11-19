#ifndef MEMBER_H
#define MEMBER_H

#include <string>
#include <vector>

/**
 * @class Member
 * @brief Represents a library patron/member who can borrow books up to a maximum limit.
 * 
 * Encapsulation:
 * - Member attributes are private.
 * - Borrow limit enforcement is handled directly in member operations.
 */
class Member {
private:
    std::string id_;                           ///< Unique member identifier (e.g., "M001")
    std::string name_;                         ///< Full name of the member
    std::vector<std::string> borrowedBookIds_; ///< IDs of books currently borrowed
    int maxBorrowLimit_{3};                    ///< Maximum allowed simultaneous borrows (default 3)

public:
    /**
     * @brief Default constructor.
     */
    Member() = default;

    /**
     * @brief Parameterized constructor.
     * @param id Unique member ID.
     * @param name Name of the member.
     * @param maxBorrowLimit Maximum books the member can borrow simultaneously (default 3).
     */
    Member(std::string id, std::string name, int maxBorrowLimit = 3);

    // --- Getters ---
    const std::string& getId() const;
    const std::string& getName() const;
    const std::vector<std::string>& getBorrowedBookIds() const;
    int getMaxBorrowLimit() const;
    size_t getBorrowedCount() const;

    // --- Validated Setters ---
    bool setId(const std::string& id);
    bool setName(const std::string& name);
    bool setMaxBorrowLimit(int limit);

    // --- Borrowing Rules & State ---

    /**
     * @brief Checks if member has capacity to borrow another book.
     * @return true if currently borrowed count < maxBorrowLimit_.
     */
    bool canBorrow() const;

    /**
     * @brief Adds a book ID to the member's active borrow list if limit not reached.
     * @param bookId The book ID to borrow.
     * @return true if successfully added, false if at limit or already borrowed.
     */
    bool borrowBook(const std::string& bookId);

    /**
     * @brief Removes a book ID from the member's active borrow list.
     * @param bookId The book ID being returned.
     * @return true if book was found and returned, false otherwise.
     */
    bool returnBook(const std::string& bookId);

    /**
     * @brief Checks if the member has currently borrowed the given book ID.
     * @param bookId The book ID to check.
     * @return true if currently borrowed, false otherwise.
     */
    bool hasBorrowed(const std::string& bookId) const;

    /**
     * @brief Serializes member record into CSV format: member_id,name,max_limit
     */
    std::string toCSV() const;
};

#endif // MEMBER_H
