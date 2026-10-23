#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <memory>
using namespace std;

// Cyber Detective: C++ OOP concepts demonstration.
// This console program is separate from index.html; it is not connected to the website.

class Player {
private:
    string name;
    int score;
    int answered;
    int correct;

public:
    Player(string n = "Detective") : name(n), score(0), answered(0), correct(0) {
        if (name.empty()) throw invalid_argument("Player name cannot be empty.");
    }

    string getName() const { return name; }
    int getScore() const { return score; }
    int getAnswered() const { return answered; }
    int getCorrect() const { return correct; }

    void recordAnswer(bool isCorrect, int points = 10) {
        if (points < 0) throw invalid_argument("Points cannot be negative.");
        ++answered;
        if (isCorrect) {
            ++correct;
            score += points;
        }
    }

    // Function overloading: same function name, different parameter lists.
    void displayScore() const {
        cout << name << "'s score: " << score << " XP\n";
    }
    void displayScore(bool detailed) const {
        displayScore();
        if (detailed) {
            cout << "Correct answers: " << correct << "/" << answered << '\n';
        }
    }

    // Friend function can access private members of Player.
    friend void detectiveReport(const Player& p);
};

void detectiveReport(const Player& p) {
    cout << "\n[FRIEND FUNCTION REPORT]\n"
         << "Detective: " << p.name << "\n"
         << "Private score: " << p.score << "\n"
         << "Accuracy: "
         << (p.answered ? (100 * p.correct / p.answered) : 0) << "%\n";
}

// Function template: reusable for different numeric types.
template <typename T>
T bonusPoints(T base, T bonus) {
    return base + bonus;
}

class Question {
    string prompt;
    vector<string> options;
    int answerIndex;
    string explanation;

public:
    Question(string p, vector<string> o, int a, string e)
        : prompt(p), options(o), answerIndex(a), explanation(e) {
        if (options.empty() || a < 0 || a >= static_cast<int>(options.size()))
            throw invalid_argument("Invalid question configuration.");
    }

    bool ask() const {
        cout << "\n" << prompt << "\n";
        for (size_t i = 0; i < options.size(); ++i)
            cout << "  " << i + 1 << ". " << options[i] << '\n';

        int choice;
        cout << "Your answer (1-" << options.size() << "): ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw runtime_error("Please enter a number.");
        }
        if (choice < 1 || choice > static_cast<int>(options.size()))
            throw out_of_range("Choice is outside the available options.");

        bool correct = (choice - 1 == answerIndex);
        cout << (correct ? "Correct deduction! " : "Not quite. ");
        cout << explanation << '\n';
        return correct;
    }
};

// Abstract base class demonstrates runtime polymorphism.
class Mission {
protected:
    string title;
    vector<Question> questions;

public:
    Mission(string t, vector<Question> q) : title(t), questions(q) {}
    virtual ~Mission() = default;
    virtual string category() const = 0;

    virtual int run(Player& player) {
        cout << "\n=== " << title << " [" << category() << "] ===\n";
        int pointsEarned = 0;
        for (const Question& q : questions) {
            try {
                bool correct = q.ask();
                player.recordAnswer(correct, correct ? 10 : 0);
                if (correct) pointsEarned += 10;
            } catch (const exception& ex) {
                cout << "Input error: " << ex.what()
                     << " (this question earns no points).\n";
                player.recordAnswer(false, 0);
            }
        }
        return pointsEarned;
    }
};

class PhishingMission : public Mission {
public:
    using Mission::Mission;
    string category() const override { return "Email forensics"; }
};

class LinkMission : public Mission {
public:
    using Mission::Mission;
    string category() const override { return "URL analysis"; }
};

class PasswordMission : public Mission {
public:
    using Mission::Mission;
    string category() const override { return "Credential defense"; }
};

class ScamMission : public Mission {
public:
    using Mission::Mission;
    string category() const override { return "Fraud investigation"; }
};

class ScoreManager {
private:
    map<string, int> leaderboard;

public:
    void update(const Player& player) {
        leaderboard[player.getName()] = player.getScore();
    }

    void saveToFile(const string& filename) const {
        ofstream file(filename);
        if (!file) throw runtime_error("Could not open score file for writing.");
        for (const auto& entry : leaderboard)
            file << entry.first << '\t' << entry.second << '\n';
        if (!file) throw runtime_error("Failed while writing score file.");
    }

