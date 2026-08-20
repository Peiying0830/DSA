#include "MovieCatalog.h"
#include "Ticket.h"

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

const int CatalogMenuWidth = 100;

void printWrappedCatalogLine(const string& firstPrefix, const string& nextPrefix, const string& text);

// MovieCatalog.cpp manages movie screenings using parallel arrays.
// One array index represents one screening: movie title, hall, showtime, and details.

// Print catalog headings using the same 100-character style as the main menus.
void printCatalogSeparator() {
    cout << string(CatalogMenuWidth, '-') << endl;
}

void printCatalogTitle(const string& title) {
    int leftPadding = (CatalogMenuWidth - static_cast<int>(title.length())) / 2;
    if (leftPadding < 0) {
        leftPadding = 0;
    }
    cout << string(leftPadding, ' ') << title << endl;
}

string shortenCatalogText(const string& text, int width) {
    // Shorten long text so table columns stay within the console width.
    if (width <= 0) {
        return "";
    }
    if (static_cast<int>(text.length()) <= width) {
        return text;
    }
    if (width <= 3) {
        return text.substr(0, width);
    }
    return text.substr(0, width - 3) + "...";
}

string takeCatalogTextChunk(const string& text, int width, size_t& position) {
    // Take part of a long title without cutting the whole table layout.
    while (position < text.length() && text[position] == ' ') {
        position++;
    }
    if (position >= text.length()) {
        return "";
    }

    size_t remainingLength = text.length() - position;
    if (static_cast<int>(remainingLength) <= width) {
        string chunk = text.substr(position);
        position = text.length();
        return chunk;
    }

    size_t breakPosition = text.rfind(' ', position + width);
    if (breakPosition == string::npos || breakPosition <= position) {
        breakPosition = position + width;
    }

    string chunk = text.substr(position, breakPosition - position);
    position = breakPosition;
    return chunk;
}

void printMovieTableHeader(bool includeTimeSlots) {
    // Staff view includes time slots, while customer movie view hides hall allocation.
    if (includeTimeSlots) {
        cout << left << setw(5) << "No."
             << setw(31) << "Movie Title"
             << setw(13) << "Genre"
             << setw(8) << "Class"
             << setw(14) << "Language"
             << setw(10) << "Duration"
             << setw(19) << "Time Slots" << endl;
    } else {
        cout << left << setw(5) << "No."
             << setw(40) << "Movie Title"
             << setw(14) << "Genre"
             << setw(8) << "Class"
             << setw(15) << "Language"
             << setw(10) << "Duration" << endl;
    }
    printCatalogSeparator();
}

void printMovieTableRow(int displayNumber, const string& movieTitle, const string& genre,
                        const string& classification, const string& language,
                        const string& duration, const string& firstTimeSlot,
                        bool includeTimeSlots) {
    // One printed movie row may wrap if the movie title is long.
    if (includeTimeSlots) {
        const int titleWidth = 31;
        size_t titlePosition = 0;
        string titleLine = takeCatalogTextChunk(movieTitle, titleWidth - 1, titlePosition);
        cout << left << setw(5) << displayNumber
             << setw(titleWidth) << titleLine
             << setw(13) << shortenCatalogText(genre, 12)
             << setw(8) << shortenCatalogText(classification, 7)
             << setw(14) << shortenCatalogText(language, 13)
             << setw(10) << shortenCatalogText(duration, 9)
             << setw(19) << shortenCatalogText(firstTimeSlot, 18) << endl;
        while (titlePosition < movieTitle.length()) {
            titleLine = takeCatalogTextChunk(movieTitle, titleWidth - 1, titlePosition);
            cout << left << setw(5) << ""
                 << setw(titleWidth) << titleLine << endl;
        }
    } else {
        const int titleWidth = 40;
        size_t titlePosition = 0;
        string titleLine = takeCatalogTextChunk(movieTitle, titleWidth, titlePosition);
        cout << left << setw(5) << displayNumber
             << setw(titleWidth) << titleLine
             << setw(14) << shortenCatalogText(genre, 13)
             << setw(8) << shortenCatalogText(classification, 7)
             << setw(15) << shortenCatalogText(language, 14)
             << setw(10) << shortenCatalogText(duration, 9) << endl;
        while (titlePosition < movieTitle.length()) {
            titleLine = takeCatalogTextChunk(movieTitle, titleWidth - 1, titlePosition);
            cout << left << setw(5) << ""
                 << setw(titleWidth) << titleLine << endl;
        }
    }
}

