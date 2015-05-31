//
// Created by Joshua Bradt on 5/30/15.
//

#include <vector>
#include "catch.hpp"

#include "libgetevt/Trace.h"

SCENARIO("Math needs to be performed on traces", "[Trace]")
{
    GIVEN("Two Traces")
    {
        getevt::Trace tr1 {};
        getevt::Trace tr2 {};

        for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
            tr1.AppendSample(i, i + 1);
            REQUIRE(tr1.GetSample(i) == (i + 1));
            tr2.AppendSample(i, 2*i + 1);
            REQUIRE(tr2.GetSample(i) == (2*i + 1));
        }

        WHEN("trace 2 is added to trace 1")
        {
            tr1 += tr2;
            THEN("the samples are added")
            {
                for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
                    CAPTURE(i);
                    REQUIRE(tr1.GetSample(i) == (2*i + i + 2));
                }
            }
        }

        WHEN("trace 1 is subtracted from trace 2")
        {
            tr2 -= tr1;
            THEN("the samples are subtracted")
            {
                for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
                    CAPTURE(i);
                    REQUIRE(tr2.GetSample(i) == (2*i - i));
                }
            }
        }

        WHEN("trace 2 is divided by trace 1")
        {
            tr2 /= tr1;
            THEN("the samples are divided")
            {
                for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
                    CAPTURE(i);
                    REQUIRE(tr2.GetSample(i) == ((2*i+1) / (i+1)));
                }
            }
        }
    }

    GIVEN("A trace and a scalar")
    {
        getevt::Trace tr {};
        getevt::sample_t scalar {4};

        for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
            tr.AppendSample(i, i + 1);
            REQUIRE(tr.GetSample(i) == (i + 1));
        }

        WHEN("the trace is divided by the scalar")
        {
            tr /= scalar;
            THEN("the samples are divided by the scalar")
            {
                for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
                    CAPTURE(i);
                    REQUIRE(tr.GetSample(i) == (i+1) / scalar);
                }
            }
        }

        WHEN("the scalar is subtracted from the trace")
        {
            tr -= scalar;
            THEN("the scalar is subtracted from the samples")
            {
                for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
                    CAPTURE(i);
                    REQUIRE(tr.GetSample(i) == (i+1) - 4);
                }
            }
        }
    }
}

bool checkIfNormalized(const getevt::Trace& tr)
{
    int mean {0};

    for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
        mean += tr.GetSample(i);
    }
    mean /= getevt::Constants::num_tbs;

    return mean == 0;
}

SCENARIO("A trace needs to be normalized", "[Trace]")
{
    GIVEN("a linear trace")
    {
        getevt::Trace tr {};
        for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
            tr.AppendSample(i, i);
            REQUIRE(tr.GetSample(i) == i);
        }

        WHEN("the trace is renormalized to zero")
        {
            tr.RenormalizeToZero();
            THEN("its mean is zero")
            {
                REQUIRE(checkIfNormalized(tr));
            }
        }
    }

    GIVEN("a quadratic trace")
    {
        getevt::Trace tr {};
        for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
            tr.AppendSample(i, ((i/16)*(i/16)));
            REQUIRE(tr.GetSample(i) == ((i/16)*(i/16)));
        }

        WHEN("the trace is renormalized to zero")
        {
            tr.RenormalizeToZero();
            THEN("its mean is zero")
            {
                REQUIRE(checkIfNormalized(tr));
            }
        }
    }

    GIVEN("an empty trace")
    {
        getevt::Trace tr {};
        REQUIRE(tr.GetNumberOfTimeBuckets() == 0);

        WHEN("we try to renormalize the trace")
        {
            THEN("it throws an exception")
            {
                REQUIRE_THROWS_AS(tr.RenormalizeToZero(), getevt::Exceptions::No_Data);
            }
        }
    }
}

