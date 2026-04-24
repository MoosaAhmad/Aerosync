#include<iostream>
using namespace std;

class ProfitCalculator
{
private:
    double totalCost;
    double totalRevenue;
    int ticketsSold;

public:
    ProfitCalculator(double cost = 0, double revenue = 0, int tickets = 0)
    {
        totalCost = cost;
        totalRevenue = revenue;
        ticketsSold = tickets;
    }

    // Methods

    double calcProfit()
    {
        return totalRevenue - totalCost;
    }
    bool isProfitable()
    {
        return calcProfit() > 0;
    }
    void displayReport()
    {
        cout << "Total Cost: " << totalCost << endl;
        cout << "Total Revenue: " << totalRevenue << endl;
        cout << "Tickets Sold: " << ticketsSold << endl;
        cout << "Profit: " << calcProfit() << endl;

        if (isProfitable())
            cout << "Status: Profitable" << endl;
        else
            cout << "Status: Not Profitable" << endl;
    }
};

int main()
{

	return 0;
}