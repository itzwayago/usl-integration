# Contributing

## Before you start

- For a wrong placement, missing record or account problem, use the [USL Discord](https://discord.gg/cgARJXV3AH). The website is the source of truth, and the mod cannot change placements.
- For a crash or a UI bug in the mod, open an issue with your Geode version, platform, the steps to reproduce and the relevant part of `geode/logs/`.

## Pull requests

Keep changes focused. A pull request that fixes one screen is easier to review than one that also renames files and reformats unrelated code.

Before opening a pull request:

1. Build the mod for Windows and at least one Android ABI.
2. Open every screen you touched in GD, including the empty and error states.
3. Check the Geode log for new warnings or errors.
4. Update `changelog.md` if the change is visible to players.

## Code

The mod targets Geode 5.11.2 and GD 2.2081, C++23.

- Use the game's own sprites and fonts before adding a new texture.
- Prefer layouts (`RowLayout`, `ColumnLayout`) over fixed positions when the content can change size.
- Null-check nodes found by ID. Another mod may have removed them.
- Do not block the main thread. Network work goes through `usl::Store`.
- No exceptions. Geode does not support them on every platform.
- Comment the reason for a non-obvious decision, not what the next line does.

## Commits

Short messages in the imperative mood:

```
fix: keep the page buttons inside the list frame
feat: show the list percent on the level card
```

Add a body when the change needs context that the diff cannot show.
