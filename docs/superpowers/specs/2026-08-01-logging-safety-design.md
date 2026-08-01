# Logging Safety Design

## Scope

Correct memory-safety and status-handling defects in `Module/logging` without
changing its blocking UART architecture or adding new logging features.

## Current defects

- `vsnprintf` returns the number of characters that would have been written.
  When that value is at least 256, the current newline write and UART transfer
  extend beyond the 256-byte stack buffer.
- A null format pointer is passed to `vsnprintf`, which has undefined behavior.
- The `vsnprintf` result is narrowed from `int` to `int16_t` before validation.
- `HAL_StatusTypeDef` is returned as `log_status` only because both enums happen
  to use the same numeric values and ordering.

## Behavior

- `log_write` returns `LOG_ERROR` if logging is not initialized, the format
  pointer is null, or formatting fails.
- A formatted message that fits is transmitted unchanged with one trailing
  newline.
- An oversized formatted message is truncated to the largest prefix that fits;
  the final byte transmitted is still the trailing newline.
- The UART transmit result is explicitly mapped to `LOG_OK`, `LOG_BUSY`,
  `LOG_TIMEOUT`, or `LOG_ERROR`.
- `log_init`, the blocking transmit model, buffer capacity, and public status
  values remain otherwise compatible.

## Implementation boundaries

- Keep production edits within `Module/logging`.
- Do not modify CubeMX-generated code or generated `Debug` makefiles.
- Add a host-side regression test with a minimal HAL stub so formatting and
  status mapping can be exercised without target hardware.
- Avoid DMA, queues, locking, timestamping, severity levels, or unrelated
  renaming in this fix.

## Verification

1. Demonstrate the regression tests fail against the current implementation for
   the oversized-message case.
2. Apply the smallest production correction and rerun the host tests.
3. Build the STM32 Debug target with the configured CubeIDE toolchain.

