#ifndef MOVIE_CATALOG_H
#define MOVIE_CATALOG_H

#include <string>

using namespace std;

// MovieCatalog stores the available movies using parallel arrays.
// This demonstrates array + count management.
class MovieCatalog {
public:
    // Maximum number of screenings that can be stored in the catalog.
    static const int MaxMovies = 50;

    MovieCatalog();

    void showMovieSummary() const;
    void showMovies() const;
    void showTodaysMovies() const;
    void showShowtimesForMovie(const string& movieTitle) const;
    bool addMovie(const string& movieTitle, const string& genre, const string& classification,
                  const string& language, const string& duration, const string& hallName, const string& showTime);
    bool updateMovie(int movieNumber, const string& movieTitle, const string& genre, const string& classification,
                     const string& language, const string& duration, const string& hallName, const string& showTime);
    bool removeMovie(int movieNumber);
    void clearMovies();
    bool isValidMovieNumber(int movieNumber) const;
    bool isValidUniqueMovieNumber(int movieNumber) const;
    bool screeningExists(const string& movieTitle, const string& hallName, const string& showTime) const;
    bool hallShowtimeExists(const string& hallName, const string& showTime) const;
    bool hallShowtimeTakenByOtherScreening(int movieNumber, const string& hallName, const string& showTime) const;
    string getUniqueMovieTitle(int movieNumber) const;
    int getScreeningNumberForMovieShowtime(const string& movieTitle, int showtimeNumber) const;
    string getMovieTitle(int movieNumber) const;
    string getGenre(int movieNumber) const;
    string getClassification(int movieNumber) const;
    string getLanguage(int movieNumber) const;
    string getDuration(int movieNumber) const;
    string getHallName(int movieNumber) const;
    string getShowTime(int movieNumber) const;
    bool findMovieDetailsByTitle(const string& movieTitle, string& genre, string& classification,
                                 string& language, string& hallName, string& showTime) const;
    int getMovieCount() const;

private:
    // Parallel arrays: the same index represents one movie's full information.
    string movies[MaxMovies];
    string genres[MaxMovies];
    string classifications[MaxMovies];
    string languages[MaxMovies];
    string durations[MaxMovies];
    string halls[MaxMovies];
    string showTimes[MaxMovies];

    // Current number of movies stored in the arrays.
    int movieCount;

    bool movieTitleAppearedBefore(int currentIndex) const;
};

#endif // MOVIE_CATALOG_H
