# Cache Simulator Project

![Python 3.6+](https://img.shields.io/badge/python-3.6+-blue.svg)
![Status: Complete](https://img.shields.io/badge/status-complete-green.svg)

A comprehensive simulator for analyzing cache performance across different memory access patterns and cache configurations.

## Overview

This project implements a customizable cache simulator that models an n-way set-associative cache with variable configurations. The simulator evaluates cache performance across different memory access patterns to understand the impact of line size and associativity on hit ratios. Six distinct memory reference generators are used to simulate various access patterns, demonstrating how cache behavior changes with different workloads.

## Features

- **Configurable Cache Parameters**:
  - Cache size: 64 KB (fixed)
  - Line size: 16, 32, 64, and 128 bytes
  - Associativity: 1-way (direct mapped), 2-way, 4-way, 8-way, and 16-way
  - LRU (Least Recently Used) replacement policy

- **Memory Access Patterns**:
  - **memGen1**: Sequential access through the entire memory space
  - **memGen2**: Random access within a 24 KB region
  - **memGen3**: Uniform random access across the entire 64 MB memory
  - **memGen4**: Sequential access within a 4 KB region
  - **memGen5**: Sequential access within a 64 KB region (equal to cache size)
  - **memGen6**: Strided access pattern with 32-byte increments

- **Performance Metrics**:
  - Hit ratio
  - Miss ratio
  - Comparative analysis across configurations

## Technical Details

The simulator is built with the following components:

- **CacheLine**: Represents a single cache line with valid bit, tag, and last-used timestamp
- **Cache**: Models an n-way set-associative cache with LRU replacement policy
- **MemoryGenerators**: Implements six different memory access patterns
- **Experiments**: Controls the execution of simulations with different cache configurations

