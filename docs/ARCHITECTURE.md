# NEO-MAXIS Architecture

## 1. Design Objective

NEO-MAXIS is intended to be a modular simulation overhaul for SimCity 2000
Special Edition.

The architecture should allow individual simulation domains to evolve
independently while communicating through clearly defined interfaces.

The project should avoid unnecessary coupling between game integration,
simulation logic, data management, and platform-specific implementation.

## 2. Architectural Principles

### Modularity
Each subsystem should have a distinct responsibility and a defined interface.

### Testability
Simulation logic should be testable independently of the original game
whenever practical.

### Portability
Platform-specific functionality should be isolated behind adapters.

### Extensibility
New systems should be addable without requiring unrelated systems to be
rewritten.

### Maintainability
Prefer readable, explicit code and small independently verifiable milestones.

## 3. Conceptual Layers

### Core
Foundational services shared by other systems:
- City session management
- Game state
- Memory access abstractions
- Shared data and lifecycle management

### Game Integration
Interfaces connecting NEO-MAXIS to the original game:
- sc2kfix hooks
- Lifecycle callbacks
- Save/load integration
- Game-specific adapters

### Simulation Systems
Independent domains that can exchange data through documented interfaces:
- Economy
- Population
- Infrastructure
- Environment
- Industry
- Government
- Disasters
- Calendar and time

### Content and Extensions
Future expansion areas:
- Buildings
- Terrain and visual assets
- Scenarios
- Simulation extensions
- Community-created modules

### Platform Layer
Platform-specific build and runtime support:
- Windows
- Linux development and testing

## 4. Design Philosophy

The project's conceptual identity draws on several esoteric traditions.

Hermetic correspondence inspires relationships between simulation domains.
Alchemy inspires transformation and resource flows.
Sacred geometry inspires spatial and proportional relationships.
Cycles inspire time-dependent processes, growth, and decay.
Emergence inspires systems whose interactions produce complex outcomes.

These are design inspirations, not substitutes for measurable simulation
rules or engineering discipline.

## 5. Dependency Direction

The intended dependency direction is:

Core services -> Simulation interfaces -> Individual systems

Game-specific integration should connect the original game to the shared
core through adapters rather than becoming the foundation of every system.

## 6. Current Implementation

Existing source areas include:
- Core city session and game state.
- Calendar integration.
- Economy and financial data structures.
- Budget and bond analysis.
- sc2kfix callback integration.

The conceptual architecture is a direction for development, not a claim
that every planned subsystem or interface already exists.
