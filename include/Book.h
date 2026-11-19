#ifndef BOOK_H
#define BOOK_H

#include <string>

/**
 * @class Book
 * @brief Represents a book entity in the library system.
 * 
 * Follows strict Object-Oriented encapsulation principles:
 * - Private member variables protect data integrity.
 * - Public getters provide read-only access.
 * - Validated setters enforce business rules before mutating state.
 * 
 * Fields match the exact seed CSV structure:
 * (book_id, title, author, category, year, status, borrower_id, due_date)
 */
class Book {
private:
    std::string id_;         ///< Unique book identifier (e.g., "B001")
    std::string title_;      ///< Title of the book
    std::string author_;     ///< Author(s) of the book
    std::string category_;   ///< Category or genre (from seed CSV: e.g., "Software")
    int year_{0};            ///< Publication year (from seed CSV: e.g., 1999)
    std::string status_{"available"}; ///< Availability status: "available" or "borrowed"
    std::string borrowerId_{""};      ///< Member ID of current borrower (empty if available)
    std::string dueDate_{""};         ///< Due date in YYYY-MM-DD (empty if available)

public:
    /**
     * @brief Default constructor creating an uninitialized book.
     */
    Book() = default;

    /**
     * @brief Parameterized constructor matching the exact CSV schema.
     * @param id Unique book ID.
     * @param title Title of the book.
     * @param author Author of the book.
     * @param category Genre/category.
     * @param year Publication year.
     * @param status "available" or "borrowed" (default "available").
     * @param borrowerId ID of member borrowing the book (empty if available).
     * @param dueDate Expected return date YYYY-MM-DD (empty if available).
     */
    Book(std::string id,
         std::string title,
         std::string author,
         std::string category,
         int year,
         std::string status = "available",
         std::string borrowerId = "",
         std::string dueDate = "");

    // --- Getters (Read-only access) ---
    const std::string& getId() const;
    const std::string& getTitle() const;
    const std::string& getAuthor() const;
    const std::string& getCategory() const;
    int getYear() const;
    const std::string& getStatus() const;
    const std::string& getBorrowerId() const;
    const std::string& getDueDate() const;

    // --- Validated Setters (Encapsulation with validation) ---
    bool setId(const std::string& id);
    bool setTitle(const std::string& title);
    bool setAuthor(const std::string& author);
    bool setCategory(const std::string& category);
    bool setYear(int year);
    bool setStatus(const std::string& status);
    bool setBorrowerId(const std::string& borrowerId);
    bool setDueDate(const std::string& dueDate);

    // --- Business State Methods ---

    /**
     * @brief Checks if the book is currently available for loan.
     * @return true if status is "available", false otherwise.
     */
    bool isAvailable() const;

    /**
     * @brief Marks the book as issued to a borrower with a due date.
     * @param borrowerId ID of the borrowing member.
     * @param dueDate Due date in YYYY-MM-DD format.
     */
    void markIssued(const std::string& borrowerId, const std::string& dueDate);

    /**
     * @brief Marks the book as returned, resetting borrower and due date.
     */
    void markReturned();

    /**
     * @brief Serializes the book into a CSV row matching the exact schema.
     * @return Formatted CSV line.
     */
    std::string toCSV() const;
};

#endif // BOOK_H
