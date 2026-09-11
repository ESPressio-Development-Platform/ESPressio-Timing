# K1/K2 migration checkpoint

Authoritative branch primitives_redesign revalidated at
9a83beba8cdd0044b357e2d5f29949df70af8814 before changes. Current source, all twelve
existing test files and the workflow were read before execution. Classification was completed before executing the migrated tests.

Semantic classification:

- test_generic_timing, test_rtc: retain generic representation/Stopwatch/RTC behavior.
- test_monotonic_clock: retain independence; replace unrestricted System SetTime
  with status-returning pre-seal rebase and add sealed rejection.
- test_system_clock_shared_core: retain shared typed facades/callback scheduler,
  migrate rebase and verify shared continuity seal.
- test_clock_sample_validation: retain ordering/elapsed/RTT checks using complete
  capture observations. Replace zero/unlimited RTT expectation: K1 requires a
  finite, nonzero admission ceiling. Add quality/reference/overflow boundaries.
- test_clock_synchronization: replace minimum-delay/exponential/pairwise/ApplyStep
  mechanism assertions with two-pass affine, uncertainty, reference and servo tests.
- test_synchronized_system_clock: replace one-sample startup stepping with >=4
  samples, positive span, qualified reference and <=500 us entry.
- test_observers: retain explicit clock/Stopwatch/RTC callback semantics; replace
  getter-driven synchronization notifications with strictly silent getters.
- test_gptimer_compile, test_gptimer_fallback, test_arduino_compile: source review
  reveals older unregistered aliases/native GPTimer assumptions, despite the
  planning-era retain recommendation. Reconcile them with current System counter
  provider APIs and register meaningful coverage rather than preserve stale aliases.
- test_timing: legacy/unregistered. Move useful tick conversion, concurrent
  Stopwatch and RTC failure assertions into current focused tests, then delete.

Implemented validation:

- All 14 registered native CTest cases pass, including retained generic/RTC/observer behavior.
- Positive/negative/zero oscillator drift, phase, heteroscedastic high-delay evidence,
  rejected outlier preservation, integer extremes and fixed-window rollover pass.
- Continuous servo publication and 2.2 million one-nanosecond evaluations through
  negative frequency/slew completion verify no backward step.
- Qualification entry, exact exclusive 1 ms boundary, unknown capture quality,
  insufficient span, reference replacement, holdover expiry, adaptive deadlines
  and scheduler miss diagnostics pass. Elapsed phase slew cannot lower uncertainty.
- Allocation denial covers estimator/discipline and repeated clock/capture/status reads.
- Five README examples, eleven independent headers and two negative compile cases
  pass with actual System/Units/Observable source. The sealed shared clock runtime
  also passes with those dependencies and allocation denial. Only Arduino String
  is stubbed for native Units; obsolete native ESP/GPTimer stubs are removed.
- CI repeats native/public-surface validation and compiles ESP32 consumers with both
  ordinary Units and optional Serializable time representations.

Timing keeps System/Units/Observable dependencies; no family/Radio dependency is
introduced. All dependency references use primitives_redesign. Existing version
numbers remain unchanged.

Physical capture/asymmetry/oscillator certification under hardware temperature,
power, saturated Radio traffic and multi-hop reference conditions remains a later
integration gate. Native numerical tests do not certify a target sub-ms profile.
GitHub workflow execution is recorded in the platform implementation checkpoint.
