#include "../Class/FareCalculator.h"

long FareCalculator::calculate(long distanceKm) {
    if (distanceKm <= 0) return 0;
    long total = distanceKm * PRICE_PER_KM;
    if (total < MIN_FARE) {
        return MIN_FARE;
    }
    return total;
}

int FareCalculator::calculateDuration(long distanceKm) {
    if (distanceKm <= 0) return 0;
    int minutes = (int)((double)distanceKm / 50.0 * 60.0);
    if (minutes < 30) return 30;
    return minutes;
}