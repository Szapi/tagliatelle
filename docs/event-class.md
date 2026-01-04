# Event Class Definition

This document describes how **events** are structured in the system and how an integrating codebase **must** define a concrete *event class*.

The event class definition is a foundational contract: both the **C++ backend** and the **Flutter UI** are compiled against it, and all storage, processing, and visualization logic depends on it.

---

## What Is an Event?

An event represents a single data point on a timeline.  
At minimum, it has a timestamp, and optionally a number of additional attributes describing context, source, and meaning.

Typical examples include:
- system logs from multiple devices
- trace events from multiple processes or threads
- events emitted by distributed services

---

## Supported Attribute Types

An event may consist of attributes of the following types.

### 1. Timestamp

- Mandatory for every event
- Serves as the **primary ordering key**
- All visualization and navigation is centered around time

---

### 2. Numeric Field

- Plain integer value
- Examples:
  - source ID
  - thread ID
  - CPU core index

---

### 3. Dynamic Enum (Interned Text)

A **dynamic enum** is a text attribute whose possible values are not known in advance, but where many events are expected to share a small set of values.

Examples:
- host name
- process name
- subsystem name

To avoid storing repeated strings, dynamic enums are encoded using a runtime-built **word dictionary**:

- each unique string is stored once
- events reference it by a compact integer ID
- decoding from integer ID to text should be possible in **O(1)** time
- encoding from text to integer ID should be possible in **O(log n)** time

This behaves like an enum discovered at runtime, hence the name *dynamic enum*.

---

### 4. Static Enum

A **static enum** has a predefined, fixed set of possible values known at compile time.

Typical example:
- log severity (`Debug`, `Info`, `Warning`, `Error`, `Fatal`)

Characteristics:
- encoded as integers
- efficient lookup and comparison
- suitable for optimized encoding strategies

---

### 5. Text Field

A plain UTF-8 text field.

Examples:
- log message
- diagnostic description
- error details

Unlike enums, text fields are free-form and are not deduplicated.

---

## Defining an Event Class

To build a working application, the integrating codebase must define:

- which attributes an event consists of
- the type of each attribute
- attribute parameters and constraints
- how events are ordered

Example (conceptual):
- Timestamp
- Dynamic enum: `host_name`
- Dynamic enum: `process_name`
- Static enum: `severity`
- Text field: `message`

---

## Attribute Parameters

Attributes may define additional parameters, such as:

- maximum allowed text length
- behavior when the limit is exceeded:
  - truncate
  - reject
  - redirect to a dedicated long-text storage
- whether the attribute is mandatory or optional

These rules are enforced consistently by the backend.

---

## Event Ordering and Determinism

Some events may have the same timestamp.  
To ensure predictable behavior, the system requires a **strict total ordering** of events.

### Ordering Rules

- **Timestamp is always the primary ordering key**
- Additional attributes may be used as deterministic tiebreakers
- The event class definition specifies:
  - which attributes participate in ordering
  - their priority order

Example:
1. timestamp
2. host name
3. process name
4. source ID

This guarantees stable visualization, reproducible queries, and consistent behavior when filtering or switching views.

---

## Why Strict Ordering Matters

Strict total ordering enables practical workflows beyond visualization.

### Duplicate Detection During Import

When importing events from files, overlapping or duplicated datasets are common.  
A strict ordering allows the backend to:

- detect identical or equivalent events across multiple files
- avoid importing the same event twice
- group competing representations of the same logical event

### User-Guided Conflict Resolution

In some cases, duplicated events differ only in quality or formatting.

Example:
- two files contain the same events
- one version preserves multi-line message formatting
- the other has lost newline characters

Because events are strictly ordered and comparable, the system can detect these conflicts and allow the user to choose which version to keep.

---

## Notes on Stability

Once an event class is defined, it becomes the schema the entire system operates on.  
Changing it affects both backend and UI and typically requires recompilation.

---

## Future Extensions

The current attribute set is intentionally minimal.  
Possible future extensions include:

- floating-point attributes
- structured or nested attributes
- derived or computed fields
- user-defined attribute types