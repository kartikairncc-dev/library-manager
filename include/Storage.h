#ifndef STORAGE_H
#define STORAGE_H

#include "Library.h"
#include <string>
#include <vector>

/**
 * @class Storage
 * @brief Handles file persistence using RAII and atomic file replacements.
 * 
 * Safety & Defensive Architecture:
 * - Atomic write: writes data to a temporary file (`.tmp`) first, flushes and closes it,
 *   then renames it to the target file. If a crash occurs during write, the existing
 *   data file remains untouched and uncorrupted.
 * - Missing file handling: gracefully starts fresh or imports seed CSV.
 * - Malformed row handling: skips corrupted lines and accumulates descriptive warnings.
 * - Duplicate ID handling: rejects duplicates, preserves first-loaded entity.
 * - Exact schema alignment: Book CSV matches seed CSV columns precisely:
 *   (book_id,title,author,category,year,status,borrower_id,due_date)
 */
class Storage {
private:
    std::string booksFilePath_;   ///< Path to runtime books CSV
    std::string membersFilePath_; ///< Path to runtime members CSV
    std::string loansFilePath_;   ///< Path to runtime loans CSV
    std::string seedFilePath_;    ///< Path to seed CSV (imported on first run)

    /**
     * @brief Parses a single CSV line into fields, handling quotes and commas.
     * @param line Raw line text.
     * @return Vector of parsed field strings.
     */
    static std::vector<std::string> parseCSVLine(const std::string& line);

    /**
     * @brief Writes string content to target path atomically via a .tmp file.
     * @param targetPath Final destination file path.
     * @param content Full content to write.
     * @param errorMsg Output error message on failure.
     * @return true if successful, false otherwise.
     */
    static bool atomicWrite(const std::string& targetPath,
                            const std::string& content,
                            std::string& errorMsg);

public:
    /**
     * @brief Constructor setting default paths.
     */
    Storage(std::string booksFile = "data/library_data.csv",
            std::string membersFile = "data/members.csv",
            std::string loansFile = "data/loans.csv",
            std::string seedFile = "data/library_seed.csv");

    // --- File System Checks ---
    static bool fileExists(const std::string& filepath);

    // --- Individual Entity Loaders (Public for granular unit testing) ---
    bool loadBooksCSV(const std::string& filepath,
                      std::vector<Book>& outBooks,
                      std::vector<std::string>& warnings,
                      std::string& errorMsg) const;

    bool saveBooksCSV(const std::string& filepath,
                      const std::vector<Book>& books,
                      std::string& errorMsg) const;

    bool loadMembersCSV(const std::string& filepath,
                        std::vector<Member>& outMembers,
                        std::vector<std::string>& warnings,
                        std::string& errorMsg) const;

    bool saveMembersCSV(const std::string& filepath,
                        const std::vector<Member>& members,
                        std::string& errorMsg) const;

    bool loadLoansCSV(const std::string& filepath,
                      std::vector<Loan>& outLoans,
                      std::vector<std::string>& warnings,
                      std::string& errorMsg) const;

    bool saveLoansCSV(const std::string& filepath,
                      const std::vector<Loan>& loans,
                      std::string& errorMsg) const;

    // --- High-Level Library Operations ---

    /**
     * @brief Loads the entire library state from disk.
     *        If runtime data file is missing, automatically imports seed CSV.
     * @param lib Target Library instance to populate.
     * @param warnings Accumulated warning messages.
     * @param errorMsg Critical error message if loading fails completely.
     * @return true on success (even if warnings occurred), false on critical failure.
     */
    bool load(Library& lib,
              std::vector<std::string>& warnings,
              std::string& errorMsg) const;

    /**
     * @brief Persists entire library state atomically to disk.
     * @param lib Library instance to save.
     * @param errorMsg Output error message on failure.
     * @return true if all files written successfully, false otherwise.
     */
    bool save(const Library& lib, std::string& errorMsg) const;

    // Getters for configured paths
    const std::string& getBooksFilePath() const { return booksFilePath_; }
    const std::string& getMembersFilePath() const { return membersFilePath_; }
    const std::string& getLoansFilePath() const { return loansFilePath_; }
    const std::string& getSeedFilePath() const { return seedFilePath_; }
};

#endif // STORAGE_H