SCENARIO("A trace needs to be thresholded", "[Trace]")
{
    GIVEN("a trace filled with values and a threshold")
    {
        getevt::Trace tr {};
        for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
            tr.AppendSample(i, i*2);
            REQUIRE(tr.GetSample(i) == (i*2));
        }

        WHEN("the trace has samples below threshold")
        {
            getevt::sample_t threshold {200};
            tr.ApplyThreshold(threshold);

            THEN("the trace is clipped")
            {
                for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
                    getevt::sample_t exp = i*2 < threshold ? 0 : i*2;
                    CAPTURE(i);
                    REQUIRE(tr.GetSample(i) == exp);
                }
            }
        }

        WHEN("the trace has no samples below threshold")
        {
            getevt::sample_t threshold {1};
            tr.ApplyThreshold(threshold);

            THEN("the trace is not clipped")
            {
                for (getevt::tb_t i = 0; i < getevt::Constants::num_tbs; i++) {
                    CAPTURE(i);
                    REQUIRE(tr.GetSample(i) == (i*2));
                }
            }
        }
    }
}

SCENARIO("Samples need to be packed and unpacked", "[Trace][Slow]")
{
    WHEN("samples are compacted")
    {
        THEN("the compact structure is correct")
        {
            for (uint16_t tb = 0; tb < 512; tb++) {
                for (int16_t val = -4095; val < 0xFFF; val++) {
                    uint32_t res = getevt::Trace::CompactSample(tb, val);
                    uint16_t res_tb = (res & 0xFF8000) >> 15;
                    int16_t res_val = res & 0xFFF;
                    int16_t res_par = (res & 0x1000) >> 12;
                    if (res_par == 1) {
                        res_val *= -1;
                    }
                    REQUIRE(tb == res_tb);
                    REQUIRE(val == res_val);
                }
            }
        }
    }

    WHEN("compacted samples are unpacked")
    {
        THEN("they are correct")
        {
            for (uint16_t tb = 0; tb < 512; tb++) {
                for (int16_t val = -4095; val < 0xFFF; val++) {
                    int16_t parity = val < 0 ? (1 << 12) : 0;
                    uint32_t compacted = (tb << 15) | abs(val) | parity;
                    auto unpacked = getevt::Trace::UnpackSample(compacted);
                    REQUIRE(tb == unpacked.first);
                    REQUIRE(val == unpacked.second);
                }
            }
        }
    }

    WHEN("a round-trip is made")
    {
        THEN("the unpacked samples equal the originals")
        {
            for (uint16_t tb = 0; tb < 512; tb++) {
                for (int16_t val = -4095; val < 0xFFF; val++) {
                    uint32_t comp = getevt::Trace::CompactSample(tb, val);
                    auto unp = getevt::Trace::UnpackSample(comp);
                    REQUIRE(tb == unp.first);
                    REQUIRE(val == unp.second);
                }
            }
        }
    }
}

SCENARIO("A trace needs its zeros dropped", "[Trace]")
{
    GIVEN("a trace with some zero elements")
    {
        getevt::Trace tr {};
        for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
            tr.AppendSample(tb, tb % 2);
        }

        WHEN("the zeros are dropped")
        {
            tr.DropZeros();

            THEN("the zero elements are removed")
            {
                for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
                    if (tb % 2) {
                        REQUIRE_NOTHROW(tr.GetSample(tb));
                    }
                    else {
                        REQUIRE_THROWS_AS(tr.GetSample(tb), std::out_of_range);
                    }
                }
            }
        }
    }
}

SCENARIO("Some elements are replaced in a trace", "[Trace]")
{
    GIVEN("a trace with no empty time buckets")
    {
        getevt::Trace tr {};

        for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
            tr.AppendSample(tb, tb);
            REQUIRE(tr.GetSample(tb) == tb);
        }

        WHEN("a sample is appended to a time bucket that's full")
        {
            THEN("the stored sample is changed")
            {
                for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
                    tr.AppendSample(tb, tb*4);
                    REQUIRE(tr.GetSample(tb) == (tb*4));
                }
            }
        }
    }
}