// Wrap long catalog lines so text does not run past the 60-character menu width.
void printWrappedCatalogLine(const string& firstPrefix, const string& nextPrefix, const string& text) {
    string remainingText = text;
    string currentPrefix = firstPrefix;

    while (!remainingText.empty()) {
        int availableWidth = CatalogMenuWidth - static_cast<int>(currentPrefix.length());
        if (availableWidth < 10) {
            availableWidth = 10;
        }

        if (static_cast<int>(remainingText.length()) <= availableWidth) {
            cout << currentPrefix << remainingText << endl;
            return;
        }

        size_t breakPosition = remainingText.rfind(' ', availableWidth);
        if (breakPosition == string::npos || breakPosition == 0) {
            breakPosition = availableWidth;
        }

        cout << currentPrefix << remainingText.substr(0, breakPosition) << endl;
        while (breakPosition < remainingText.length()
               && remainingText[breakPosition] == ' ') {
            breakPosition++;
        }
        remainingText = remainingText.substr(breakPosition);
        currentPrefix = nextPrefix;
    }
}

// Wrap screening slots without splitting one "Hall X (HH:MM)" item across lines.
void printWrappedTimeSlot(const string& prefix, const string& nextPrefix, const string& slotText,
                          bool& lineStarted, int& currentLineLength) {
    if (!lineStarted) {
        cout << prefix << slotText;
        lineStarted = true;
        currentLineLength = static_cast<int>(prefix.length() + slotText.length());
        return;
    }

    string itemText = ", " + slotText;
    if (currentLineLength + static_cast<int>(itemText.length()) > CatalogMenuWidth) {
        cout << "," << endl;
        cout << nextPrefix << slotText;
        currentLineLength = static_cast<int>(nextPrefix.length() + slotText.length());
    } else {
        cout << itemText;
        currentLineLength += static_cast<int>(itemText.length());
    }
}

// Constructor loads default movie data when the system starts.
MovieCatalog::MovieCatalog()
    : movieCount(0) {
    // No default hardcoded movies are loaded here.
    // Staff/Admin inserts screenings, or the system loads them from movies_autosave.txt.
}

// Staff summary view: group repeated screenings under the same movie title.
void MovieCatalog::showMovieSummary() const {
    // This groups screenings by movie title so the same movie is not repeated many times.
    cout << endl;
    printCatalogSeparator();
    printCatalogTitle("Movie List");
    printCatalogSeparator();
    if (movieCount == 0) {
        cout << "No movies available." << endl;
        return;
    }

    printMovieTableHeader(true);
    int displayNumber = 1;
    for (int index = 0; index < movieCount; ++index) {
        if (movieTitleAppearedBefore(index)) {
            continue;
        }

        string firstTimeSlot;
        // Find the first time slot for this movie to print on the main row.
        for (int slotIndex = 0; slotIndex < movieCount; ++slotIndex) {
            if (toLowerText(movies[slotIndex]) == toLowerText(movies[index])) {
                firstTimeSlot = halls[slotIndex] + " (" + showTimes[slotIndex] + ")";
                break;
            }
        }

        printMovieTableRow(displayNumber, movies[index], genres[index], classifications[index],
                           languages[index], durations[index], firstTimeSlot, true);
        bool skippedFirstTimeSlot = false;
        // Print the remaining time slots below the first row.
        for (int slotIndex = 0; slotIndex < movieCount; ++slotIndex) {
            if (toLowerText(movies[slotIndex]) == toLowerText(movies[index])) {
                if (!skippedFirstTimeSlot) {
                    skippedFirstTimeSlot = true;
                    continue;
                }
                cout << left << setw(81) << ""
                     << halls[slotIndex] << " (" << showTimes[slotIndex] << ")" << endl;
            }
        }
        cout << endl;
        displayNumber++;
    }
    printCatalogSeparator();
}

// Staff edit/delete view: display every screening separately with its internal number.
void MovieCatalog::showMovies() const {
    // This view shows every screening separately because edit/delete needs the real array index.
    cout << endl;
    printCatalogSeparator();
    printCatalogTitle("Screening List");
    printCatalogSeparator();
    if (movieCount == 0) {
        cout << "No movies available." << endl;
        return;
    }

    for (int index = 0; index < movieCount; ++index) {
        cout << index + 1 << ". " << movies[index] << " | " << showTimes[index]
             << " | " << halls[index] << endl;
        cout << "   Genre: " << genres[index]
             << " | Classification: " << classifications[index]
             << " | Language: " << languages[index]
             << " | Duration: " << durations[index] << endl;
    }
}

// Check whether the movie title already appeared earlier in the screening array.
bool MovieCatalog::movieTitleAppearedBefore(int currentIndex) const {
    // Used by grouped movie lists to avoid printing duplicate titles.
    for (int index = 0; index < currentIndex; ++index) {
        if (toLowerText(movies[index]) == toLowerText(movies[currentIndex])) {
            return true;
        }
    }
    return false;
}

