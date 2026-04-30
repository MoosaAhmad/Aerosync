#pragma once
#include <string>
#include<iostream>

using namespace std;
class Person {
protected:
    string name;
    string email;
    string passwordHash;
    string decrypter()
    {
        string decrypted = "";
        for (int i = 0; i < passwordHash.length(); i++)
        {
            decrypted += (char)(passwordHash[i] - i);
        }
        return decrypted;
    }

public:
    string encrypter(string pass)
    {
        string encrypted = "";
        for (int i = 0; i < pass.length(); i++)
        {
            encrypted += (char)(pass[i] + i);
        }
        return encrypted;
    }
    Person(string name, string email, string password,bool encrypted) : name(name), email(email)
    {
        if (!encrypted)
            passwordHash = encrypter(password);
        else passwordHash = password;
    }
    bool login(const string& input)
    {
        return passwordHash == encrypter(input);
    }
    void logout() {
        cout << name << " logged out.\n";
    }
    string getHash()
    {
        return passwordHash;
    }
    string getname()
    {
        return name;
    }
    string getemail()
    {
        return email;
    }
    virtual void updatePassword(string newpass)
    {
        passwordHash = encrypter(newpass);
    }
    //  virtual void authenticate() = 0;  // pure virtual so now Person is abstract
    virtual ~Person() {}  // virtual destructor cuz its base class
};
