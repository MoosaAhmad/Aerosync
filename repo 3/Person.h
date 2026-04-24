#pragma once
class Person {
protected:
    string name;
    string email;
    string passwordHash;
public:
    Person(string name, string email, string passwordHash) : name(name), email(email), passwordHash(passwordHash) {}
    void login() {
        cout << name << " logged in.\n";
    }
    void logout() {
        cout << name << " logged out.\n";
    }
    virtual void authenticate() = 0;  // pure virtual so now Person is abstract
    virtual ~Person() {}  // virtual destructor cuz its base class
};