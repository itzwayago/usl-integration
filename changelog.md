# Changelog

## v1.2.0

- Fixed a crash when opening a list page. Newer bindings renumbered `Level` onto `Achievement`, so the game tried to build achievement cells from level data. The list now passes the value the 5.11.2 loader uses.

## v1.1.0

- USL rates: levels on the list show the demon difficulty and the Featured, Epic, Legendary or Mythic glow the USL team gave them. Can be turned off in the settings.
- Spam is now three lists, SCL, Controlled and Unlimited, each with its own placements, players and changes. Before, only SCL levels were shown.
- The level card shows the USL rate next to the placement.
- Screens redone in the style of the game's own menus: stock buttons and backgrounds, no custom cards. Every screen fits 4:3 and 16:9.
- Level pages say which demon it is (Easy, Medium, Hard, Insane or Extreme Demon), like rated demons in the game.
- Players and Changes switch between the five lists with arrows.
- Every list explains its own rules behind the info button.
- Icons stay sharp at every size: no more broken outlines on small badges.
- New USL logo, the same one the website uses.
- Failed API requests wait 15 seconds before retrying, instead of firing on every level cell.
- Published by wayago under the id wayago.usl-integration. Settings start fresh.

## v1.0.4

- USL logo badge is centred with even padding, nothing touches the rim.

## v1.0.3

- The USL button is now the full original logo in a round frame.
- The placement plate sits centred above the difficulty face.
- Level pages of unrated levels (common on the Spam and Challenge lists) now show the placement too.
- Cleaner website icon.

## v1.0.2

- Original round USL badge as the button on the level search screen.
- All icons are drawn at full resolution, nothing is upscaled anymore.
- Placements show as a dark plate with the list icon and a white number in the top-left corner of level cells.
- Rounded panels keep clean corners at any size.
- Hub cards with a coloured rim and dark body, so text never blends into the list colour.

## v1.0.1

- New USL button icon that fits the round button.
- Placements no longer show as "No1": they read as the list icon with the number, or "Top 1".
- Search waits until you stop typing, understands "#12" and level IDs, and no longer overlaps the list frame.
- List tabs use the same text size.
- Android support (64-bit and 32-bit).

## v1.0.0

- USL hub with the Classic, Challenge and Spam lists.
- Lists with real level cells, search, category filter and pages.
- Placements on level cells and level pages, level card with records.
- Players leaderboard and recent changes.
