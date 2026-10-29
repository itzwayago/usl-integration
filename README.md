<p align="center">
  <img src="logo.png" width="150" alt="USL Integration logo">
</p>

<h1 align="center">USL Integration</h1>

<p align="center">
  <a href="https://geode-sdk.org"><img src="https://img.shields.io/badge/Geode-5.11.2-5967c9" alt="Geode 5.11.2"></a>
  <a href="https://geometrydash.com"><img src="https://img.shields.io/badge/GD-2.2081-00aff0" alt="Geometry Dash 2.2081"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-green" alt="MIT license"></a>
</p>

<p align="center">The <a href="https://ultimateshittylist.fun">Ultimate Shitty List</a> inside Geometry Dash.</p>

The mod brings all five USL rankings, placements, rates, player leaderboards and recent changes into the game, using the same level cells and menus players already know.

## Features

- **Five lists**: Classic, Challenge, SCL, Controlled and Unlimited, each with its own placements, points and players.
- **Placements on level cells** and on level pages, including unrated levels.
- **USL rates**: the demon face and the Featured, Epic, Legendary or Mythic glow chosen by the USL team.
- **Level card** with rate, points, list percent and every record with its video.
- **Players**: the top 100 of each list with records, verified levels and hardest level.
- **Recent changes** for each list.
- **Search and filters** by name, creator, verifier, placement or level ID, plus Main, Extended and Advanced categories.
- **Settings** to turn placements, points or USL rates off independently.

## Installation

1. Install [Geode](https://geode-sdk.org) for Geometry Dash 2.2081.
2. Install the mod from the Geode index, or drop the `.geode` file from [Releases](https://github.com/itzwayago/usl-integration/releases) into your Geode `mods` folder.

The mod only reads public list data from `ultimateshittylist.fun`. It does not send your account or level data anywhere.

## Usage

Open the USL hub with the round logo button on the level search screen. From there you can open any list, the player leaderboard, recent changes or the website. Arrow keys and the on-screen arrows move between pages and lists.

## Building

You need the [Geode SDK](https://docs.geode-sdk.org/getting-started/) and the Geode CLI.

```sh
git clone https://github.com/itzwayago/usl-integration
cd usl-integration
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Set `GEODE_SDK` if the SDK is not next to the project. The GitHub workflow builds Windows, Android 64 and Android 32 with the same commands through `geode-sdk/build-geode-mod`.

## Project layout

```
src/
  USL.cpp/.hpp        API client, cache, list metadata
  UI.cpp/.hpp         shared GD-style widgets
  Rate.cpp/.hpp       USL rates applied to difficulty sprites
  hooks/              LevelCell, LevelInfoLayer, LevelSearchLayer
  layers/             hub and list browser
  popups/             level card, players, changes
resources/            sprites and list icons
```

## Data

All data comes from the public USL API:

- `/levels`
- `/levels/:slug`
- `/leaderboards/players`
- `/changes`
- `/stats`
- `/community/levels` for USL rates

Responses are cached in memory: lists for 30 minutes, the smaller feeds for 2 minutes. A failed request is not retried for 15 seconds, so a bad connection does not turn every level cell into a request.

## Contributing

Bug reports and placement corrections are welcome in [Issues](https://github.com/itzwayago/usl-integration/issues). Wrong placements are usually fixed faster in the [USL Discord](https://discord.gg/cgARJXV3AH).

## Credits

- [Ultimate Shitty List](https://ultimateshittylist.fun) for the rankings and the API.
- [Geode SDK](https://geode-sdk.org) for the mod loader and UI helpers.

## License

MIT. See [LICENSE](LICENSE).