    void showLeaderboard() const {
        cout << "\n=== SCOREBOARD ===\n";
        if (leaderboard.empty()) cout << "No scores recorded yet.\n";
        for (const auto& entry : leaderboard)
            cout << entry.first << " : " << entry.second << " XP\n";
    }
};

// Procedural approach: a standalone function performing a task.
void showCyberSafetyTip() {
    cout << "\n[PROCEDURAL FUNCTION] Tip: Never share an OTP or password.\n";
}

int main() {
    try {
        cout << "====================================\n"
             << "       CYBER DETECTIVE (C++)\n"
             << "====================================\n";
        showCyberSafetyTip();

        string name;
        cout << "\nEnter your detective codename: ";
        getline(cin, name);
        if (name.empty()) name = "Detective";

        Player player(name); // Object created from Player class.

        vector<unique_ptr<Mission>> missions;
        // Mission questions are stored using STL vector.
        missions.push_back(make_unique<PhishingMission>(
            "Phishing Protocol", vector<Question>{
                Question("An email demands your password urgently. What is safest?",
                    {"Click the link", "Verify through the official website", "Reply with password"},
                    1, "Urgency and password requests are phishing warning signs."),
                Question("An unexpected attachment asks you to enable macros.",
                    {"Enable them", "Open it on your phone", "Do not open; verify independently"},
                    2, "Unexpected attachments may contain malware.")
            }));
        missions.push_back(make_unique<LinkMission>(
            "Link Interrogation", vector<Question>{
                Question("HTTPS means a website is definitely legitimate.",
                    {"True", "False"},
                    1, "HTTPS encrypts the connection but does not prove the site is genuine."),
                Question("A parcel text contains an unexpected shortened link.",
                    {"Open it", "Use the courier's official app/site directly", "Share it"},
                    1, "Navigate independently to the official service.")
            }));
        missions.push_back(make_unique<PasswordMission>(
            "Password Fortress", vector<Question>{
                Question("Which is the best password practice?",
                    {"Reuse one password", "Use a long unique password with a manager", "Use your birthday"},
                    1, "Long, unique passwords and password managers improve security."),
                Question("What should you do after a password reuse breach?",
                    {"Ignore it", "Change reused passwords and enable MFA", "Post the password"},
                    1, "Change reused credentials and enable multifactor authentication.")
            }));
        missions.push_back(make_unique<ScamMission>(
            "Scam Surveillance", vector<Question>{
                Question("A prize requires an upfront fee and OTP.",
                    {"Pay the fee", "Share the OTP", "Do not pay/share; verify independently"},
                    2, "Unexpected fees and OTP requests are scam warning signs."),
                Question("A supposed manager asks for gift card codes urgently.",
                    {"Buy them now", "Verify using a known separate channel", "Send bank details"},
                    1, "Verify unusual requests through a trusted channel.")
            }));

        ScoreManager scores;
        cout << "\nChoose a mission:\n";
        for (size_t i = 0; i < missions.size(); ++i)
            cout << i + 1 << ". " << missions[i]->category() << '\n';
        cout << "0. Exit\n";

        while (true) {
            int choice;
            cout << "\nMission number: ";
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Please enter a number.\n";
                continue;
            }
            if (choice == 0) break;
            if (choice < 1 || choice > static_cast<int>(missions.size())) {
                cout << "Choose a valid mission number.\n";
                continue;
            }

            // Polymorphism: base-class pointer invokes derived mission behavior.
            missions[choice - 1]->run(player);
            scores.update(player);
            player.displayScore();          // Overload 1
            player.displayScore(true);      // Overload 2
            detectiveReport(player);        // Friend function
            cout << "Template bonus example: "
                 << bonusPoints<int>(10, 5) << " points\n";

            try {
                scores.saveToFile("cyber_scores.txt"); // File handling
                cout << "Score saved to cyber_scores.txt\n";
            } catch (const exception& ex) {
                cerr << "File error: " << ex.what() << '\n';
            }
            scores.showLeaderboard();
        }

        cout << "\nFinal report:\n";
        player.displayScore(true);
        cout << "Keep investigating safely. Goodbye, detective!\n";
    } catch (const exception& ex) {
        cerr << "Application error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
