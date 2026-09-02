# System Clock Scheduling

The shared `SystemClockCore` provides scheduling against the disciplined System Clock timeline.

Scheduled callback deadlines are retained in raw nanosecond ticks so scheduling is independent of the public `TTime` representation used to register or service them.

## Shared-core semantics

A callback registered through one `SystemClock<TTime>` facade belongs to the shared core and can be executed when another facade services the same core.

## Discipline interaction

`SlewOnly` is the preferred synchronization mode while deadline consumers are active because it preserves monotonic progression instead of abruptly moving scheduled deadlines relative to the public clock.

Hard steps (`SetTime()` or explicit step modes) should be treated as deliberate timeline rebases and coordinated with consumers whose correctness depends on monotonic deadlines.

## Servicing

Scheduling is not a hidden transport or task system. The application/integration remains responsible for servicing the System Clock as required by the API and for ensuring callback work itself is appropriate for the servicing context.

## Observability

System Clock observers can receive callback scheduled, scheduling failed, executed, execution failed and callbacks-cleared notifications.