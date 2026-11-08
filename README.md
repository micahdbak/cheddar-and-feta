# Cheddar and Feta

![Cheddar and Feta dancing][dance]

Cheddar and Feta is a 2D RPG and corresponding engine built using [SDL3][sdl]
for cross-platform windowing and graphics, and [libdatachannel][datachannel] for
WebRTC connectivity.

The game builds as one `cnf` executable. At launch, the player chooses to host
or join a session. Host game code lives in `src/`, guest client code in
`src/guest/`, and shared HUD and asset code in `src/shared/`. The engine lives
in `thoom/`.

## Copyright and License

Copyright (c) 2026 Micah Baker

This project is licensed under the terms of the GPL-3.0 license. See the
[LICENSE][license] file for full license details.

[sdl]: https://libsdl.org/
[datachannel]: https://libdatachannel.org/
[license]: LICENSE
[dance]: steam_assets/dance.gif
