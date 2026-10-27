# Changelog

## 2.0.0

Works on DayZ 1.30 (scripts, configs and mission files checked against the 1.30 release; nothing needed changing).

Server owners: the generated economy files are rewritten on the first start after the update. Copy `alieninvasion-types.xml` and `alieninvasion-spawnabletypes.xml` into your mission's `alieninvasion` folder again (they gain `Ammo_Plasma_Cell` and the plasma grenade), and replace the `geb_Aliencrash` group in your mission's `mapgroupproto.xml` with the new one from `alieninvasion-mapgroupproto.xml` (new loot points).

Stored Montauk Rifles: the rifle now takes 8×3 slots instead of 5×3 and has no buttstock wrap, hunting scope or suppressor slot. On the first restart after the update, a rifle saved in a tent, container or backpack where it no longer fits is put on the ground next to it, and any wrap, scope or suppressor fitted to a rifle is put on the ground beside it. Nothing is deleted; the server log notes each one as "dropped ... because of invalid location".

### Added
- **Plasma Grenade** (`geb_PlasmaGrenade`), an alien sticky grenade.
  - A dark violet alloy ball the size of vanilla's M67, with a raised steel band, panel seams, magenta slit lights and cyan pin lights, a recessed disc with a cross of lights, a hub and two small ports. It's held like the M67.
  - Prime it and throw it: it sticks to the first thing it touches. On players, infected and animals it sits on the outside of the body and rides the part it hit; on cars and boats it rides along; anywhere else it stays put. It can't stick to the hand that threw it.
  - Armed, its lights blaze blue-white, plasma crackles round it and it leaves a blue trail in flight.
  - Its prompts say Prime and Disarm instead of Unpin and Pin. As with vanilla's pin, you can disarm it again until it leaves your hand.
  - It bursts 3 seconds after leaving the hand in a blue plasma blast with lightning and white-violet streaks, with plasma motes and a pale blue vapour hanging in the air for a few seconds: a smaller blast than the M67's (4 m against 5) but twice its damage, and whoever it's stuck to takes the full blast.
  - Found at UFO crash sites (up to `CrashPlasmaGrenadesMax` per wreck, default 2, a new setting in `alieninvasion.json`) and sometimes on aliens.
  - Its own explosion sound: an electric crack, a deep whump, a falling plasma sweep and crackling arcs, with close, mid and distant versions that carry like vanilla's grenade.
- **Specimen Tank** (`geb_AlienTank`), a prop for admins and map makers.
  - A sleeping alien floats in a 3.6 m containment tank of glowing green fluid, wired to the lid.
  - The alien hangs in the fluid with its feet off the floor grille, gently bobbing and swaying while its head, arms and legs barely drift. It only moves for players within 40 m.
  - Bubbles rise from the floor grille, green motes drift, and wisps leave the vent valve.
  - It casts a green glow, has a console with a live read-out and a warning plate, and hums.
  - The console screen is animated: a heartbeat trace sweeps across it with one slow beat every 5 seconds, the readings tick, DORMANT and DO NOT OPEN blink, and the activity bars flicker. It only animates for players within 40 m.
  - Nothing spawns it: place it with an admin tool or a map editor.
- **Four plasma colours.** Montauk Cartridges now come in green, blue, lime and violet (`geb_PlasmaCartridge`, `_Arc`, `_Lance`, `_Void`). The loaded cell decides how each shot looks, and a window along each side of the cartridge glows in its colour.
  - Green: a white-hot core, a short jet and green crackle.
  - Blue: lightning flickering on the muzzle and a wide flash.
  - Lime: a two-metre lance, three shock diamonds and a shock ring.
  - Violet: a spinning arc vortex, violet crackle and a wide ring.
  - Every shot fires a plasma ball in the cell's colour that you can watch fly: a white-hot core in a crackling glow half a metre across, with a soft tail that stretches out behind it, 90 m in a second. It's round from every side, and it flies where the round goes: your own ball aims at what your crosshair is on, other players' follow their barrels. It replaces the original plasma laser's streak, a flat electric strip. The round itself still flies at full speed, with a small plasma-ball tracer in place of vanilla's flat tracer strip.
  - A cartridge can't be split any more (R or right-click in the inventory): it used to share its cells out into a second cartridge. Tapping R with the rifle in hand no longer throws the chambered cell on the ground; with an empty chamber it still loads the next cell.
  - The muzzle effect is over in a fraction of a second; only the plasma ball flies on.
  - The muzzle light takes the cell's colour instead of vanilla's orange.
  - Crash sites, aliens and admin spawns hand out a random colour. All four recharge, fit the rifle and hit aliens just as hard.