SCENARIO("A trace needs to be moved or copied", "[Trace]")
{
    GIVEN("two traces")
    {
        getevt::addr_t cobo = 4;
        getevt::addr_t asad = 2;
        getevt::addr_t aget = 1;
        getevt::addr_t channel = 14;
        getevt::pad_t pad = 1942;

        getevt::Trace tr {cobo, asad, aget, channel, pad};

        for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
            tr.AppendSample(tb, tb*2);
        }

        getevt::addr_t cobo2 = 5;
        getevt::addr_t asad2 = 3;
        getevt::addr_t aget2 = 0;
        getevt::addr_t channel2 = 27;
        getevt::pad_t pad2 = 2014;

        getevt::Trace tr2 {cobo, asad, aget, channel, pad};

        for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
            tr2.AppendSample(tb, tb + 5);
        }

        WHEN("the trace is copy-assigned to a new trace")
        {
            getevt::Trace tr3 = tr;

            THEN("the trace is copied")
            {
                REQUIRE(tr3.getCoboId() == tr.getCoboId());
                REQUIRE(tr3.getAsadId() == tr.getAsadId());
                REQUIRE(tr3.getAgetId() == tr.getAgetId());
                REQUIRE(tr3.getChannel() == tr.getChannel());
                REQUIRE(tr3.getPadId() == tr.getPadId());

                for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
                    CAPTURE(tb);
                    REQUIRE(tr3.GetSample(tb) == tr.GetSample(tb));
                }
            }
        }

        WHEN("the trace is copy-assigned to a filled trace")
        {
            tr2 = tr;

            THEN("the old trace is replaced with a copy of the new one")
            {
                REQUIRE(tr2.getCoboId() == tr.getCoboId());
                REQUIRE(tr2.getAsadId() == tr.getAsadId());
                REQUIRE(tr2.getAgetId() == tr.getAgetId());
                REQUIRE(tr2.getChannel() == tr.getChannel());
                REQUIRE(tr2.getPadId() == tr.getPadId());

                for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
                    CAPTURE(tb);
                    REQUIRE(tr2.GetSample(tb) == tr.GetSample(tb));
                }
            }
        }

        WHEN("the trace is move-assigned to a new trace")
        {
            getevt::Trace tr3 = std::move(tr);

            THEN("the trace is moved")
            {
                REQUIRE(tr3.getCoboId() == tr.getCoboId());
                REQUIRE(tr3.getAsadId() == tr.getAsadId());
                REQUIRE(tr3.getAgetId() == tr.getAgetId());
                REQUIRE(tr3.getChannel() == tr.getChannel());
                REQUIRE(tr3.getPadId() == tr.getPadId());

                for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
                    CAPTURE(tb);
                    REQUIRE(tr3.GetSample(tb) == (2*tb));
                }

                REQUIRE(tr.GetNumberOfTimeBuckets() == 0);
            }
        }

        WHEN("the trace is move-assigned to a filled trace")
        {
            tr2 = std::move(tr);

            THEN("the old trace is replaced with the new one")
            {
                REQUIRE(tr2.getCoboId() == tr.getCoboId());
                REQUIRE(tr2.getAsadId() == tr.getAsadId());
                REQUIRE(tr2.getAgetId() == tr.getAgetId());
                REQUIRE(tr2.getChannel() == tr.getChannel());
                REQUIRE(tr2.getPadId() == tr.getPadId());

                for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
                    CAPTURE(tb);
                    REQUIRE(tr2.GetSample(tb) == (2*tb));
                }

                REQUIRE(tr.GetNumberOfTimeBuckets() == 0);
            }
        }

        WHEN("the trace is copy-constructed to a new trace")
        {
            getevt::Trace tr3 {tr};

            THEN("the trace is copied")
            {
                REQUIRE(tr3.getCoboId() == tr.getCoboId());
                REQUIRE(tr3.getAsadId() == tr.getAsadId());
                REQUIRE(tr3.getAgetId() == tr.getAgetId());
                REQUIRE(tr3.getChannel() == tr.getChannel());
                REQUIRE(tr3.getPadId() == tr.getPadId());

                for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
                    CAPTURE(tb);
                    REQUIRE(tr3.GetSample(tb) == tr.GetSample(tb));
                }
            }
        }

        WHEN("the trace is move-constructed to a new trace")
        {
            getevt::Trace tr3 {std::move(tr)};

            THEN("the trace is moved")
            {
                REQUIRE(tr3.getCoboId() == tr.getCoboId());
                REQUIRE(tr3.getAsadId() == tr.getAsadId());
                REQUIRE(tr3.getAgetId() == tr.getAgetId());
                REQUIRE(tr3.getChannel() == tr.getChannel());
                REQUIRE(tr3.getPadId() == tr.getPadId());

                for (getevt::tb_t tb = 0; tb < getevt::Constants::num_tbs; tb++) {
                    CAPTURE(tb);
                    REQUIRE(tr3.GetSample(tb) == (2*tb));
                }

                REQUIRE(tr.GetNumberOfTimeBuckets() == 0);
            }
        }
    }
}