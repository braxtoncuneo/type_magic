# Type Magic 🪄

This is an experimental repo for a static plugin system. This system consists of three main categories of abstraction:

 - Traits: representing an abstract data type (an interface with specific behavior)
 - Components: implementations of traits
 - Modules: representing a family of zero or more components.

All three are represented through types.

A trait is effectively a label identifying whatever ADT developers ascribe to it. Something as simple as the following can be a trait:

```cpp
struct HasAName {};
```

Let's say that `HasAName` represents types which have a method called `name` which return a `std::string`. The following types would be components implementing `HasAName`:

```cpp
struct Michael {
  std::string name() {
    return "Mike";
  }
};


struct Robert {
  std::string name() {
    return "Bobby";
  }
};
```

Components are organized through modules, which define an `ImplFor` template, which accepts a single type (a trait) and returns a `TypeMap` specialization as an inner type called `type`.
This type defines a mapping where each key is a component implementing the queried trait and the value is a `TypeSet` specialization representing the set of all traits that are required by this component.

The following is a module that contains only `Michael`, which requires nothing to implement `HasAName`L

```cpp
struct MichaelModule {

  template<typename T>
  struct ImplFor{
    typedef TypeMap<> type;
  };

  template<>
  struct ImplFor {
    typedef TypeMap<Binding<Michael,TypeSet<>>> type;
  };

};
```

With these basic building blocks, a template solver can take a `TypeSet` of traits required for a type, query a module for implementation of those traits, and attempt to construct a type matching the corresponding ADTs.





-------------
This work was supported by the Center for Advancing the Radiation Resilience of Electronics (CARRE), a PSAAP-IV project funded by the Department of Energy, grant number: DE-NA0004268.


