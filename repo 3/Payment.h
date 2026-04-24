#pragma once
class Payment {
protected:
    string transactionId;
    double amount;
    string bankName;
    string paymentMethod;

public:
    Payment(string tId, double amt, string bank, string method) {
        transactionId = tId;
        amount = amt;
        bankName = bank;
        paymentMethod = method;
    }

    bool processPayment() {
        cout << "Initiating secure connection to " << bankName << "...\n";

        if (amount <= 0) {
            cout << "Error - Invalid amount ($" << amount << ").\n";
            return false;
        }

        cout << "Charging $" << amount << " via " << paymentMethod << "...\n";
        cout << "Charge successful.\n";
        return true;
    }

    bool verifyPayment() {
        cout << "Checking transaction ID [" << transactionId << "] against bank records...\n";

        if (transactionId.empty()) {
            cout << "Failed. No transaction ID provided.\n";
            return false;
        }

        cout << "Transaction authenticated and verified.\n";
        return true;
    }

    // It takes a Booking pointer so it can pass it to the Receipt.
    void generateReceipt(Booking* currentBooking) {
        cout << "\nRequesting receipt generation...\n";

        if (verifyPayment()) {
            string rId = "REC-" + transactionId;
            DateTime currentTime;

            // Creating a "Receipt" class object.
            // We use 'this' to pass the current Payment object as a pointer (Payment*)
            Receipt newReceipt(rId, currentBooking, this, currentTime);

            // here we just call the methods defined in the Receipt class like:

            //newReceipt.display();
            //newReceipt.print();
            //newReceipt.exportToFile();
        }
        else {
            cout << "Cannot generate receipt. Payment verification failed.\n";
        }
    }
};