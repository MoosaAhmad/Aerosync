#include<iostream>
using namespace std;

#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[]) {
    // 1. Initialize the Qt Application instance
    QApplication app(argc, argv);

    // 2. Create the window object
    QWidget window;
    window.resize(1200, 768);                  // Width, Height in pixels
    window.setWindowTitle("Aerosync - Main"); // Set the window title

    // 3. Make the window visible
    window.show();

    // 4. Start the application's main event loop
    return app.exec();
}