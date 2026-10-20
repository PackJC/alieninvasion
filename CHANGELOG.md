# Changelog

## Unreleased

Works on DayZ 1.30 (scripts, configs and mission files checked against the 1.30 release; nothing needed changing).

### Added
- **Specimen Tank** (`geb_AlienTank`), a prop for admins and map makers.
  - A sleeping alien floats in a 3.6 m containment tank of glowing green fluid, wired to the lid.
  - Bubbles rise from the floor grille, green motes drift, and wisps leave the vent valve.
  - It casts a green glow, has a console with a live read-out and a warning plate, and hums.
  - Nothing spawns it: place it with an admin tool or a map editor.
- **Four plasma colours.** Montauk Cartridges now come in green, blue, lime and violet (`geb_PlasmaCartridge`, `_Arc`, `_Lance`, `_Void`). The loaded cell decides how each shot looks, and a window along each side of the cartridge glows in its colour.
  - Green: a white-hot core, a short jet, crackle, streak sparks and ions that hang in the air.
  - Blue: lightning flickering on the muzzle, falling sparks that bounce, and static zaps after the shot, with a blue bolt.
  - Lime: a two-metre lance, three shock diamonds, a shock ring and dripping embers, with a yellow bolt.
  - Violet: a spinning arc vortex in a dark puff, a wide ring and floating sparks, with a violet bolt.
  - The muzzle light takes the cell's colour instead of vanilla's orange.
  - Crash sites, aliens and admin spawns hand out a random colour. All four recharge, fit the rifle and hit aliens just as hard.
- UFO crash sites smoke like a heli crash, and electrical sparks crackle from different spots on the hull every few seconds.
- Aliens bleed green. Bullet and melee hits spray green blood and leave green splatter on the ground. Gutting one leaves green blood on bare hands; gutting anything else turns the blood red again.
- Aliens have a fuller voice. Their two old sounds are kept and cut, pitched and stretched into 75 takes, mixed with vanilla's skinny infected voice. They cover idling, noticing, hunting, chasing, attacking, getting hit, jumping, landing and calling others in, plus five death screams. Every take is levelled like vanilla's infected, and the scream no longer clips.

### Fixed
- **Server crash** whenever a loaded Montauk Rifle had a round in the chamber shown.
  - This crashed admin-tool spawns with ammo (e.g. COT), and could also crash a crash site that rolled a rifle and a restart with a loaded rifle in storage.
  - Cause: the plasma round named no chambered-round class, and DayZ draws the chambered round without checking.
  - Admin debug spawns also give the rifle a plasma cartridge now, instead of trying a 10/22 magazine.
- Aliens cast shadows like the infected. The rebuilt model had shadows switched off on every level of detail.
- UFO crash sites and the Montauk Rifle cast shadows, and both get lighter to draw with distance.
  - DayZ draws a model's shadow from its farthest level of detail and turns shadows off when that level is too detailed. The crash site's six levels were all full-detail copies (32,431 faces, the far five even with the wrong hull and dirt materials), and the rifle had a single level of 11,536 faces.
  - The crash site now steps down to 16,207, 8,102, 3,886, 1,940 and 1,293 faces, with the right materials on every level. The rifle steps down to 5,768, 2,307 and 1,099. Its first-person model is unchanged.
- The UFO hull and the Montauk Rifle show their surface detail. Their normal maps shipped with the mod but weren't used: the hull used the dirt's, and the rifle used a flat one.
- Hitting an alien sounds like hitting an infected. Its hit geometry had no surface material, so hits clanged like metal.
- Alien swings make vanilla's infected swing sounds. Every swing and every grunt used to play the same alien recording.
- Aliens walk with bare-foot steps like the patient infected. The spawnable base alien used to walk silently.
- Aliens move like infected. The model was built in a T-pose and almost every vertex was weighted to the head bone, so the infected animations twisted and dragged the body around. It is now rebuilt on the infected skeleton's own pose, with infected-style weights including the arm and leg twist bones.
- Shots count for the body part they hit (head, neck, torso, arms, legs, feet). The model had no hit zones, so the config's per-zone damage never applied.

### Changed
- Aliens stand about 1.9 m tall instead of 1.6 m: their joints now sit where the infected skeleton's are. Head, hands and feet keep their size.
- The alien model has four levels of detail instead of one.
- New mod logo, in the same style as Geb's other mods: a flying saucer with a see-through tractor beam, white in a black frame normally and alien lime when hovered. The hovered logo now actually shows; the config set `logoHover`, which DayZ doesn't read (it reads `logoOver`). The website's tab icon is the saucer too.

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
