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

        string buffer;
        while (getline(file, buffer)) {
            if (buffer.empty()) continue;
            users.push_back(deserializeUser(buffer));
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

    bool updateUserPass(int id,const string & newPassword) {
        user* u = nullptr;
        for (auto us : users)if (us->get_userId() == id) { u = us;break; }
        if (u) {
            u->updatePassword(newPassword);
            return true;
        }
        return false;
    }

private:
    static string serialize(user* u) {
        int id = u->get_userId();
        string name = u->getname();
        string email = u->getemail();
        string hashed = u->getHash();

        string tmp;

        int len = to_string(id).size() + name.size() + email.size() + hashed.size() + 3;

        tmp.reserve(len);
        tmp += to_string(id);
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

        if (s.size() < 4)
            throw runtime_error("Invalid user line: " + line);

        return new user(stoi(s[0]), s[1], s[2], s[3],true);
    }
};
//....................
// ToDo:
//    Add User
//    
//....................