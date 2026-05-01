#include <iostream>
#include"userRepository.h"
#include"BookingRepository.h"
// this is just a test file, when you make your code/functions and programs, you can run them here when you dont want to make any changes to the main file.
int main()
{

    UserRepository ur("users.txt");
    ur.loadUser();
    user* u = ur.searchUserByEmail("moosa679@gmail.com");
    if (u) {
        cout << "found\n";
    }
    else {
        cout << "not found\n";
    }
    ur.saveUsers();


    BookingRepository br("bookings.txt");

    br.load();

    br.save();


    std::cout << "Hello World!\n";
    return 0;
}