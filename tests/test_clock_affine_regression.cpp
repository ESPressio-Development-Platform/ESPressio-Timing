#include <ESPressio_ClockRegression.hpp>
#include <cassert>
#include <cmath>
using namespace ESPressio::Timing;
int main() {
    ClockSynchronizationProfile p;
    ClockRegression<8> r;
    for (unsigned i=0;i<6;++i) r.Add({i*1000000000ull,1000,1000,ClockUncertainty::Known(100),true});
    // Two large-delay excursions retain low statistical weight. They cannot become the entire phase estimate.
    r.Add({6000000000ull,5000000,5000000,ClockUncertainty::Known(10000000),true});
    r.Add({7000000000ull,-5000000,-5000000,ClockUncertainty::Known(10000000),true});
    const auto fit=r.Calculate(p); assert(fit.Valid && fit.Inliers==8);
    assert(std::fabs(fit.Intercept-1000)<2 && std::fabs(fit.Slope*1000000)<0.001);
    assert(fit.MaximumObservationUncertainty==10000000); // A good fit never discounts the physical safety bound.
    ClockRegression<4> narrow;
    for (unsigned i=0;i<4;++i) narrow.Add({100+i,7,7,ClockUncertainty::Known(1),true});
    assert(narrow.Calculate(p).ObservationSpan==3);
    auto invalid=p; invalid.MaximumSlewRatePpm=500; invalid.MaximumFrequencyCorrectionPpm=999499.9999;
    assert(!invalid.IsValid(8)); // Rounding to ppb must not permit a zero effective rate.
    ClockRegression<4> extremes;
    for (unsigned i=0;i<4;++i) {
        const auto value=INT64_MAX-static_cast<std::int64_t>(100+i*10);
        extremes.Add({UINT64_MAX-1000+i*100,value,value,ClockUncertainty::Known(100),true});
    }
    assert(extremes.Calculate(p).Valid && std::isfinite(extremes.Calculate(p).Slope));
}
