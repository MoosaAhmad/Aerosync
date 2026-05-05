cmake_minimum_required(VERSION 3.16)

project(test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# IMPORTANT: Qt path (your installation)
set(CMAKE_PREFIX_PATH "C:/Qt/6.11.0/msvc2022_64")

# Qt settings (NO .ui files)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

# Find Qt Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Source files
set(SOURCES
    main.cpp
    LoginWindow.cpp
    UserDashboard.cpp
    AdminDashboard.cpp
    ChangePasswordDialog.cpp
)

# Header files
set(HEADERS
    LoginWindow.h
    UserDashboard.h
    AdminDashboard.h
    ChangePasswordDialog.h

    Admin.h
    Person.h
    AppContext.h
    Booking.h
    DateTime.h
    Flight.h
    Payment.h
    ProfitCalculator.h
    Receipt.h
    Seat.h
    User.h
    BookingRepository.h
    FlightRepository.h
    PaymentRepository.h
    ReceiptRepository.h
    userRepository.h
    Parser.h
)

# Create executable
add_executable(test
    ${SOURCES}
    ${HEADERS}
)

# Link Qt
target_link_libraries(test PRIVATE Qt6::Widgets)