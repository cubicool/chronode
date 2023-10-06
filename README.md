# Chronode
A modern, header-only, in-source C++ profiling library. The name is
derived from the fact that timing/measurement information is stored
in a graph structure, where each timer (Node) is aware of it's own
absolute time and the time relative to a parent.

# TODO

- Add "count" to chronode::Node, for each time it's start/stopped.
- Implement better error handling/checking (prevent "reporting" when
  any Node isn't properly stopped, etc).
- Currently, the Timer object requires a final, weird stop(), which is
  confusing.
