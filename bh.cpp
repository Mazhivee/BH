#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

using namespace std;

// Declare the oldTermios variable
struct termios oldTermios;

// Function to set terminal to non-canonical mode
void setNonCanonicalMode(int fd, struct termios &oldTermios) {
    struct termios newTermios;
    tcgetattr(fd, &oldTermios);          // Get current terminal attributes
    newTermios = oldTermios;
    newTermios.c_lflag &= ~(ICANON);    // Disable canonical mode (line buffering)
    newTermios.c_lflag &= ~(ECHO);      // Disable echoing of characters
    tcsetattr(fd, TCSANOW, &newTermios); // Apply new terminal attributes
}

// Function to reset terminal to previous mode
void resetTerminalMode(int fd, struct termios &oldTermios) {
    tcsetattr(fd, TCSANOW, &oldTermios);
}

void displayHelp() {
    cout << "Usage: bh [OPTION]" << endl;
    cout << "Display and execute commands from Bash history." << endl;
    cout << "Options:" << endl;
    cout << "  --help      Display this help message" << endl;
    cout << "  -u USERNAME Display the Bash history of a specific user" << endl;
    cout << endl;
    cout << "This program was made by Mazhive Productions with assistance from CHATGPT." << endl;
}

void customPager(const vector<string> &lines) {
    const int PAGE_SIZE = 20; // Number of lines per page
    int start = 0;
    int end = PAGE_SIZE;
    int numLines = lines.size();
    char input;

    while (end < numLines) {
        for (int i = start; i < end; ++i) {
            cout << i + 1 << ") " << lines[i] << endl;
        }

        cout << "Press 'q' to quit or any other key to continue: ";
        cin >> input;

        if (input == 'q' || input == 'Q') {
            break;
        }

        start = end;
        end += PAGE_SIZE;

        if (end > numLines) {
            end = numLines;
        }
    }

    // Display the remaining lines
    for (int i = start; i < numLines; ++i) {
        cout << i + 1 << ") " << lines[i] << endl;
    }
}

int main(int argc, char* argv[]) {
    string history_file;

    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        displayHelp();
        return 0;
    } else if (argc == 3 && strcmp(argv[1], "-u") == 0) {
        const char* username = argv[2];
        history_file = "/home/";
        history_file += username;
        history_file += "/.bash_history";
    } else {
        // Use the current user's home directory if no username is specified
        const char* home_dir = getenv("HOME");
        if (home_dir == nullptr) {
            cerr << "Error: Unable to determine home directory." << endl;
            return 1;
        }
        history_file = home_dir;
        history_file += "/.bash_history";
    }

    ifstream history(history_file);
    if (!history) {
        cerr << "Error: Unable to open history file." << endl;
        return 1;
    }

    vector<string> commands;
    string line;
    while (getline(history, line)) {
        // Skip empty lines and lines starting with '#'
        if (line.empty() || line[0] == '#') {
            continue;
        }
        commands.push_back(line);
    }

    if (commands.empty()) {
        cout << "No valid commands found in history." << endl;
        return 0;
    }

    // Check terminal window size
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int windowHeight = w.ws_row;

    if (windowHeight < static_cast<int>(commands.size())) {
        // If too many lines for the terminal window, use custom pager
        setNonCanonicalMode(STDIN_FILENO, oldTermios);
        customPager(commands);
        resetTerminalMode(STDIN_FILENO, oldTermios);
    } else {
        // Otherwise, print all lines with line numbers
        for (size_t i = 0; i < commands.size(); ++i) {
            cout << i + 1 << ") " << commands[i] << endl;
        }
    }

    cout << "Enter the number of the command you want to reuse: ";
    int command_number;
    cin >> command_number;

    if (command_number >= 1 && command_number <= static_cast<int>(commands.size())) {
        string selected_command = commands[command_number - 1];

        cout << "You selected the following command:" << endl;
        cout << selected_command << endl;

        cout << "Do you want to execute this command? (y/n): ";
        char confirm;
        cin >> confirm;

        if (confirm == 'y' || confirm == 'Y') {
            system(selected_command.c_str());
        } else {
            cout << "Command execution canceled." << endl;
        }
    } else {
        cerr << "Invalid input. Please enter a valid number." << endl;
        return 1;
    }

    return 0;
}