- **Montauk Rifle rebuilt.**
  - A shaped skeleton stock: the top strut flows out of the back of the receiver and tapers to the butt, with a raised cheek rest; the lower strut sweeps off the back of the grip with a smooth joint; the butt is contoured, with a lightening slot and a sling socket, and a rounded rubber recoil pad that is flush with it. Every edge is rounded or chamfered. The butt sits where the Ruger 10/22's butt is in the hand and shoulder poses the rifle uses. The rifle is 0.96 m long, takes 8×3 slots and weighs 3.4 kg.
  - A real pistol grip: rounded, with a palm swell, finger grooves, a beavertail where the web of the hand sits, a flared base and stippled side and back panels. A proper trigger guard loop with room for a gloved finger, and a trigger blade on one gentle curve with a grooved face, coming out of a slot in a small pinned trigger housing. The grip, trigger and guard sit where the Ruger animation puts the hand and finger.
  - Iron sights machined like real ones: a slim post in a round protective hood with a lime tritium-style dot, and a thin round ghost ring on a tapered post between slim wings, with chamfered edges and screws where they clamp on. Parkerized black with worn edges. Aim with the post's tip in the middle of the ring; the front hood sits as a small circle inside it.
  - The read-out on the back of the receiver is a live screen: a cell gauge, a scrolling plasma waveform, a thermal bar and a blinking READY. It animates while the rifle is in someone's hands or lying near you.
  - The six vent tubes on the shroud are glass with glowing purple gas inside, just like the gas in vanilla's POX grenade: a murky, cloudy gas flecked with bubbles, glowing softly, with brighter wisps drifting the other way over it at the POX gas's own slow pace. Their housings are rebuilt clean: a satin steel flange, collar and bevelled end with a recessed glass end window, the gas running down the middle of each tube.
  - The lime parts flow the same way, in lime green-yellow. The big window on the shroud and the six slots behind it are glass now, with a drum of lime fluid turning under the window and a ring under each slot. The disc where the shroud meets the receiver swirls like a slow vortex. The small bars on the magazine housing and the strip on each side of the stock are sealed glass capsules, held by two clips, with lime gas drifting inside like the POX grenade's vial. Every part moves at its own pace, so none of them pulse together.
  - The purple and lime fluid is a hidden part of its own (`geb_PlasmaRifleFluid`), the way vanilla's Magnum keeps its cylinder: DayZ doesn't animate parts of a weapon's own model, so the fluid stood still until it moved there. Every rifle gets it automatically, rifles already on the server too.
  - New textures, twice the resolution and drawn from the model: a satin olive finish like a Cerakote coat, with a slightly different shade on each panel and sun-faded tops, and dark nitrided steel on the bare-metal parts. Pins where a rifle has them (trigger, hammer, selector, front pivot, rear takedown) and SAFE / FIRE by the selector, paint worn off the most exposed edges and where the hands work, light grime in the corners, a serial plate, caution, warning and cell markings. No UFO stencil or logo. Relief and shine maps to match (satin paint that catches the light, shiny worn metal, dull grime), and the rifle shows wear as it's damaged.
  - New sounds. The shot is an electric crack, a deep thump and a tearing discharge, with close, mid and distant versions (three takes of each), an interior version, and the capacitor whining back up after each shot. It used to be one short clip at every range. Its echo is a plasma echo, not a gunshot's: the zap rings back as falling "pyew" reflections with a crackle of ionised air, dense among trees, a few long far returns across fields, hard slapbacks fluttering between houses and a tight ring indoors, duller and later the further off you are. Every handling sound is the rifle's own, with no gun clicks left: magnetic clamps release the cell, it slides out powering down and venting, slides in, seats with a heavy magnetic chunk, powers up and beeps ready; working the action runs a servo, vents and seals the chamber and draws a charge into it, and the capacitor arms with a chirp; drawing the rifle hums it on; dry fire is a relay clack and a sputter; clearing a jam is straining servos and sputtering arcs.
