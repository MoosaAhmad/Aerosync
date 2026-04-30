#pragma once
#include <string>

class Receipt
{
private:
    std::string receiptId;
    std::string bookingId;
    std::string paymentId;

public:
    Receipt();
   
    Receipt(const std::string& rId,
        const std::string& bId,
        const std::string& pId);

    Receipt(const Receipt& other);
 
    Receipt& operator=(const Receipt& other);

    std::string getReceiptId() const;
    std::string getBookingId() const;
    std::string getPaymentId() const;
};
