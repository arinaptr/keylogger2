#include <iostream>
#include <fstream>
#include <windows.h>
#include <unordered_map>
#include <regex>

#define FILE_NAME "###\\keylogger.html"
#define DECRYPTED_FILE_NAME "decrypted_keylogger.html"
#define ENCRYPTION_KEY 0x5A
using namespace std;

string xorDecrypt(const string& data) {
    string decryptedData = data;
    for (size_t i = 0; i < decryptedData.length(); i++) {
        decryptedData[i] ^= ENCRYPTION_KEY;
    }
    return decryptedData;
}

string decryptDate(const string& encryptedDate) {
    return xorDecrypt(encryptedDate);
}

int main() {
    ifstream read(FILE_NAME);
    if (!read) {
        cerr << "Error opening the file." << endl;
        return 1;
    }

    ofstream write(DECRYPTED_FILE_NAME);
    if (!write) {
        cerr << "Error opening the decrypted file." << endl;
        return 1;
    }

    string line;
    while (getline(read, line)) {
        regex regexPattern("<h2>(.+)</h2>");
        smatch matches;
        if (regex_search(line, matches, regexPattern)) {
            string encryptedDate = matches[1].str();
            string decryptedDate = decryptDate(encryptedDate);
            cout << "Date: " << decryptedDate << endl;
            write << "<h2>Date: " << decryptedDate << "</h2>\n";
        }
        else {
            regexPattern = regex("<h3>(.+)</h3>");
            if (regex_search(line, matches, regexPattern)) {
                string encryptedActiveWindow = matches[1].str();
                string decryptedActiveWindow = xorDecrypt(encryptedActiveWindow);
                cout << "Active Window: " << decryptedActiveWindow << endl;
                write << "<h3>Active Window: " << decryptedActiveWindow << "</h3>\n";
            }
            else {
                regexPattern = regex("<p>(.+)</p>");
                if (regex_search(line, matches, regexPattern)) {
                    string encryptedCharacters = matches[1].str();
                    string decryptedCharacters = xorDecrypt(encryptedCharacters);
                    cout << "Characters: " << decryptedCharacters << endl;
                    write << "<p>Characters: " << decryptedCharacters << "</p>\n";
                }
            }
        }
    }

    read.close();
    write.close();

    cout << "Decryption complete. Decrypted data saved to 'decrypted_keylogger.html'." << endl;

    return 0;
}
