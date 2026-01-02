## Backend–UI Architecture Overview

The application is split into two main parts:

- a **C++ backend**, responsible for data handling and computation
- a **Flutter UI**, responsible for visualization and user interaction

The two communicate through a **C-compatible API**, with data exchanged using **Protocol Buffers** and zero-copy views where possible.

---

### Backend (C++)

The backend is responsible for all data- and compute-heavy tasks, including:

- storing large volumes of events in memory-efficient structures
- encoding event data for performance
- applying rule-based filtering and labeling
- search operations (e.g. regex search)
- producing the data needed to visualize events on screen

From the UI’s point of view, the backend exposes a single **Processing Engine**.  
The Engine maintains the currently active dataset and produces a derived, ordered list of events that are ready to be visualized (the *Visualization List*).

The backend runs its own event loop and executes requests from the UI asynchronously.  
Some updates may also originate from the backend itself (e.g. live event ingestion).

---

### UI (Flutter)

The Flutter UI is responsible for:

- rendering a virtualized, scrollable view of the Visualization List
- decorating the visualized events (e.g. coloring, other visual cues)
- handling user interaction (scrolling, filtering, searching, navigation)
- maintaining UI-side caches for smooth scrolling and low latency
- presenting timeline-based visualizations (e.g. heat maps, markers)

The UI does **not** perform heavy computations. Instead, it reacts to updates published by the backend and requests only the data it needs.

---

### Interaction Model

The interaction between UI and backend follows these principles:

- The UI issues **commands** (e.g. change filter, import file)
- Commands are processed asynchronously by the backend
- When results change, the backend **publishes updates** and notifies the UI via callbacks
- Updates may include:
  - a new Visualization List
  - information about which parts of the data became invalid (“dirty”)
  - auxiliary metadata needed for visualization

This publish-based model allows the system to support both batch-based workflows (e.g. file import) and live data sources without coupling backend behavior to UI actions.

---

### Data Exchange

- Backend → UI:
  - zero-copy views (e.g. string views, spans) where possible through C-API
  - serialized batches of visualization data when ownership transfer is required

- UI → Backend:
  - Protocol Buffer messages for commands, filtering rules, and other parameters

This design keeps data transfer efficient and minimal while maintaining a clear ownership boundary between layers.