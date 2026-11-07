# NEO-MAXIS
### The Living City Project

An experimental, modular simulation overhaul for SimCity 2000 Special Edition.

NEO-MAXIS aims to expand the original city-building experience through deeper
simulation systems, interconnected economic models, population dynamics,
infrastructure, environmental processes, and community-created extensions.

The project is being developed with a long-term goal: create an expandable
simulation ecosystem that other developers and players can build upon.

## Philosophy

NEO-MAXIS draws conceptual inspiration from:

- **Hermeticism:** interconnected systems and correspondence.
- **Alchemy:** transformation, development, and resource processes.
- **Sacred geometry:** proportion, spatial organization, and relationships.
- **Cycles and duality:** growth, decay, stability, and disruption.
- **Emergence:** complex behavior arising from simple interacting rules.

These ideas influence the project's simulation design and identity while
the implementation remains modular, readable, and testable.

## Project Status

NEO-MAXIS is under active development.

Current areas of work include:

- Game integration through sc2kfix.
- City session and game-state management.
- Calendar and simulation hooks.
- Economic data structures and analysis.
- Budget and bond systems.
- Automated tests.
- Windows and Linux development workflows.

Features and compatibility are still evolving. This is not yet a finished
public release.

## Architecture

The project is organized around independently maintainable components:

- Core: shared state, city sessions, and foundational services.
- Hooks: integration with the original game.
- Systems: independent simulation domains.
- Tests: isolated verification of project behavior.
- Platform tooling: build and development support.

See [Architecture](docs/ARCHITECTURE.md).

## Platforms

- Windows: intended player-facing platform and game integration target.
- Linux: development, testing, and cross-platform engineering environment.

Windows and Linux build support will be maintained as part of the project.

## Dependencies

NEO-MAXIS integrates with sc2kfix. Its SDK and runtime requirements must be
obtained and used according to the applicable sc2kfix licensing terms.

The original SimCity 2000 game is not included in this repository.
Users must provide their own legally obtained game installation.

## Development

See:

- [Building](docs/BUILDING.md)
- [Roadmap](docs/ROADMAP.md)
- [Modding API](docs/MODDING_API.md)
- [Contributing](../CONTRIBUTING.md)

Build instructions will be finalized as the Windows and Linux build
environments are made reproducible.

## Community

The long-term goal is to support community-created simulation systems,
content, and extensions through documented interfaces.

Contributions, testing, technical discussion, and documentation improvements
will help shape the project.

## License

Project licensing is being evaluated. No blanket license is granted by this
README. Third-party components remain subject to their respective licenses.
