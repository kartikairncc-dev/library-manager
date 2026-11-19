# Test Suite Documentation: C++17 Object-Oriented Library Manager

This document provides a detailed overview of the test cases implemented in `tests/test_library.cpp` and manual verification scenarios covering edge cases, defensive validation, and persistence safety.

---

## Automated Test Suites (`tests/test_library.cpp`)

| Suite # | Test Scenario | Description | Expected Result | Status |
| :--- | :--- | :--- | :--- | :--- |
| **Test 1** | **Empty Collection** | Querying, searching, issuing, and returning against an empty `Library` instance. | Returns empty collections or `nullptr` without crashing; operations return `false` with descriptive error messages. | **PASS** |
| **Test 2** | **Duplicate IDs** | Adding books or members with identical IDs (`B001`, `M001`). | First instance is accepted; subsequent duplicate is rejected with `"already exists"`. First record remains intact. | **PASS** |
| **Test 3** | **Missing File** | Loading when runtime CSV files and seed files are nonexistent. | Storage returns graceful failure or starts fresh; library remains valid empty collection without unhandled exceptions. | **PASS** |
| **Test 4** | **Malformed CSV Rows** | Reading a CSV file containing rows with missing columns, invalid non-numeric year, negative year, empty IDs, or invalid statuses. | Malformed rows are skipped; descriptive warning accumulated for each; all valid rows are successfully parsed and loaded. | **PASS** |
| **Test 5** | **Boundary Dates** | Calendar validation including leap years (`2024-02-29`, `2000-02-29` vs `2023-02-29`, `1900-02-29`), month ends (`04-30` vs `04-31`), invalid months (`00`, `13`), and invalid delimiters. | Validates true Gregorian dates; rejects non-leap leap days, nonexistent 31st days, and malformed strings. | **PASS** |
| **Test 6** | **Issuing Unavailable Book** | Attempting to issue a book that is already marked as borrowed. | Fails with message indicating book is already borrowed; second member's borrow count is not incremented. | **PASS** |
| **Test 7** | **Exceeding Borrow Limit** | Borrowing books up to limit (3), then attempting to borrow a 4th book. | First 3 borrows succeed; 4th borrow is rejected with borrow limit message. Book remains available. | **PASS** |
| **Test 8** | **Returning Book Not Issued** | Attempting to return an available book or an already-returned book. | Fails with `"No active loan found"`. Returning nonexistent book fails with `"Book does not exist"`. | **PASS** |
| **Test 9** | **Save & Reload Roundtrip** | Populating books, members, active loans, saving to disk with atomic write, then reloading into a fresh `Library` instance. | 100% of state preserved: IDs, titles, categories, years, member borrow counts, active loan records, and due dates match. | **PASS** |

---

## Defensive & Persistence Verification Checklist

- [x] **Atomic File Replacement**: File write uses temporary `.tmp` file, flushes/closes stream via RAII, removes old file, and renames.
- [x] **Seed CSV Import on First Run**: If `data/library_data.csv` does not exist, `data/library_seed.csv` is imported automatically.
- [x] **Safe Terminal Input**: `std::getline` with whitespace trimming prevents buffer overflow and newline residue.
- [x] **Graceful EOF Handling**: Piped input or `Ctrl+D` / `Ctrl+Z` terminates cleanly with status code `0`.
- [x] **No Raw Pointers / No Memory Leaks**: Uses C++ standard library containers (`std::vector`, `std::string`) and RAII semantics.
