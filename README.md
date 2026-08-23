# Sköll

Participant-side engine to plug into an existing project of mine 
[Valkyrie](https://github.com/Semilore317/valkyrie-dotnet).

The setup is simple, Valkyrie is an exchange that has different configurable modes...

It can work with other participants in the system (which might be other instances of Sköll runnig) and test different trading strategies

Sköll is written in C++20 and currently implements a simple market-making strategy;
buy at best bid, sell at best ask.

## Features
* live market data ingestion over websockets
* market data capture in JSONL
* order execution over REST
* replay of captured market data, providing a base for future backtesting
* local order book replica
* position and P&L tracking

## Getting Started
Requirements:

* C++20
* CMake
* vcpkg
* Ninja or another supported CMake generator

The project uses CMake and vcpkg for source builds.

## Configuration

Copy the `.env.example` to `.env` and edit as needed.

## Running

Start Valkyrie in simulated-market mode:

```powershell
dotnet run --project .\Valkyrie --launch-profile simulated-market
```

Then run Sköll:

```powershell
.\skoll.exe
```

Example live session:

```text
Sköll v0.1
connected to market data feed
placed order 5349 buy 1 @ 3120
placed order 5350 sell 1 @ 3135
position=1 cash=-3120 pnl=7
placed order 5357 buy 1 @ 3119
placed order 5363 sell 1 @ 3120
position=0 cash=0 pnl=0
```

## Current limitations

The maker strategy uses a fixed quantity and quotes the current best bid and ask. It does not yet include advanced inventory controls, stale-order cancellation, or multiple strategies.