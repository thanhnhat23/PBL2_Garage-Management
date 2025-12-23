#include <algorithm>
class FareCalculator {
public:
    static const long PRICE_PER_KM = 1000; 
    static const long MIN_FARE = 50000; 
    static long calculate(long distanceKm);
    static int calculateDuration(long distanceKm);
};