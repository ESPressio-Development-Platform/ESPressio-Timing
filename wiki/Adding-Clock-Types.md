# Adding Clock Types

Add a new Timing clock type only when it represents a genuinely distinct clock/lifecycle semantic, not merely a different target timer peripheral or public value type.

## Reuse the layers

A new clock should consume ESPressio System clock/counter capabilities for raw timing, use `TimeTraits<TTime>` for public representation, follow the existing lock-policy/tick genericity where applicable, and integrate observers through ESPressio Observable when meaningful state transitions exist.

## Platform hardware

If the only change is how raw monotonic/high-resolution ticks are acquired on a new MCU, implement a System provider instead of adding a new Timing clock.

## Persistence

Do not make live clock runtime state serializable by default. Define a separate snapshot/state DTO when restoration semantics are meaningful.

## Tests

Cover lifecycle, resolution, monotonicity, conversion, observer transitions, error/initialization paths and behaviour over both fallback and dedicated counter sources where applicable.