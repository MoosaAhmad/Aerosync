cmake_minimum_required(VERSION 3.16)
project(OOP-Aerosync LANGUAGES CXX)
set(CMAKE_AUTOMOC ON) # Handles Qt Signals/Slots
set(CMAKE_AUTOUIC ON) # Handles .ui files
find_package(Qt6 REQUIRED COMPONENTS Widgets)
# Add ALL your .cpp files here
# Note the quotes around pathnames containing spaces
add_executable(OOP-Aerosync
    "repo 3/Admin.h"
    "repo 3/Booking.cpp"
    "repo 3/Booking.h"
    "repo 3/BookingRepository.h"
    "repo 3/DateTime.h"
    "repo 3/Flight.h"
    "repo 3/FlightRepository.h"
    "repo 3/Main.cpp"
    "repo 3/Parser.h"
    "repo 3/Payment.h"
    "repo 3/PaymentRepository.h"
    "repo 3/Profit Calculator.h"
    "repo 3/Person.h"
    "repo 3/Receipt.cpp"
    "repo 3/ReceiptRepository.h"
    "repo 3/Seat.h"
    "repo 3/User.h"
    "repo 3/userRepository.h"
)
target_link_libraries(OOP-Aerosync PRIVATE Qt6::Widgets)