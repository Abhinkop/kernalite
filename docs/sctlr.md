# SCTLR (System Control Register)
* **Purpose:** High-level control of the system, including MMU, caches, and alignment.
* **Size:** 64-bit (Architectural controls primarily reside in bits [31:0]).

## SCTLR_EL1 Register Bit Definitions

This document details the configuration of the **System Control Register (EL1)** used in this kernel. The SCTLR_EL1 is the primary control register for architectural features at Exception Level 1.

| Bit | Name | Value (Policy) | Description |
| :--- | :--- | :--- | :--- |
| [31] | DSSBS | 0 | Spectre V4 Mitigation (Strict) |
| [30] | TE | 0 | Thumb Exception Enable (Ignored in A64) |
| [29] | AFE | 1 | Access Flag Enable (AArch64 Default) |
| [28] | TRE | 1 | TEX Remap Enable (AArch64 Default) |
| [25] | EE | 0 | Exception Endianness (Little-Endian) |
| [23] | SPAN | 0 | Set Privileged Access Never (Strict) |
| [20] | UWXN | 1 | Unprivileged Write implies EL1 XN |
| [19] | WXN | 1 | Write implies Execute-Never |
| [18] | nTWE | 1 | Not Trap WFE (Allow EL0 WFE) |
| [16] | nTWI | 1 | Not Trap WFI (Allow EL0 WFI) |
| [13] | V | 0 | Vectors Bit (Use VBAR_EL1) |
| [12] | I | 1 | Instruction Cache Enable |
| [10] | EnRCTX| 0 | Disable EL0 access to Predictor flushes |
| [8] | SED | 1 | SETEND Instruction Disable (AArch32) |
| [7] | ITD | 1 | IT Instruction Disable (AArch32) |
| [5] | CP15B | 1 | CP15 Barrier Enable (Legacy Support) |
| [2] | C | 1 | Data Cache Enable |
| [1] | A | 1 | Alignment Check Enable |
| [0] | M | 1 | MMU Enable |

---

## [31] DSSBS - Default Speculative Store Bypass Safe
**When FEAT_SSBS is implemented:**
Defines the default value of `PSTATE.SSBS` upon taking an exception to EL1.
* **0b0**: Mitigation **enabled** (Secure).
* **0b1**: Mitigation **disabled** (Performance).

**Kernel Policy**: Set to `0`. Prioritizes security against Spectre V4.

## [30] TE - T32 Exception Enable
**For AArch32 state:**
Controls if AArch32 exceptions are taken in ARM (A32) or Thumb (T32) mode.
* **0b0**: Exceptions taken to A32 state.
* **0b1**: Exceptions taken to T32 state.

**Kernel Policy**: Set to `0`. Note: This bit is ignored in AArch64 state.

## [29] AFE - Access Flag Enable
**When using the translation system:**
* **Kernel Policy**: In AArch64, behaves as **always 1**. `AP[0]` is the Access Flag.

## [28] TRE - TEX Remap Enable
**When using the translation system:**
* **Kernel Policy**: In AArch64, behaves as **always 1**. Attributes managed via `MAIR_EL1`.

## [25] EE - Exception Endianness
This bit defines the endianness of data accesses at EL1 and, crucially, the endianness of **Stage 1 translation table walks**.

* **0b0**: **Little-endian**. `PSTATE.E` is cleared on exception. MMU reads data accesses as LE.
* **0b1**: **Big-endian**. `PSTATE.E` is set on exception. MMU reads data accesses as BE.

**Kernel Policy**: Set to `0`. Matches Little-endian C compiler structures.

## [23] SPAN - Set Privileged Access Never
**When FEAT_PAN is implemented:**
Controls the automatic update of `PSTATE.PAN` on an exception to EL1.
* **0b0**: **Strict**. `PSTATE.PAN` is set to 1 on exception.
* **0b1**: **Permissive**. `PSTATE.PAN` value is unchanged.

**Kernel Policy**: Set to `0`. Prevents accidental kernel access to user memory.

## [20] UWXN - Unprivileged Write implies PL1 XN
**When using the translation system:**
* **0b0**: No effect.
* **0b1**: Any region writable at EL0 is forced to **Execute-Never** for EL1.

**Kernel Policy**: Set to `1`. Essential security hardening.

