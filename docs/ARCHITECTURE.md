# Architecture

The mod has three layers: a cache that talks to the USL API, screens built from the game's own controls, and hooks that put USL data on existing GD screens.

## Data

`usl::Store` in `src/USL.cpp` owns every request.

- `loadLevels` fetches one list. The five lists are fetched separately, so opening Spam does not wait for Classic.
- `loadRates` fetches `/community/levels` once and keeps `slug -> difficulty, quality` in a map. Rates are applied to every list, including lists loaded earlier.
- `loadPlayers`, `loadChanges`, `loadStats` and `loadLevel` cover the smaller screens.
- Lists stay fresh for 30 minutes. Players, changes and stats stay fresh for 2 minutes.

Every slot in the cache tracks `loading`, `loadedAt` and `failedAt`. A request that is already running is shared: callers append a callback and wait. A failed request blocks new attempts for 15 seconds, which keeps a dead connection from firing one request per level cell. Forced refreshes (the refresh button) ignore that cooldown.

Requests run through `arc::Future` and `web::WebRequest`, then return to the main thread with `queueInMainThread`. Callbacks must not touch the cache from the network thread.

`Store::byGdId` maps a Geometry Dash level ID to its USL entry. When a level is on more than one list, the first list in `LIST_TYPES` wins (Classic, then Challenge, then the spam lists). The pointer is valid until the next list refresh; screens copy the entry before keeping it.

## Screens

- `USLHubLayer` is the entry point. It reads counts and stats from the store and opens the other screens.
- `USLListLayer` renders a list with `GJListLayer` and real GD level cells. It requests levels by ID from the GD servers, then restores USL order before showing them.
- `USLListPopup` is the common base for the players and changes popups. It owns the scroll layer, list switcher and loading state.
- `USLLevelPopup` is the level card. It loads records on demand and opens videos and the website in the browser.

`src/UI.cpp` holds the shared widgets: the placement plate, the rate face, the GD background and the list switcher. Colours and names for the lists live in one table in `USL.cpp`, so a new list only changes one place.

## Hooks

- `LevelSearchLayer` adds the USL button to the search screen.
- `LevelCell` adds a placement plate to normal and compact cells.
- `LevelInfoLayer` adds the placement row and applies the USL rate to the difficulty face. The face is re-applied after `updateLabelValues`, `levelDownloadFinished` and `levelUpdateFinished`, because the game redraws it from the level's real rating.

Hooks check for their own node IDs before adding anything. `LevelInfoLayer::init` can run more than once for the same level, and a missing node means another mod changed the layout; the hook then leaves the screen alone instead of crashing.

## Settings

`show-placements`, `show-points` and `fake-rate` are read where they are used, through `usl::settings` and `usl::rate`. Turning a setting off only stops the mod from drawing or replacing something; the saved level is never modified.
