#pragma once

#include <limits>

#include "ESPressio_IClock.hpp"
#include "ESPressio_ITimeSource.hpp"
#include "ESPressio_TimeSource.hpp"
#include "ESPressio_TimeTraits.hpp"
#include "ESPressio_LockPolicy.hpp"
#include "ESPressio_ThreadSafeLockPolicy.hpp"

namespace ESPressio {

    namespace Timing {

        /// <summary>Base implementation for clocks backed by a raw platform-neutral time source.</summary>
        /// <typeparam name="TTime">Public unit-aware time representation.</typeparam>
        /// <typeparam name="TTick">Raw nanosecond tick representation used internally.</typeparam>
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members:
 * - _timeSource (ITimeSource*): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
template<
            typename TTime = DefaultClockTime,
            typename TTick = ClockTick
        >
        class ClockBase :
            public virtual IClock<TTime> {

            protected:
                ITimeSource* _timeSource;

                /// <summary>Creates a clock over the supplied time source, falling back to the high-resolution source when null.</summary>
                explicit ClockBase(
                    ITimeSource* timeSource =
                        HighResolutionTimeSource::
                            GetInstance()
                )
                    : _timeSource(
                        timeSource == nullptr
                            ? HighResolutionTimeSource::
                                GetInstance()
                            : timeSource
                    ) {
                }


                TTick GetSourceTime() const {
                    return static_cast<TTick>(
                        Internal::TicksToNanoseconds(
                            _timeSource->GetTicks(),
                            _timeSource->
                                GetTicksPerSecond()
                        )
                    );
                }


                static TTick AddSaturated(
                    TTick left,
                    TTick right
                ) {
                    const TTick maximum =
                        std::numeric_limits<
                            TTick
                        >::max();

                    return
                        right >
                            maximum -
                            left
                            ? maximum
                            : left +
                                right;
                }


                static TTime CreateTime(
                    TTick nanoseconds,
                    TTick resolution
                ) {
                    return
                        TimeTraits<TTime>::
                            template
                            FromNanoseconds<TTick>(
                                nanoseconds,
                                resolution
                            );
                }


                static TTick GetNanoseconds(
                    const TTime& time
                ) {
                    return
                        TimeTraits<TTime>::
                            template
                            ToNanoseconds<TTick>(
                                time
                            );
                }


            public:
                using TimeType = TTime;
                using TickType = TTick;


                /// <inheritdoc/>
                TTime GetResolution() const override {
                    const TTick resolution =
                        static_cast<TTick>(
                            Internal::
                                GetSourceResolution(
                                    _timeSource->
                                        GetTicksPerSecond()
                                )
                        );

                    return CreateTime(
                        resolution,
                        resolution
                    );
                }


                /// <summary>Returns the raw time source backing this clock.</summary>
                ITimeSource*
                GetTimeSource() const {
                    return _timeSource;
                }
        };


        /// <summary>Settable clock base that combines a raw monotonic time source with a configurable public time origin.</summary>
        /// <typeparam name="TLockPolicy">Lock policy used to protect mutable clock origin state.</typeparam>
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 12 bytes [0 bytes dynamic allocation]
 * Members:
 * - _clockMutex (TLockPolicy::Mutex): sizeof(TLockPolicy::Mutex) [0 bytes dynamic allocation]
 * - _baseTime (TTick): sizeof(TTick) [0 bytes dynamic allocation]
 * - _baseSourceTime (TTick): sizeof(TTick) [0 bytes dynamic allocation]
 * Total Memory: 12 bytes known/aligned storage + sizeof(TLockPolicy::Mutex) + sizeof(TTick) + sizeof(TTick) + virtual-base placement/padding per Itanium C++ ABI [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
template<
            typename TTime = DefaultClockTime,
            typename TLockPolicy =
                ThreadSafeLockPolicy,
            typename TTick = ClockTick
        >
        class ClockSettableBase :
            public ClockBase<
                TTime,
                TTick
            >,
            public virtual IClockSettable<
                TTime
            > {

            protected:
                using Base =
                    ClockBase<
                        TTime,
                        TTick
                    >;

                mutable
                    typename TLockPolicy::Mutex
                        _clockMutex;

                TTick _baseTime = 0;
                TTick _baseSourceTime = 0;


                /// <summary>Creates a settable clock over the supplied source and captures its initial source timestamp.</summary>
                explicit ClockSettableBase(
                    ITimeSource* timeSource =
                        HighResolutionTimeSourceT<
                            TLockPolicy
                        >::GetInstance()
                )
                    : Base(timeSource),
                      _baseSourceTime(
                          this->GetSourceTime()
                      ) {
                }


            public:
                using TimeType = TTime;
                using TickType = TTick;


                /// <inheritdoc/>
                TTime GetTime() const override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    const TTick elapsed =
                        sourceTime >=
                            _baseSourceTime
                            ? sourceTime -
                                _baseSourceTime
                            : 0;

                    return this->CreateTime(
                        this->AddSaturated(
                            _baseTime,
                            elapsed
                        ),
                        static_cast<TTick>(
                            Internal::
                                GetSourceResolution(
                                    this->_timeSource->
                                        GetTicksPerSecond()
                                )
                        )
                    );
                }


                /// <inheritdoc/>
                void SetTime(
                    const TTime& time
                ) override {
                    const TTick sourceTime =
                        this->GetSourceTime();

                    typename TLockPolicy::Guard
                        lock(_clockMutex);

                    _baseSourceTime =
                        sourceTime;

                    _baseTime =
                        this->GetNanoseconds(
                            time
                        );
                }
        };

    }

}
