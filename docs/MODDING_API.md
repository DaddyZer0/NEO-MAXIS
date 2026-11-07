# NEO-MAXIS Modding API

## Status

This document describes the intended direction of the extension system.
It is not a finalized or stable public API specification.

## Goals

The future NEO-MAXIS extension interface should allow independent creators
to add functionality without modifying the core project.

Potential extension categories include:

- Simulation rules
- Economic models
- Population systems
- Infrastructure behavior
- Environmental processes
- Buildings and visual content
- Scenarios and events

## Intended Capabilities

The extension architecture should eventually provide:

- Module identification and versioning.
- Dependency declarations.
- Compatibility checks.
- Defined initialization and shutdown stages.
- Documented event interfaces.
- Controlled access to simulation data.
- Error isolation and diagnostics.

## Compatibility

Public extensions should declare the API version they require.

Changes to public interfaces should be documented. Breaking changes should
use explicit version transitions and migration guidance.

## Safety and Reliability

Extensions should not be allowed to corrupt shared simulation state through
undocumented access patterns.

The implementation strategy, loading model, and security boundaries remain
under design.

## Current Status

The existing sc2kfix integration is an early game-integration mechanism.
It should not be confused with a completed general-purpose NEO-MAXIS
modding API.
