# Chronode

A modern, header-only, in-source C++17 profiling library. The name is
derived from the fact that timing/measurement information is stored
in a graph structure, where each timer (Node) is aware of it's own
absolute time and the time relative to a parent.

# Example (Pure C++)

```c++
// Soon!
```

# Example (C++ & Macros)

```c++
// Soon!
```

# TODO

- Add "count" to chronode::Node, for each time it's start/stopped.
- Implement better error handling/checking (prevent "reporting" when
  any Node isn't properly stopped, etc).
- Currently, the Timer object requires a final, weird stop(), which is
  confusing.
- Some kind of vector-based (SVG, HTML5 Canvas, Cairo) reporting display.
- Introduce a "Profile" class that will call the `copy()` method of a Node and
  manage some variable number of measurements, allowing both individual report
  inspection and amalgamated/averaged reports. This will need to be handled by
  the reporting functions explicitly so that any kind of "visualization" app
  can properly manage the data.
- Timer should probably remove the `node()` method, adding Node/Timer
  overloading when necessary. Furthermore, AT LEAST a `reset()` will need to be
  added (since we can't call `node().reset()` anymore).
