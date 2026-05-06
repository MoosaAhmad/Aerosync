#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#pragma once
#include <vector>
#include <fstream>
#include <string>
#include "User.h"
#include "Admin.h"
#include "Parser.h"

class UserRepository {
private:
    string userFile;
    string adminFile;

    vector<user*> users;
    vector<Admin*> admins;

public:

    UserRepository(const string& userfile,const string&adminfile): userFile(userfile),adminFile(adminfile) {}

    ~UserRepository() {
        for (auto u : users)
            delete u;

        for (auto a : admins)
            delete a;

        users.clear();
        admins.clear();
    }

    void loadUser() {
        ifstream file(userFile);
        if (!file.is_open())
            throw runtime_error("Cannot open file: " + userFile);

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
                    "File: \"" + userFile +
                    "\", Line " + std::to_string(lineNo) +
                    ": \"" + buffer +
                    "\" corrupted. Reason: " + e.what()
                    );
            }
        }
    }

    void saveUsers() const {
        ofstream file(userFile);
        if (!file.is_open())
            throw runtime_error("Error opening file: " + userFile);

        for (const auto u : users) {
            file << serializeUser(u) << "\n";
        }
    }

    void loadAdmins() {
        ifstream file(adminFile);
        if (!file.is_open())
            throw runtime_error("Cannot open file: " + adminFile);

        int lineNo = 0;
        string buffer;
        while (getline(file, buffer)) {
            lineNo++;
            if (buffer.empty()) continue;
            try {
                admins.push_back(deserializeAdmin(buffer));
            }
            catch (const runtime_error& e) {
                throw runtime_error(
                    "File: \"" + userFile +
                    "\", Line " + std::to_string(lineNo) +
                    ": \"" + buffer +
                    "\" corrupted. Reason: " + e.what()
                    );
            }
        }
    }
    void saveAdmins() const {
        ofstream file(adminFile);
        if (!file.is_open())
            throw runtime_error("Error opening file: " + adminFile);

        for (const auto a : admins) {
            file << serializeAdmin(a) << "\n";
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
    vector<user*>getUsers(){return users;}// needed for admin dashboard

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
    static string serializeUser(user* u) {
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
    static std::string serializeAdmin(Admin* u) {
        std::string id     = u->get_AdminId();
        std::string name   = u->getname();
        std::string email  = u->getemail();
        std::string hashed = u->getHash();

        std::string tmp;

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
    static Admin* deserializeAdmin(const string& line) {
        auto s = Parser::slice(line, '|');

        if (s.size() != 4) {
            throw std::runtime_error("4 fields were expected but got " +
                                     std::to_string(s.size()) +
                                     " fields");
        }
        return new Admin(s[0], s[1], s[2], s[3],true);
    }
};
//....................
// ToDo:
//    Add User
//
//....................

#endif // USERREPOSITORY_H