// Customer view: display today's unique movies with details, but not hall allocation.
void MovieCatalog::showTodaysMovies() const {
    // Customer sees the movie list first, then selects a movie to view showtimes.
    cout << endl;
    printCatalogSeparator();
    printCatalogTitle("Today's Movies");
    printCatalogSeparator();
    if (movieCount == 0) {
        cout << "No movies available today." << endl;
        return;
    }

    printMovieTableHeader(false);
    int displayNumber = 1;
    for (int index = 0; index < movieCount; ++index) {
        if (movieTitleAppearedBefore(index)) {
            continue;
        }
        printMovieTableRow(displayNumber, movies[index], genres[index], classifications[index],
                           languages[index], durations[index], "", false);
        displayNumber++;
    }
    printCatalogSeparator();
}

// Display only the showtimes for one selected movie.
void MovieCatalog::showShowtimesForMovie(const string& movieTitle) const {
    // The showtime number displayed here is converted back to a real screening number later.
    cout << endl;
    printCatalogSeparator();
    printCatalogTitle("Showtimes");
    printCatalogSeparator();
    printCatalogTitle(movieTitle);
    int displayNumber = 1;
    for (int index = 0; index < movieCount; ++index) {
        if (toLowerText(movies[index]) == toLowerText(movieTitle)) {
            cout << displayNumber << ". " << showTimes[index]
                 << " (" << durations[index] << ")" << endl;
            displayNumber++;
        }
    }
    if (displayNumber == 1) {
        cout << "No showtimes available for this movie." << endl;
    }
}

// Add a new screening into the catalog array if there is space and no duplicate screening.
bool MovieCatalog::addMovie(const string& movieTitle, const string& genre, const string& classification,
                            const string& language, const string& duration, const string& hallName, const string& showTime) {
    // Array capacity check for the movie catalog.
    if (movieCount >= MaxMovies) {
        cout << "Cannot add movie: movie catalog is full." << endl;
        return false;
    }
    if (hallShowtimeExists(hallName, showTime)) {
        // A hall cannot show two different movies at exactly the same time.
        cout << "Cannot add movie: " << hallName << " already has a screening at "
             << showTime << "." << endl;
        return false;
    }
    if (screeningExists(movieTitle, hallName, showTime)) {
        cout << "Cannot add movie: same movie, hall, and showtime already exists." << endl;
        return false;
    }

    // Store all details at the same index across all parallel arrays.
    movies[movieCount] = movieTitle;
    genres[movieCount] = genre;
    classifications[movieCount] = classification;
    languages[movieCount] = language;
    durations[movieCount] = duration;
    halls[movieCount] = hallName;
    showTimes[movieCount] = showTime;
    movieCount++;
    return true;
}

// Update one screening without changing the array position.
bool MovieCatalog::updateMovie(int movieNumber, const string& movieTitle, const string& genre, const string& classification,
                               const string& language, const string& duration, const string& hallName, const string& showTime) {
    // movieNumber is 1-based for the user, but array index is 0-based.
    if (!isValidMovieNumber(movieNumber)) {
        cout << "Invalid movie number. Please select from the movie list." << endl;
        return false;
    }

    if (hallShowtimeTakenByOtherScreening(movieNumber, hallName, showTime)) {
        cout << "Cannot update movie: " << hallName << " already has a screening at "
             << showTime << "." << endl;
        return false;
    }

    int updateIndex = movieNumber - 1;
    // Update every parallel array at the same index to keep the screening record consistent.
    movies[updateIndex] = movieTitle;
    genres[updateIndex] = genre;
    classifications[updateIndex] = classification;
    languages[updateIndex] = language;
    durations[updateIndex] = duration;
    halls[updateIndex] = hallName;
    showTimes[updateIndex] = showTime;
    return true;
}

// Delete a movie by shifting later array elements one position to the left.
bool MovieCatalog::removeMovie(int movieNumber) {
    // Deleting from an array requires shifting later items left.
    if (!isValidMovieNumber(movieNumber)) {
        cout << "Invalid movie number. Please select from the movie list." << endl;
        return false;
    }

    int removeIndex = movieNumber - 1;
    for (int index = removeIndex; index < movieCount - 1; ++index) {
        movies[index] = movies[index + 1];
        genres[index] = genres[index + 1];
        classifications[index] = classifications[index + 1];
        languages[index] = languages[index + 1];
        durations[index] = durations[index + 1];
        halls[index] = halls[index + 1];
        showTimes[index] = showTimes[index + 1];
    }
    movieCount--;
    return true;
}

// Reset the catalog before loading movie data from a text file.
void MovieCatalog::clearMovies() {
    // The old data can be ignored by resetting the count to 0.
    movieCount = 0;
}

