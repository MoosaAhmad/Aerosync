#include "Receipt.h"

Receipt::Receipt() : receiptId(""), bookingId(""), paymentId("") {}
Receipt::Receipt(const std::string& rId,
    const std::string& bId,
    const std::string& pId)
    : receiptId(rId), bookingId(bId), paymentId(pId) {
}
Receipt::Receipt(const Receipt& other)
    : receiptId(other.receiptId),
    bookingId(other.bookingId),
    paymentId(other.paymentId) {
}
Receipt& Receipt::operator=(const Receipt& other)
{
    if (this != &other)
    {
        receiptId = other.receiptId;
        bookingId = other.bookingId;
        paymentId = other.paymentId;
    }
    return *this;
}
std::string Receipt::getReceiptId() const { return receiptId; }
std::string Receipt::getBookingId() const { return bookingId; }
std::string Receipt::getPaymentId() const { return paymentId; }