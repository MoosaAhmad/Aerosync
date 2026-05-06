#include "LoginWindow.h"
#include "AdminDashboard.h"
#include "UserDashboard.h"

#include <functional>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

LoginWindow::LoginWindow(AppContext* ctx, QWidget* parent)
    : QWidget(parent), ctx(ctx)
{
    setWindowTitle("AeroSync");
    setFixedSize(460, 600);

    stack = new QStackedWidget(this);
    stack->addWidget(buildLoginPage());    // 0
    stack->addWidget(buildRegisterPage()); // 1

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(stack);
    applyStyles();
}

// Helper: wraps a password QLineEdit with an eye-toggle button
static QWidget* makePasswordRow(QLineEdit* edit, QWidget* parent) {
    auto* row = new QWidget(parent);
    auto* h = new QHBoxLayout(row);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);

    edit->setParent(row);
    edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto* eye = new QPushButton("👁", row);
    eye->setObjectName("eyeBtn");
    eye->setFixedSize(42, 42);
    eye->setCursor(Qt::PointingHandCursor);
    eye->setCheckable(true);

    QObject::connect(eye, &QPushButton::toggled, [edit](bool checked) {
        edit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    h->addWidget(edit);
    h->addWidget(eye);
    return row;
}

QWidget* LoginWindow::buildLoginPage() {
    auto* w = new QWidget;
    w->setObjectName("pageCard");
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(60, 40, 60, 40);
    v->setSpacing(10);

    // Logo row: icon + text
    auto* logoRow = new QHBoxLayout;
    logoRow->setSpacing(10);
    logoRow->setAlignment(Qt::AlignCenter);

    auto* logoIcon = new QLabel("✈");
    logoIcon->setObjectName("logoIcon");

    auto* logoText = new QLabel("AeroSync");
    logoText->setObjectName("logo");

    logoRow->addWidget(logoIcon);
    logoRow->addWidget(logoText);

    auto* sub = new QLabel("Flight Booking System");
    sub->setObjectName("sub");
    sub->setAlignment(Qt::AlignCenter);

    // Divider line
    auto* divider = new QFrame;
    divider->setObjectName("divider");
    divider->setFrameShape(QFrame::HLine);
    divider->setFixedHeight(1);

    loginEmail = new QLineEdit;
    loginEmail->setPlaceholderText("Email address");
    loginEmail->setObjectName("inputField");

    loginPass = new QLineEdit;
    loginPass->setPlaceholderText("Password");
    loginPass->setEchoMode(QLineEdit::Password);
    loginPass->setObjectName("inputField");

    auto* emailLabel = new QLabel("EMAIL");
    emailLabel->setObjectName("fieldLabel");
    auto* passLabel = new QLabel("PASSWORD");
    passLabel->setObjectName("fieldLabel");

    auto* btn = new QPushButton("LOGIN");
    btn->setObjectName("primaryBtn");
    btn->setCursor(Qt::PointingHandCursor);
    connect(btn, &QPushButton::clicked, this, &LoginWindow::handleLogin);

    auto* regLink = new QPushButton("Don't have an account?  Register here →");
    regLink->setObjectName("linkBtn");
    regLink->setCursor(Qt::PointingHandCursor);
    connect(regLink, &QPushButton::clicked,
            [this] { stack->setCurrentIndex(1); });

    v->addStretch(1);
    v->addLayout(logoRow);
    v->addWidget(sub);
    v->addSpacing(16);
    v->addWidget(divider);
    v->addSpacing(16);
    v->addWidget(emailLabel);
    v->addWidget(loginEmail);
    v->addSpacing(6);
    v->addWidget(passLabel);
    v->addWidget(makePasswordRow(loginPass, w));
    v->addSpacing(18);
    v->addWidget(btn);
    v->addSpacing(8);
    v->addWidget(regLink);
    v->addStretch(2);
    return w;
}

QWidget* LoginWindow::buildRegisterPage() {
    auto* w = new QWidget;
    w->setObjectName("pageCard");
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(60, 36, 60, 36);
    v->setSpacing(8);

    auto* title = new QLabel("Create Account");
    title->setObjectName("logo");
    title->setAlignment(Qt::AlignCenter);

    auto* sub = new QLabel("Join AeroSync today");
    sub->setObjectName("sub");
    sub->setAlignment(Qt::AlignCenter);

    auto* divider = new QFrame;
    divider->setObjectName("divider");
    divider->setFrameShape(QFrame::HLine);
    divider->setFixedHeight(1);

    regName = new QLineEdit; regName->setPlaceholderText("Full name"); regName->setObjectName("inputField");
    regEmail = new QLineEdit; regEmail->setPlaceholderText("Email address"); regEmail->setObjectName("inputField");
    regPass = new QLineEdit; regPass->setPlaceholderText("Password"); regPass->setEchoMode(QLineEdit::Password); regPass->setObjectName("inputField");
    regConfirm = new QLineEdit; regConfirm->setPlaceholderText("Confirm password"); regConfirm->setEchoMode(QLineEdit::Password); regConfirm->setObjectName("inputField");

    std::function<QLabel*(const char*)> mkLabel = [](const char* txt) -> QLabel* {
        auto* l = new QLabel(txt);
        l->setObjectName("fieldLabel");
        return l;
    };

    auto* btn = new QPushButton("CREATE ACCOUNT");
    btn->setObjectName("primaryBtn");
    btn->setCursor(Qt::PointingHandCursor);
    connect(btn, &QPushButton::clicked, this, &LoginWindow::handleRegister);

    auto* back = new QPushButton("Already have an account?  Login →");
    back->setObjectName("linkBtn");
    back->setCursor(Qt::PointingHandCursor);
    connect(back, &QPushButton::clicked,
            [this] { stack->setCurrentIndex(0); });

    v->addStretch(1);
    v->addWidget(title);
    v->addWidget(sub);
    v->addSpacing(12);
    v->addWidget(divider);
    v->addSpacing(12);
    v->addWidget(mkLabel("FULL NAME"));        v->addWidget(regName);
    v->addSpacing(4);
    v->addWidget(mkLabel("EMAIL"));            v->addWidget(regEmail);
    v->addSpacing(4);
    v->addWidget(mkLabel("PASSWORD"));         v->addWidget(makePasswordRow(regPass, w));
    v->addSpacing(4);
    v->addWidget(mkLabel("CONFIRM PASSWORD")); v->addWidget(makePasswordRow(regConfirm, w));
    v->addSpacing(18);
    v->addWidget(btn);
    v->addSpacing(8);
    v->addWidget(back);
    v->addStretch(2);
    return w;
}

void LoginWindow::handleLogin() {
    std::string email = loginEmail->text().trimmed().toStdString();
    std::string pass = loginPass->text().toStdString();

    if (email.empty() || pass.empty()) {
        QMessageBox::warning(this, "Error",
                             "Please enter your email and password.");
        return;
    }

    Admin* adm = ctx->userRepo.searchAdminByEmail(email);
    if (adm && adm->login(pass)) {
        hide();
        (new AdminDashboard(adm,ctx))->show();
        return;
    }

    user* u = ctx->userRepo.searchUserByEmail(email);
    if (u && u->login(pass)) {
        hide();
        (new UserDashboard(u, ctx))->show();
        return;
    }

    QMessageBox::critical(this, "Login Failed",
                          "Invalid email or password.");
}

void LoginWindow::handleRegister() {
    QString name = regName->text().trimmed();
    QString email = regEmail->text().trimmed();
    QString pass = regPass->text();
    QString confirm = regConfirm->text();

    if (name.isEmpty() || email.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Error", "All fields are required.");
        return;
    }

    if (pass != confirm) {
        QMessageBox::warning(this, "Error", "Passwords do not match.");
        return;
    }

    if (ctx->userRepo.searchUserByEmail(email.toStdString()) != nullptr) {
        QMessageBox::warning(this, "Error",
                             "This email is already registered.");
        return;
    }

    std::string newId = ctx->userRepo.getNextId();
    user newUser(newId,
                 name.toStdString(),
                 email.toStdString(),
                 pass.toStdString(),
                 false);

    ctx->userRepo.addUser(newUser);

    try {
        ctx->userRepo.saveUsers();
        QMessageBox::information(this, "Account Created",
                                 "Welcome, " + name + "!\n"
                                                      "You can now login with your email.");
        loginEmail->setText(email);
        loginPass->clear();
        stack->setCurrentIndex(0);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Save Error",
                              QString::fromStdString(e.what()));
    }
}

