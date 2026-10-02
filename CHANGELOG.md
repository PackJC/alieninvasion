# Changelog

## 1.2.0

Server owners: the generated `alieninvasion-types.xml` gains four items. It is rewritten on the first start after the update; copy it into your mission's `alieninvasion` folder again.

### Added
- **Roswell Leather:** the Roswell Hide tans into green leather with garden lime, by hand or in a barrel (it used to give ordinary Tanned Leather).
- **Roswell Leather Backpack:** two Roswell Leather and a leather sewing kit.
- **Roswell Hide Courier Bag** (hide + rope) and **Roswell Hide Backpack** (courier bag + three sticks).
- Each new bag breaks back down like its vanilla counterpart. Vanilla's own recipes skip the green items, so the action menu never offers a brown result for them.
- New strings translated into all 13 languages.

### Changed
- Area 51 Steak is green at every cooking stage; cooked steak used to show vanilla's beef textures.
- The Roswell Hide looks like the alien's own skin instead of flat neon green.
- Montauk Rifle texture and normal map doubled to 1024×1024.
- The tin foil hat's reflection map is silver; it used to reflect a gold sunset.

## 1.1.0

Server owners: on first start the mod now writes `$profile:Gebs/alieninvasion.json` and ready-made economy files in `$profile:Gebs/mpmissions/`. See the [server guide](https://packjc.github.io/alieninvasion/#install).

### Fixed
- The Montauk Rifle no longer locks up after the last shot in a cartridge. Its script ran the FAL's bolt-lock state machine against the Ruger 10/22 animations its config uses.
- Aliens have voices and footsteps. They were silent and logged a "No sound callback for 'ZombieBase'" warning on every mind-state change.
- Alien attacks no longer land invisible hits from 100 m every 100 seconds. Up close they use normal infected melee; at range they use the new, visible psychic zap.
- Script-spawned aliens are removed with their wreck instead of accumulating for the whole server uptime.
- Vanilla helicopter and sleigh crashes keep their distant crash sound when the mod is loaded.
- The wreck's green light and fire/mist effects are removed when it despawns.
- Aliens spawn on the ground, using the configured 10–15 count.
- Skinning an alien now yields the Roswell Hide (the config referenced a class that didn't exist).
- Tin Foil Hat: shows its name rather than its description, is hat-sized (3×2, 60 g), and no longer references missing damage materials.
- The UFO wreck's materials no longer depend on another mod's Mi-8 textures.
- Plasma shots play one muzzle effect instead of two and no longer log "Plasma_Shot" warnings.
- Config patches no longer reuse a vanilla patch name (`DZ_Sounds_Effects`) and load after every vanilla class they extend.
- Removed the alien preparation recipe, which could never run (recipes can't take a corpse). Skinning gives the steaks.

### Added
- **Psychic zap:** aliens chasing a player in line of sight zap them from 4–80 m, at most once a minute each, with a green burst and crackle.
- **Tin Foil Hat works:** aliens can't track the wearer beyond 3 m and can't zap them.
- **Toxic crash zone:** an 18 m gas cloud (vanilla contaminated-area rules, NBC gear protects) around each wreck.
- **Crash salvage:** 35% chance of a Montauk Rifle with a cartridge, plus up to two partly charged spare cartridges.
- **On-approach spawning:** a wreck's aliens appear when a player comes within 300 m, so unvisited crash sites cost nothing.
- **Admin-spawned wrecks** run the full encounter on dedicated servers.
- **Rechargeable cartridges:** combine with a 9V battery.
- **Green plasma bolts.** Plasma deals double damage to aliens; other firearms deal 75%.
- **Alien death effect:** a green burst and scream.
- **Area 51 Steak effects:** cooked meat gives 90 s of green night vision; raw, burnt, or rotten meat gives food poisoning.
- **Server config** `$profile:Gebs/alieninvasion.json` with every number above.
- **Generated economy files** (types, spawnable types, events, crash positions for any map, wreck loot points, cfgeconomycore block) in `$profile:Gebs/mpmissions/`.
- New strings translated into all 13 languages.

## 1.0

First release: the UFO crash event, Little Green Men, the Montauk Rifle and Cartridge, the Tin Foil Hat, Area 51 Steak, and Roswell Hide.
