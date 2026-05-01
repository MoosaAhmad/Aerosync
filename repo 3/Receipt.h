#pragma once
#include <string>

class Receipt
{
private:
    std::string receiptId;
    std::string bookingId;
    std::string paymentId;

public:

    Receipt() : receiptId(""), bookingId(""), paymentId("") {}
    Receipt(const std::string& rId,
        const std::string& bId,
        const std::string& pId)
        : receiptId(rId), bookingId(bId), paymentId(pId) {
    }
    Receipt(const Receipt& other)
        : receiptId(other.receiptId),
        bookingId(other.bookingId),
        paymentId(other.paymentId) {
    }
    Receipt& operator=(const Receipt& other)
    {
        if (this != &other)
        {
            receiptId = other.receiptId;
            bookingId = other.bookingId;
            paymentId = other.paymentId;
        }
        return *this;
    }
    std::string getReceiptId() const { return receiptId; }
    std::string getBookingId() const { return bookingId; }
    std::string getPaymentId() const { return paymentId; }
};