void LoginWindow::applyStyles() {
    setStyleSheet(R"(

        /* ── Base ─────────────────────────────────────────── */
        QWidget {
            background: #0e0e0e;
            color: #e5e7eb;
            font-family: 'Segoe UI';
            font-size: 14px;
        }

        /* ── Page card (inner page background) ────────────── */
        QWidget#pageCard {
            background: #0e0e0e;
        }

        /* ── Logo icon (airplane emoji) ───────────────────── */
        QLabel#logoIcon {
            font-size: 26px;
            color: #f97316;
        }

        /* ── Logo text ────────────────────────────────────── */
        QLabel#logo {
            font-size: 28px;
            font-weight: 700;
            color: #ffffff;
            letter-spacing: 1px;
        }

        /* ── Subtitle ─────────────────────────────────────── */
        QLabel#sub {
            font-size: 12px;
            color: #6b7280;
            letter-spacing: 0.5px;
        }

        /* ── Divider ──────────────────────────────────────── */
        QFrame#divider {
            background: #1f2937;
            border: none;
        }

        /* ── Field labels (uppercase small caps) ──────────── */
        QLabel#fieldLabel {
            font-size: 10px;
            font-weight: 600;
            color: #6b7280;
            letter-spacing: 1.5px;
            margin-bottom: 2px;
        }

        /* ── Text inputs ──────────────────────────────────── */
        QLineEdit#inputField {
            background: #141414;
            border: 1px solid #1f2937;
            border-radius: 8px;
            padding: 11px 14px;
            color: #f9fafb;
            font-size: 14px;
            selection-background-color: #f97316;
        }

        QLineEdit#inputField:hover {
            border: 1px solid #374151;
        }

        QLineEdit#inputField:focus {
            border: 1px solid #f97316;
            background: #161616;
        }

        QLineEdit#inputField::placeholder {
            color: #4b5563;
        }

        /* ── Primary button ───────────────────────────────── */
        QPushButton#primaryBtn {
            background: #f97316;
            color: #000000;
            border: none;
            border-radius: 8px;
            padding: 13px;
            font-weight: 700;
            font-size: 13px;
            letter-spacing: 1.5px;
        }

        QPushButton#primaryBtn:hover {
            background: #fb923c;
        }

        QPushButton#primaryBtn:pressed {
            background: #ea6c0a;
        }

        /* ── Link / secondary button ──────────────────────── */
        QPushButton#linkBtn {
            background: transparent;
            color: #6b7280;
            border: none;
            font-size: 12px;
            letter-spacing: 0.3px;
        }

        QPushButton#linkBtn:hover {
            color: #f97316;
        }

        /* ── Eye toggle button ────────────────────────────── */
        QPushButton#eyeBtn {
            background: #141414;
            border: 1px solid #1f2937;
            border-left: none;
            border-radius: 0 8px 8px 0;
            color: #4b5563;
            font-size: 16px;
            padding: 0 8px;
        }

        QPushButton#eyeBtn:hover {
            color: #f97316;
            background: #1a1a1a;
        }

        QPushButton#eyeBtn:checked {
            color: #f97316;
        }

        /* ── QMessageBox theming ──────────────────────────── */
        QMessageBox {
            background: #141414;
        }

        QMessageBox QLabel {
            color: #e5e7eb;
            font-size: 13px;
        }

        QMessageBox QPushButton {
            background: #f97316;
            color: #000000;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            font-weight: 600;
            min-width: 80px;
        }

        QMessageBox QPushButton:hover {
            background: #fb923c;
        }

    )");
}