- **Overheating arcs.** Fire a quick string of shots (six or more, less than 0.6 s apart) and blue-white electric arcs crackle on the muzzle, heavier the longer you keep going. They die off a few seconds after you stop.
- The aliens' psychic zap is a green beam from the alien's head to its target, with a crackle where it hits.
- UFO crash sites have a smoke column rising from a fire on the hull, and electrical sparks crackle from different spots on the hull every few seconds.
  - The fire is real burning fire now (vanilla's oil fire, sized to the hull) instead of a few large flat flame sprites.
  - The smoke is vanilla's heli-crash column thinned to about a quarter of the smoke, which was one of the biggest frame-rate costs near a crash.
  - The green aurora over the wreck starts 12 m up instead of at ground level, with about a quarter of the glow sprites at two-thirds the size. You no longer stand inside some two hundred 30 m sprites at the wreck, the main cause of frame drops near crash sites.
- Aliens bleed green. Bullet and melee hits spray green blood and leave green splatter on the ground. Gutting one leaves green blood on bare hands; gutting anything else turns the blood red again.
- Aliens have a fuller voice. Their two old sounds are cut, pitched and stretched into 75 takes, mixed with vanilla's skinny infected voice. They cover idling, noticing, hunting, chasing, attacking, getting hit, jumping, landing and calling others in, plus five death screams. Every take is levelled like vanilla's infected, and the scream no longer clips.
- **Plasma Cell** (`Ammo_Plasma_Cell`). Clearing a loaded Montauk Rifle's chamber gives the cell back as a loose Plasma Cell, which loads back into any Montauk Cartridge and fires in the colour it was loaded as. It never spawns as loot.
  - The plasma round always named this class for a cleared cell, but it never existed, so the cell was lost with a "cannot store cartridge" error.
  - New strings translated into all 13 languages.

### Fixed
- **Plasma shots show their cell's colour.** The laser every shot fires stayed green whatever cell was loaded: its master colour was green and multiplied every colour it was given. Blue, lime and violet electric effects are also tinted for the pink electric sprite they're drawn with, so they come out the colour they're meant to be (lime was coming out orange).
- **No more cloud of glowing dots from a hot rifle.** Each shot threw sparks, drifting motes and embers that hung in the air for up to two seconds, and once the rifle was hot every shot played the whole burst a second time: vanilla hands the loaded ammo's muzzle effect to the overheating effects too. Rapid fire built a cloud of hundreds of white dots round the muzzle that stayed when the rifle was dropped. The overheating effects now play their own arcs.
- A dying alien no longer looks like it shoots itself. Its death played the zap's plasma laser from its chest, pointing along the world's x axis; it is a green burst with green blood now.
- Montauk Rifle model:
  - The small green windows on the magazine housing were blotchy smears of paint; they are clean vials now (see Added).
  - The stray dash marks painted on the receiver, magwell and handguard are gone.
  - The vent tubes' edges and collars showed smeared strips of the body's paint, one side of each collar had a see-through notch, the end caps showed a dark arrow shape, and the purple fluid sat off-centre and poked out through the glass. All fixed with the rebuilt housings.
  - Hard edges shade hard. Every point's normal was averaged over all the faces round it, 90-degree edges included, so edges shaded as soft blurry gradients and some corners came out dark.
  - The vent tubes were part of the cartridge, so a reload would have pulled them out of the rifle with it.
  - The first-person view showed an older version of the model, with eight vent tubes and a lower carry handle. It is the same rifle as everyone else sees now.
  - Seven stray points removed. It lowers near walls at the Ruger 10/22's length, now that it has the 10/22's length.
- Aliens no longer die in toxic gas. They spawn around their own crash site, inside its cloud, and it slowly killed them (it looked as if their zap killed them). Like vanilla's NBC infected, they ignore gas now.
- **Aliens can be skinned.** The Skin action never appeared: aliens inherited vanilla's rule that infected bodies can't be skinned, so there were no Area 51 Steaks, Roswell Hides or anything made from them. Like an animal, a frozen alien can't be skinned until it thaws. Anything the alien carries is dropped beside it when it's skinned instead of vanishing with the body.
- **Aliens have hit zones again.** Headshots do one and a half times the damage and can kill outright, and arms, legs and feet take their own damage. The Little Green Man's config replaced the whole damage system to change its health and threw the zones away.
- The Roswell Hide's condition follows the alien's torso. It was read from two animal hit zones the alien doesn't have.
- **UFO crash loot lies on the wreck.** 18 new loot points in a ring on the crater around the hull, measured from the UFO model. The old ones were copied from vanilla's Mi-8 wreck, so most were outside the UFO and all of them 1.9 m underground.
- The spare cartridge aliens carry one time in ten now spawns. The wreck created its aliens with their attachments but not their cargo.
- The Montauk Rifle no longer takes the Ruger 10/22's buttstock wrap, hunting scope and suppressor. Its model has nowhere to show them, so they were invisible, and the invisible scope still changed the sights.
- The Montauk Rifle weighs 3.4 kg instead of 350 g. Its collision shape sits on the rifle, stock included, and weighs 3.4 kg: it was two thin boxes off to one side of it, weighing 200 kg, so a dropped rifle sank into the ground.
- Montauk Rifle shots are heard up to 2 km away like other rifles' (they stopped at 1 km).
- The Montauk Cartridge's body has the rifle's material, with its surface detail and shine. Nine faces in ten had no material at all.
- Tin Foil Hat on the ground: its collision shape floated 11 cm above it and weighed 8 kg, its loot box had no width, and its inventory view point was misnamed. All three are rebuilt around the hat.
- **Server crash** whenever a loaded Montauk Rifle had a round in the chamber shown.
  - This crashed admin-tool spawns with ammo (e.g. COT), and could also crash a crash site that rolled a rifle and a restart with a loaded rifle in storage.
  - Cause: the plasma round named no chambered-round class, and DayZ draws the chambered round without checking.
  - Admin debug spawns also give the rifle a plasma cartridge now, instead of trying a 10/22 magazine.
- Aliens cast shadows like the infected. The rebuilt model had shadows switched off on every level of detail.
- UFO crash sites and the Montauk Rifle cast shadows, and both get lighter to draw with distance.
  - DayZ draws a model's shadow from its farthest level of detail and turns shadows off when that level is too detailed. The crash site's six levels were all full-detail copies (32,431 faces, the far five even with the wrong hull and dirt materials), and the rifle had a single level of 11,536 faces.
  - The crash site now steps down to 16,207, 8,102, 3,886, 1,940 and 1,293 faces, with the right materials on every level. The rifle steps down to 12,020, 3,922 and 1,316.
- The UFO hull shows its surface detail. Its normal map shipped with the mod but wasn't used: the hull used the dirt's.
- No more blotchy shading on the UFO hull in sunlight. The hull and the crater's dirt used vanilla's Mi-8 wreck shadow and shine maps, painted for the helicopter's layout, so the Mi-8's doors, windows and rust showed on the UFO as dark smudges and shiny patches. The hull now has a shine map of its own (clean metal shines, soot and rust stay dull) and a proper highlight; the dirt is matte.
- Hitting an alien sounds like hitting an infected. Its hit geometry had no surface material, so hits clanged like metal.
- Alien swings make vanilla's infected swing sounds. Every swing and every grunt used to play the same alien recording.
- Aliens walk with bare-foot steps like the patient infected. The spawnable base alien used to walk silently.
- Aliens move like infected. The model was built in a T-pose and almost every vertex was weighted to the head bone, so the infected animations twisted and dragged the body around. It is now rebuilt on the infected skeleton's own pose, with infected-style weights including the arm and leg twist bones.
- Shots count for the body part they hit (head, neck, torso, arms, legs, feet). The model had no hit zones, so the config's per-zone damage never applied.

- Aliens' resistance to firearms applies before a hit lands, so it also saves them from shots that would otherwise kill. A multiplier of zero makes them immune to firearms, and the plasma multipliers keep the hit zone and the impact speed.

### Changed
- The toxic cloud around a wreck reaches 8 m from it instead of 18 m, with one ring of gas like vanilla's gas grenade instead of three: it covers the wreck and costs far less frame rate. The spooky mist stays.
- The Roswell Hide is the size of a wolf pelt instead of a bear's: 6 kg and 5×3 instead of 15 kg and 8×4, and it tans into up to 4 Roswell Leather by hand or 6 in a barrel instead of 12 and 10. Each crash leaves 10–15 aliens to skin.
- Classes another mod could also use are renamed with the `geb_` prefix: the plasma shot sounds (`geb_Plasma_Shot_SoundSet` and the `geb_Plasma_*_SoundShader`s; `Plasma_SoundSet` before), the distant crash sound (`geb_AlienCrash_Distant*`), the plasma ammo types (`geb_AType_Plasma_Cell*`) and the base alien (`geb_GreenAlienBase`; the old `GreenAlienBase` stays as an alias). Items, ammo and the Little Green Man keep their names, so saved items still load (the Montauk Rifle's new size can move some to the ground: see the note for server owners at the top).
- The repository has its `mod.cpp` (launcher name, logos, version and the Discord link).
- The game no longer ships the two original alien recordings: nothing played them, they were only the source for the voice takes.
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
