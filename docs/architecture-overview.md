# Core Backend–UI Architecture Overview

This document describes the high-level architecture and working principles of the backend–UI interaction.

---

## Responsibility Split

### C++ Backend

The C++ backend is responsible for **owning and processing all event data**. Its primary responsibilities include:

- storing events in memory-efficient data structures
- encoding event data for performance
- performing compute-heavy operations such as:
  - filtering
  - searching (e.g. regex-based)
  - rule-based processing
  - visualization metadata calculation
- managing versioning, caching, and update propagation

The backend is the single source of truth for all event-related data.

---

### UI (Flutter)

The UI is primarily responsible for **presenting the results produced by the backend**.

Its responsibilities include:
- visualizing the processed event data
- handling user interaction and input
- scrolling, navigation, and presentation logic
- reacting to backend updates and invalidations

The UI does not perform heavy computation and does not own the dataset.

---

## Processing Engine

The backend exposes a **Processing Engine** (or *Engine* for short) through its API.  
The Engine represents the runtime core of the backend.

Conceptually, the Engine consists of the following components:

### Codec
- Encodes and decodes events for efficient storage and transfer
- Optimized for performance and memory usage

### Event Pool (Pool)
- Stores all loaded events, ordered by timestamp
- Acts as the canonical in-memory dataset
- Events may be appended incrementally to the end of the pool, or loaded and merged in bulk

### Processing Pipeline
- A parameterized transformation pipeline applied to the Pool
- Responsible for:
  - filtering
  - rule-based labeling
  - other metadata generation (for future features)

The output of the pipeline is an ordered list of events with visualization metadata.

This output is referred to as the **Visualization List**.

---

## Visualization List

The Visualization List represents the **final, screen-ready result** of the processing pipeline.

It contains:
- references to events from the Pool by index
- computed visualization metadata

The UI consumes this list directly through memory views.

---

## Engine Execution Model

The Engine runs its own **main event loop**.

The UI interacts with the Engine by issuing **commands**, such as:
- importing events from a file
- updating pipeline parameters (e.g. changing filtering or coloring rules)
- Start/stop recieving events in real-time

All commands are handled **asynchronously**.

Command execution may:
- modify runtime data (e.g. Pool contents)
- trigger partial or full recalculation of the pipeline
- result in updated published data

At present, the Engine manages:
- one Codec
- one Pool
- one Pipeline
- one Visualization List

The backend may internally cache intermediate or final results at any stage.

---

## Publish and Notification Model

The Engine uses a **publish–notify mechanism** to communicate changes to interested listeners (primarily the UI).

Whenever relevant data changes, the Engine:
- publishes updated views of affected data structures
- notifies listeners via callbacks

This model is intentionally designed to support:
- UI-driven actions (e.g. file import)
- backend-driven updates (e.g. live event ingestion)

Live event reception does not originate from the UI and is independent of it, hence the publish-notify mechanism.

---

## Versioning

To support caching and incremental updates, the Engine assigns version numbers to its core data structures.

### Codec Version
- Single monotonically increasing version number
- The Codec can only accumulate more encoded data
- It is never cleared or reset during the lifetime of the process

### Pool Version
- Consists of:
  - **Major version**
  - **Minor version**

Rules:
- Minor version increments when new events are appended to the end of the Pool
- Major version increments when:
  - the Pool is cleared
  - events are merged or reordered
  - bulk imports replace existing data

### Visualization List Version
- The Visualization List has its own versioning
- Exact versioning strategy is intentionally left open
- Designed to support future caching and incremental invalidation

---

## Published Objects

When publishing updates, the Engine may publish one or more of the following objects:

- the Pool
- the Codec (required to decode Pool data)
- the Visualization List
- additional metadata (future extensions)

Objects may be published together or independently, depending on what changed.

---

## Data Sharing Model

To minimize overhead:
- the Engine publishes **views** of internal data structures
  - e.g. `string_view`, array views, spans
- data sharing is designed to be **zero-copy** wherever possible

Some data flowing between the UI and the backend (e.g. commands, rule descriptions, progress updates) is transferred using **ProtoBuf** messages.

---

## Summary

The backend owns all data and computation.  
The UI reacts to published results and focuses on presentation.

This architecture is designed to be:
- efficient
- predictable
- extensible
- suitable for both batch and live data workflows