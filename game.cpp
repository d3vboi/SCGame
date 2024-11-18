#include <cctype>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <random>
#include <algorithm>

// get a single char input
char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcgetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}

std::string toUpper(const std::string &input) {
    std::string result = input; // Copy the input string
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return result;
}

std::string getText() {
    // placeholder
    return "In the forest deep and green,\n"
           "Lies a world we've never seen.";
}

// Function to generate a lettermap
std::map<char, char> generateLetterMap() {
    std::vector<char> originalLetters;
    for (char c = 'A'; c <= 'Z'; ++c) {
        originalLetters.push_back(c);
    }

    std::vector<char> jumbledLetters = originalLetters;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(jumbledLetters.begin(), jumbledLetters.end(), g);

    // Create a map from original to jumbled letters
    std::map<char, char> letterMap;
    for (size_t i = 0; i < originalLetters.size(); ++i) {
        letterMap[originalLetters[i]] = jumbledLetters[i];
    }

    return letterMap;
}

// Function to apply the encryption map to a text string
std::string applyEncryption(const std::string &text, const std::map<char, char> &letterMap) {
    std::string encryptedText;

    for (const char &ch : toUpper(text)) {
        if (std::isalpha(ch)) {
            encryptedText += letterMap.at(ch); // Replace alphanumeric chars
        } else {
            encryptedText += ch; // Keep everything else untouched
        }
    }

    return encryptedText;
}

void printBoard(const std::string &text, int cursorCol = 0, int cursorRow = 0, const std::map<char, char> &replacements = {}) {
    // clear screen
    std::cout << "\033[2J\033[1;1H";

    std::string fgGray = "\033[90m";
    std::string fgWhiteBold = "\033[1;97m";
    std::string fgWhite = "\033[97m";
    std::string reset = "\033[0m";

    int currentRow = 0;
    int currentCol = 0;

    for (const char &ch : text) {
        if (ch == '\n') {
            if (currentRow == cursorRow && currentCol == cursorCol) {
                std::cout << fgWhite << ' ' << reset; // Highlight newline as space
            } else {
                std::cout << '\n';
            }
            currentRow++;
            currentCol = 0; // Reset column for new row
            continue;
        }

        char displayChar = (replacements.count(ch) > 0) ? replacements.at(ch) : ch;

        if (currentRow == cursorRow && currentCol == cursorCol) {
            std::cout << fgWhite << displayChar << reset; // Highlight current character
        } else if (replacements.count(ch) > 0) {
            std::cout << fgWhiteBold << displayChar << reset; // Replaced characters
        } else {
            std::cout << fgGray << displayChar << reset; // Default characters
        }

        currentCol++;
    }
    std::cout << std::endl;
}

// Function to handle arrow keys for navigation
void playVisual() {
    int cursorRow = 0;
    int cursorCol = 0;
    
    std::map<char, char> letterMap = generateLetterMap();
    std::string text = applyEncryption(getText(), letterMap);
    std::map<char, char> replacements;

    while (true) {
        printBoard(text, cursorCol, cursorRow, replacements);

        char input = getch();
        if (input == '\033') { // Escape character
            getch();             // Skip the '[' character
            char direction = getch();
            switch (direction) {
                case 'A': // Up arrow
                    cursorRow = std::max(0, cursorRow - 1);
                    break;
                case 'B': // Down arrow
                    cursorRow = std::min(cursorRow + 1, static_cast<int>(std::count(text.begin(), text.end(), '\n')));
                    break;
                case 'C': // Right arrow
                    cursorCol++;
                    break;
                case 'D': // Left arrow
                    cursorCol = std::max(0, cursorCol - 1);
                    break;
            }
        } else if (std::isalpha(input)) { // Letter input
            // Determine the character at the cursor position
            int currentRow = 0, currentCol = 0;
            char currentChar = '\0';
            for (const char &ch : text) {
                if (ch == '\n') {
                    currentRow++;
                    currentCol = 0;
                    if (currentRow > cursorRow) break;
                    continue;
                }

                if (currentRow == cursorRow && currentCol == cursorCol) {
                    currentChar = ch;
                    break;
                }

                currentCol++;
            }

            if (std::isalpha(currentChar)) {
                // Assign or reassign the replacement
                replacements[currentChar] = input;
            }
        } else if (input == 127) { // Backspace key (ASCII 127)
            // Determine the character at the cursor position
            int currentRow = 0, currentCol = 0;
            char currentChar = '\0';
            for (const char &ch : text) {
                if (ch == '\n') {
                    currentRow++;
                    currentCol = 0;
                    if (currentRow > cursorRow) break;
                    continue;
                }

                if (currentRow == cursorRow && currentCol == cursorCol) {
                    currentChar = ch;
                    break;
                }

                currentCol++;
            }

            if (std::isalpha(currentChar) && replacements.count(currentChar) > 0) {
                // Remove the replacement and revert to the original
                replacements.erase(currentChar);
            }
        }
    }
}


void helpMenu() {
    std::cout << "Usage: TODO " << std::endl;
    std::cout << "  --help: Display this menu" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--help") {
        helpMenu();
        return 0;
    }
    try {
        playVisual();
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
    }

    return 0;
}