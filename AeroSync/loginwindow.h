#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QStackedWidget>
#include "AppContext.h"

class LoginWindow : public QWidget {
    Q_OBJECT
public:
    explicit LoginWindow(AppContext* ctx, QWidget* parent = nullptr);

private slots:
    void handleLogin();
    void handleRegister();

private:
    AppContext* ctx;
    QStackedWidget* stack;

    QLineEdit* loginEmail;
    QLineEdit* loginPass;
    QLineEdit* regName;
    QLineEdit* regEmail;
    QLineEdit* regPass;
    QLineEdit* regConfirm;

    QWidget* buildLoginPage();
    QWidget* buildRegisterPage();
    void     applyStyles();
};


#endif // LOGINWINDOW_H
