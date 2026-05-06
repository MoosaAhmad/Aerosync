#include <QApplication>
#include <QMessageBox>
#include "LoginWindow.h"
#include "AppContext.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    AppContext* ctx = new AppContext("");
    try {
        ctx->loadAll();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Startup Error",
                              QString("Failed to load data files:\n%1\n\n"
                                      "Make sure Qt Creator's Run > Working Directory\n"
                                      "points to the folder that contains your 'repo 3/' folder.")
                                  .arg(e.what()));
        delete ctx;
        return 1;
    }

    LoginWindow* login = new LoginWindow(ctx);
    login->show();
    return app.exec();
}
