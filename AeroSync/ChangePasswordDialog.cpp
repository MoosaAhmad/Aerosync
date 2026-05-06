#include "ChangePasswordDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

ChangePasswordDialog::ChangePasswordDialog(Person* u,AppContext* ctx, QWidget* parent)
    : QDialog(parent), currentUser(u),appCtx(ctx)
{
    setWindowTitle("Change Password");
    setFixedSize(420, 360);
    setStyleSheet(R"(
    QDialog {
        background:#0e0e0e; color:#e5e7eb;
        font-family:'Segoe UI'; font-size:13px;
    }
    QLabel#dlgTitle {
        font-size:17px; font-weight:bold; color:#f97316;
    }
    QLabel#fieldLabel {
        color:#6b7280; font-size:11px;
        font-weight:600; letter-spacing:1px; margin-top:4px;
    }
    QLabel#errLabel {
        color:#fca5a5; font-size:12px;
        background:#1f0000; border:1px solid #7f1d1d;
        border-radius:6px; padding:8px 12px;
    }
    QLineEdit {
        background:#141414; border:1px solid #1f2937;
        border-radius:6px; padding:9px 12px; color:#f9fafb;
    }
    QLineEdit:focus { border:1px solid #f97316; }
    QPushButton#confirmBtn {
        background:#f97316; color:#000000; border:none;
        border-radius:6px; padding:10px;
        font-weight:bold; font-size:13px;
    }
    QPushButton#confirmBtn:hover { background:#fb923c; }
    QPushButton#confirmBtn:pressed { background:#ea6c0a; }
    QPushButton#cancelBtn {
        background:#141414; color:#6b7280;
        border:1px solid #1f2937; border-radius:6px; padding:10px;
    }
    QPushButton#cancelBtn:hover {
        background:#1f2937; color:#e5e7eb;
    }
)");

    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(28, 24, 28, 24);
    v->setSpacing(8);

    // Title
    auto* titleLbl = new QLabel("🔒  Change Password");
    titleLbl->setObjectName("dlgTitle");
    v->addWidget(titleLbl);

    v->addSpacing(6);

    // Helper lambda to make a labelled password field
    auto mkField = [&](const QString& labelText, const QString& placeholder) -> QLineEdit* {
        auto* lbl = new QLabel(labelText);
        lbl->setObjectName("fieldLabel");
        v->addWidget(lbl);
        auto* edit = new QLineEdit;
        edit->setPlaceholderText(placeholder);
        edit->setEchoMode(QLineEdit::Password);
        v->addWidget(edit);
        return edit;
    };

    oldPassEdit     = mkField("Current Password",     "Enter your current password");
    newPassEdit     = mkField("New Password",          "Enter new password");
    confirmPassEdit = mkField("Confirm New Password",  "Re-enter new password");

    v->addSpacing(4);

    // Error label (hidden by default)
    errorLabel = new QLabel;
    errorLabel->setObjectName("errLabel");
    errorLabel->setWordWrap(true);
    errorLabel->hide();
    v->addWidget(errorLabel);

    v->addStretch();

    // Buttons
    auto* btnRow    = new QHBoxLayout;
    auto* cancelBtn  = new QPushButton("Cancel");
    auto* confirmBtn = new QPushButton("✅  Update Password");
    cancelBtn->setObjectName("cancelBtn");
    confirmBtn->setObjectName("confirmBtn");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setCursor(Qt::PointingHandCursor);

    connect(cancelBtn,  &QPushButton::clicked, this, &QDialog::reject);
    connect(confirmBtn, &QPushButton::clicked, this, &ChangePasswordDialog::onConfirm);

    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(confirmBtn);
    v->addLayout(btnRow);
}

void ChangePasswordDialog::onConfirm()
{
    QString oldPass  = oldPassEdit->text();
    QString newPass  = newPassEdit->text();
    QString confPass = confirmPassEdit->text();

    // 1. Old password must not be blank
    if (oldPass.isEmpty()) {
        errorLabel->setText("❌  Please enter your current password.");
        errorLabel->show();
        return;
    }

    // 2. Verify old password using login()
    if (!currentUser->login(oldPass.toStdString())) {
        errorLabel->setText("❌  Current password is incorrect.");
        errorLabel->show();
        oldPassEdit->clear();
        oldPassEdit->setFocus();
        return;
    }

    // 3. New password must not be empty
    if (newPass.isEmpty()) {
        errorLabel->setText("❌  New password cannot be empty.");
        errorLabel->show();
        newPassEdit->setFocus();
        return;
    }

    // 4. New password must not be same as old
    if (newPass == oldPass) {
        errorLabel->setText("❌  New password must be different from the current password.");
        errorLabel->show();
        newPassEdit->clear();
        confirmPassEdit->clear();
        newPassEdit->setFocus();
        return;
    }

    // 5. New and confirm must match
    if (newPass != confPass) {
        errorLabel->setText("❌  New passwords do not match. Please try again.");
        errorLabel->show();
        confirmPassEdit->clear();
        confirmPassEdit->setFocus();
        return;
    }

    // 6. All checks passed — update password
    currentUser->updatePassword(newPass.toStdString());
    appCtx->userRepo.saveUsers();
    accept();  // closes dialog with QDialog::Accepted
}