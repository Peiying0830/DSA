#include <iostream>
#include <string>
#include <cstdlib> // produce the random number
#include <ctime>   // produce the random number
using namespace std;

// Class for the Number Guessing Game
class GuessingGame {
private:
    string playerName;
    int secretNumbers[3]; // Store 3 random secret numbers
    int score;            // Store player's score

public:
    // Default constructor
    GuessingGame() {
        playerName = "Player";

        // Generate random secret numbers for each level
        secretNumbers[0] = rand() % 10 + 1;   // Easy: 1-10
        secretNumbers[1] = rand() % 50 + 1;   // Medium: 1-50
        secretNumbers[2] = rand() % 100 + 1;  // Hard: 1-100

        score = 0;
    }

    // Parameterized constructor
    GuessingGame(string name) {
        playerName = name;

        // Generate random secret numbers for each level
        secretNumbers[0] = rand() % 10 + 1;   // Easy: 1-10
        secretNumbers[1] = rand() % 50 + 1;   // Medium: 1-50
        secretNumbers[2] = rand() % 100 + 1;  // Hard: 1-100

        score = 0;
    }

    // Function to display the game menu
    void displayMenu() {
        cout << "\n========== NUMBER GUESSING GAME ==========" << endl;
        cout << "1. Easy" << endl;
        cout << "2. Medium" << endl;
        cout << "3. Hard" << endl;
        cout << "4. Display Score" << endl;
        cout << "5. Exit" << endl;
        cout << "Enter your choice: ";
    }

    // Function to start the guessing game
    void startGame(int level, int limit, int maxAttempts) {
        int guess;
        bool correct = false;

        // Generate a new random number when the game starts
        secretNumbers[level] = rand() % limit + 1;

        // Display the number range and maximum attempts
        cout << "Guess a number between 1 and " << limit << "." << endl;
        cout << "You have " << maxAttempts << " attempts." << endl;

        // For loop controls the maximum number of attempts
        for (int attempt = 1; attempt <= maxAttempts; attempt++) {

            cout << "\nAttempt " << attempt << "/" << maxAttempts << endl;
            cout << "Enter your guess: ";
            cin >> guess;

            // While loop checks if the guess is within the number limit
            while (guess < 1 || guess > limit) {
                cout << "Invalid number!" << endl;
                cout << "Please enter a number between 1 and "
                     << limit << ": ";
                cin >> guess;
            }

            // Check if the player guessed the correct number
            if (guess == secretNumbers[level]) {

                cout << "\nCongratulations, " << playerName << "!" << endl;
                cout << "You guessed the correct number successfully!" << endl;

                // Add 10 points for a correct guess
                score += 10;

                correct = true;

                // Stop the loop when the answer is correct
                break;
            }

            // Give a hint if the guess is too low
            else if (guess < secretNumbers[level]) {
                cout << "Hint: Too Low! Try a higher number." << endl;
            }

            // Give a hint if the guess is too high
            else {
                cout << "Hint: Too High! Try a lower number." << endl;
            }

            // Display the number of attempts remaining
            cout << "Attempts remaining: "
                 << maxAttempts - attempt << endl;
        }

        // Display the correct number if all attempts are used
        if (correct == false) {
            cout << "\nMaximum attempts reached!" << endl;
            cout << "The correct number was: "
                 << secretNumbers[level] << endl;
        }
    }

    // Function to display the player's score
    void displayScore() {
        cout << "\n========== PLAYER SCORE ==========" << endl;
        cout << "Player Name: " << playerName << endl;
        cout << "Score: " << score << endl;
    }
};

int main() {
    string name;
    int choice;

    // Initialize the random number generator
    // This helps generate different random numbers
    srand(time(0));

    // Ask the player to enter their name
    cout << "Enter your name: ";
    getline(cin, name);

    // Create a game object using the parameterized constructor
    GuessingGame game(name);

    // While loop keeps the menu running until the player exits
    while (true) {

        game.displayMenu();
        cin >> choice;

        // Switch-case handles the menu selection
        switch (choice) {

        case 1:
            cout << "\nYou selected EASY level." << endl;

            // Easy: array index 0, range 1-10, maximum 5 attempts
            game.startGame(0, 10, 5);
            break;

        case 2:
            cout << "\nYou selected MEDIUM level." << endl;

            // Medium: array index 1, range 1-50, maximum 7 attempts
            game.startGame(1, 50, 7);
            break;

        case 3:
            cout << "\nYou selected HARD level." << endl;

            // Hard: array index 2, range 1-100, maximum 10 attempts
            game.startGame(2, 100, 10);
            break;

        case 4:
            // Display the player's current score
            game.displayScore();
            break;

        case 5:
            // Exit the program
            cout << "\nThank you for playing, "
                 << name << "!" << endl;
            return 0;

        default:
            // Display error message for invalid menu choice
            cout << "\nInvalid choice!" << endl;
            cout << "Please enter a number from 1 to 5." << endl;
        }
    }

    return 0;
}