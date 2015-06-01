//
// Created by Joshua Bradt on 5/31/15.
//

#include "catch.hpp"
#include "libgetevt/Event.h"

SCENARIO("An event needs processing", "[Event]")
{
    GIVEN("an Event and some pedestals")
    {
        getevt::Event evt {};
        getevt::sample_t dataValue {200};

        for (getevt::addr_t cobo = 0; cobo < getevt::Constants::num_cobos; cobo++) {
            for (getevt::addr_t asad = 0; asad < getevt::Constants::num_asads; asad++) {
                for (getevt::addr_t aget = 0; aget < getevt::Constants::num_agets; aget++) {
                    for (getevt::addr_t ch = 0; ch < getevt::Constants::num_channels; ch++) {
                        getevt::Trace tr {};
                        for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
                            tr.AppendSample(tb, dataValue);
                        }
                        evt.AppendTrace(std::move(tr));
                    }
                }
            }
        }

        REQUIRE(evt.Size() > 0);
    }
}