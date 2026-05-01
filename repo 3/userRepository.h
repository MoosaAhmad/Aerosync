#pragma once
#include <vector>
#include <fstream>
#include <string>
#include "User.h"
#include "Admin.h"
#include "Parser.h"

class UserRepository {
private:
    string filename;

    vector<user*> users;
    vector<Admin*> admins;

public:

    UserRepository(const string& filename): filename(filename) {}

    ~UserRepository() {
        for (auto u : users)
            delete u;

        for (auto a : admins)
            delete a;

        users.clear();
        admins.clear();
    }

    void loadUser() {
        ifstream file(filename);
        if (!file.is_open())
            throw runtime_error("Cannot open file: " + filename);

        int lineNo = 0;
        string buffer;
        while (getline(file, buffer)) {
            lineNo++;
            if (buffer.empty()) continue;
            try {
                users.push_back(deserializeUser(buffer));
            }
            catch (const runtime_error& e) {
                throw runtime_error(
                    "File: \"" + filename +
                    "\", Line " + std::to_string(lineNo) +
                    ": \"" + buffer +
                    "\" corrupted. Reason: " + e.what()
                );
            }
        }
    }

    void saveUsers() const {
        ofstream file(filename);
        if (!file.is_open())
            throw runtime_error("Error opening file: " + filename);

        for (const auto u : users) {
            file << serialize(u) << "\n";
        }
    }

    user* searchUserByEmail(const string& email) const {
        for (auto u : users)
            if (u->getemail() == email)
                return u;
        return nullptr;
    }

    Admin* searchAdminByEmail(const string& email) const {
        for (auto a : admins)
            if (a->getemail() == email)
                return a;
        return nullptr;
    }

    void addUser(const user& u) {
        users.push_back(new user(u));
    }

    bool updateUserPass(std::string  id,const string & newPassword) {
        user* u = nullptr;
        for (auto us : users)if (us->get_userId() == id) { u = us;break; }
        if (u) {
            u->updatePassword(newPassword);
            return true;
        }
        return false;
    }

    //format: USR-1  USR-2  USR-3 ...
    std::string getNextId() const {
        int max = 0;

        for (const auto& u : users) {
            std::string id = u->get_userId();

            int num = std::stoi(id.substr(4));

            if (num > max)  max = num;
        }
        return "USR-" + std::to_string(max + 1);
    }

private:
    static string serialize(user* u) {
        std::string  id = u->get_userId();
        string name = u->getname();
        string email = u->getemail();
        string hashed = u->getHash();

        string tmp;

        int len = id.size() + name.size() + email.size() + hashed.size() + 3;

        tmp.reserve(len);
        tmp += id;
        tmp.push_back('|');
        tmp += name;
        tmp.push_back('|');
        tmp += email;
        tmp.push_back('|');
        tmp += hashed;

        return tmp;
    }

    static user* deserializeUser(const string& line) {
        auto s = Parser::slice(line, '|');

        if (s.size() != 4) {
            throw std::runtime_error("4 fields were expected but got " +
                std::to_string(s.size()) +
                " fields");
        }
        return new user(s[0], s[1], s[2], s[3],true);
    }
};
//....................
// ToDo:
//    Add User
//    
//....................