## [19] WXN - Write implies Execute-Never
**When using the translation system:**
* **0b0**: No effect.
* **0b1**: Any region writable in the translation regime is forced to **Execute-Never**.

**Kernel Policy**: Set to `1`. Enforces W^X (Write XOR Execute) policy.

## [18] nTWE - Not Trap WFE
**Controls EL0 execution of WFE:**
* **0b0**: Traps EL0 `WFE` instructions to EL1.
* **0b1**: Does not trap `WFE`.

**Kernel Policy**: Set to `1`. Allows user-space power-saving hints.

## [16] nTWI - Not Trap WFI
**Controls EL0 execution of WFI:**
* **0b0**: Traps EL0 `WFI` instructions to EL1.
* **0b1**: Does not trap `WFI`.

**Kernel Policy**: Set to `1`. Change to `0` if implementing a strict power scheduler.

## [13] V - Vectors Bit
**Selects base address of exception vectors:**
* **0b0**: Normal exception vectors. Base address held in `VBAR_EL1`.
* **0b1**: High exception vectors (Hivecs) at `0xFFFF0000`.

**Kernel Policy**: Set to `0`. Allows for programmable vector tables.

## [12] I - Instruction Cache Control
**For accesses at EL1 and EL0:**
* **0b0**: Instruction accesses are Non-cacheable.
* **0b1**: Instruction accesses are Cacheable.

**Kernel Policy**: Set to `1`. Essential for reasonable performance.

## [10] EnRCTX - Enable Restriction Context
**When FEAT_SPECRES is implemented:**
* **0b0**: EL0 access to Predictor Restriction instructions is **Disabled**.
* **0b1**: EL0 access is **Enabled**.

**Kernel Policy**: Set to `0`. Prevents user-space from interfering with speculation state.

## [8] SED - SETEND Instruction Disable
**For A32/T32 execution:**
* **0b0**: `SETEND` instruction execution is enabled.
* **0b1**: `SETEND` instructions are **UNDEFINED**.

**Kernel Policy**: Set to `1`. Prevents runtime endianness changes.

## [7] ITD - IT Disable
**For T32 execution:**
* **0b0**: All `IT` instruction functionality is enabled.
* **0b1**: `IT` instructions are **UNDEFINED** (deprecated complex logic disabled).

**Kernel Policy**: Set to `1`.

## [5] CP15BEN - CP15 Barrier Enable
**Enables legacy barrier instructions (DMB, DSB, ISB):**
* **0b0**: CP15 barrier instructions are **UNDEFINED**.
* **0b1**: CP15 barrier instructions are enabled.

**Kernel Policy**: Set to `1`. Maintains compatibility for legacy code.

## [2] C - Data Cache Control
**For data accesses and page table walks:**
* **0b0**: Data access to Normal memory is Non-cacheable.
* **0b1**: Data access to Normal memory is Cacheable.

**Kernel Policy**: Set to `1`. Requires active MMU for proper operation.

## [1] A - Alignment Check Enable
**Enable for Alignment fault checking:**
* **0b0**: Alignment fault checking disabled.
* **0b1**: Alignment fault checking enabled.

**Kernel Policy**: Set to `1`. Catches unaligned access bugs immediately.

## [0] M - MMU Enable
**Master switch for address translation:**
* **0b0**: EL1 and EL0 Stage 1 address translation disabled.
* **0b1**: EL1 and EL0 Stage 1 address translation enabled.

**Kernel Policy**: Set to `1`. Required for virtual memory.

---

## Initialization (boot.s)

```assembly
    mrs     x0, sctlr_el1
    
    // Clear bits (Set to 0)
    // 31:DSSBS, 30:TE, 25:EE, 23:SPAN, 13:V, 10:EnRCTX
    ldr     x1, =0x82802400
    bic     x0, x0, x1
    
    // Set bits (Set to 1)
    // 29:AFE, 28:TRE, 20:UWXN, 19:WXN, 18:nTWE, 16:nTWI
    // 12:I, 8:SED, 7:ITD, 5:CP15BEN, 2:C, 1:A, 0:M
    ldr     x1, =0x301D11A7
    orr     x0, x0, x1
    
    msr     sctlr_el1, x0
    isb                           // Ensure changes are visible immediately