// Check whether a selected movie number is inside the valid range.
bool MovieCatalog::isValidMovieNumber(int movieNumber) const {
    // Valid screening numbers are from 1 to movieCount.
    return movieNumber >= 1 && movieNumber <= movieCount;
}

// Check whether a unique movie number is valid in the customer movie list.
bool MovieCatalog::isValidUniqueMovieNumber(int movieNumber) const {
    // Validates the grouped customer list number, not the raw screening number.
    return isValidText(getUniqueMovieTitle(movieNumber));
}

// Case-insensitive duplicate screening check.
bool MovieCatalog::screeningExists(const string& movieTitle, const string& hallName, const string& showTime) const {
    for (int index = 0; index < movieCount; ++index) {
        if (toLowerText(movies[index]) == toLowerText(movieTitle)
            && toLowerText(halls[index]) == toLowerText(hallName)
            && toLowerText(showTimes[index]) == toLowerText(showTime)) {
            return true;
        }
    }
    return false;
}

// Check whether a hall is already occupied at the selected showtime.
bool MovieCatalog::hallShowtimeExists(const string& hallName, const string& showTime) const {
    for (int index = 0; index < movieCount; ++index) {
        if (toLowerText(halls[index]) == toLowerText(hallName)
            && toLowerText(showTimes[index]) == toLowerText(showTime)) {
            return true;
        }
    }
    return false;
}

// Edit validation: ignore the current screening but check all other screenings.
bool MovieCatalog::hallShowtimeTakenByOtherScreening(int movieNumber, const string& hallName, const string& showTime) const {
    int currentIndex = movieNumber - 1;
    for (int index = 0; index < movieCount; ++index) {
        if (index != currentIndex
            && toLowerText(halls[index]) == toLowerText(hallName)
            && toLowerText(showTimes[index]) == toLowerText(showTime)) {
            return true;
        }
    }
    return false;
}

// Return the movie title from the unique customer movie list.
string MovieCatalog::getUniqueMovieTitle(int movieNumber) const {
    // Convert customer movie number into the movie title from the grouped list.
    if (movieNumber <= 0) {
        return "";
    }

    int displayNumber = 0;
    for (int index = 0; index < movieCount; ++index) {
        if (movieTitleAppearedBefore(index)) {
            continue;
        }
        displayNumber++;
        if (displayNumber == movieNumber) {
            return movies[index];
        }
    }
    return "";
}

// Convert a selected showtime number for one movie into the real screening number.
int MovieCatalog::getScreeningNumberForMovieShowtime(const string& movieTitle, int showtimeNumber) const {
    // Convert selected showtime number into the actual internal screening index.
    if (showtimeNumber <= 0) {
        return 0;
    }

    int displayNumber = 0;
    for (int index = 0; index < movieCount; ++index) {
        if (toLowerText(movies[index]) == toLowerText(movieTitle)) {
            displayNumber++;
            if (displayNumber == showtimeNumber) {
                return index + 1;
            }
        }
    }
    return 0;
}

string MovieCatalog::getMovieTitle(int movieNumber) const {
    if (!isValidMovieNumber(movieNumber)) {
        return "";
    }
    return movies[movieNumber - 1];
}

string MovieCatalog::getGenre(int movieNumber) const {
    if (!isValidMovieNumber(movieNumber)) {
        return "";
    }
    return genres[movieNumber - 1];
}

string MovieCatalog::getClassification(int movieNumber) const {
    if (!isValidMovieNumber(movieNumber)) {
        return "";
    }
    return classifications[movieNumber - 1];
}

string MovieCatalog::getLanguage(int movieNumber) const {
    if (!isValidMovieNumber(movieNumber)) {
        return "";
    }
    return languages[movieNumber - 1];
}

string MovieCatalog::getDuration(int movieNumber) const {
    if (!isValidMovieNumber(movieNumber)) {
        return "";
    }
    return durations[movieNumber - 1];
}

string MovieCatalog::getHallName(int movieNumber) const {
    if (!isValidMovieNumber(movieNumber)) {
        return "";
    }
    return halls[movieNumber - 1];
}

string MovieCatalog::getShowTime(int movieNumber) const {
    if (!isValidMovieNumber(movieNumber)) {
        return "";
    }
    return showTimes[movieNumber - 1];
}

bool MovieCatalog::findMovieDetailsByTitle(const string& movieTitle, string& genre, string& classification,
                                           string& language, string& hallName, string& showTime) const {
    for (int index = 0; index < movieCount; ++index) {
        if (toLowerText(movies[index]) == toLowerText(movieTitle)) {
            genre = genres[index];
            classification = classifications[index];
            language = languages[index];
            hallName = halls[index];
            showTime = showTimes[index];
            return true;
        }
    }
    return false;
}

int MovieCatalog::getMovieCount() const {
    return movieCount;
}
