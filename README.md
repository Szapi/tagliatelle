# tagliatelle

# Event Visualizer Blueprint

This repository is a **blueprint for building a desktop software event visualizer**.

The goal is to provide a reusable foundation for applications that need to **collect, process, and visually explore large volumes of time-ordered events** coming from one or more sources.

The project is intentionally structured as a split system:
- a **C++ backend** that handles data storage and heavy computation
- a **Flutter-based frontend** that provides a responsive, interactive UI

---

## What Is an “Event”?

An event is defined by an **event class**: a structured set of attributes describing something that happened at a specific point in time.

Typical attributes include (but are not limited to):
- timestamp
- host or source identifier
- severity / level
- message text
- flags or custom metadata

The exact event schema is configurable and intended to be adapted to different domains.

---

## What This Software Does

Based on an event class definition, the codebase can be built into:

### Backend (C++)
A backend library (DLL) responsible for:
- storing large event datasets
- searching and filtering (e.g. regex-based)
- rule-based coloring and metadata computation
- efficient paging and caching
- optional live ingestion or batch import

### Frontend (Flutter)
A Flutter desktop UI responsible for:
- visualizing events in a scrollable, virtualized list
- applying filters and coloring rules
- navigating events along a timeline
- keeping the user’s temporal context stable while exploring data

The UI and backend communicate through a C-compatible API using **ProtoBuf** for data exchange.

---

## Typical Use Cases

This architecture is especially useful when events originate from **multiple simultaneous sources**, for example:

- system or application logs from many devices on a network
- trace events from multiple running processes
- traces produced by a multithreaded application
- mixed live and imported diagnostic data

The focus is on helping humans **see patterns over time**, identify active vs. idle periods, and understand how events from different sources relate to each other.

---

## What This Repo Is (and Isn’t)

**This repo is:**
- a reference architecture
- a practical starting point for real implementations
- focused on performance, scalability, and UX consistency

**This repo is not:**
- a polished end-user product
- tied to a specific logging or tracing format
- limited to a single domain

---

## Project Status

This project is under active design and development.  
Architecture decisions are documented in the `/docs` folder and may evolve as implementation progresses.
