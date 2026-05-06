#ifndef CHANGEPASSWORDDIALOG_H
#define CHANGEPASSWORDDIALOG_H
#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include"Person.h"
#include "AppContext.h"

class ChangePasswordDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChangePasswordDialog(Person* u, AppContext* ctx,QWidget* parent = nullptr);

private slots:
    void onConfirm();

private:
    Person*      currentUser;
    QLineEdit* oldPassEdit;
    QLineEdit* newPassEdit;
    QLineEdit* confirmPassEdit;
    QLabel*    errorLabel;
    AppContext* appCtx;
};

#endif // CHANGEPASSWORDDIALOG_H