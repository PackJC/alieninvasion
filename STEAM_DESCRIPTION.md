[img]https://i.imgur.com/3rhti8A.gif[/img]
[url=https://discord.gg/G8uSGZ8yyf][img]https://i.imgur.com/4IyA522.png[/img][/url]

[h1]Mod Information[/h1]
Alien Invasion adds a UFO crash encounter to DayZ. A burning saucer comes down like a helicopter crash, a toxic cloud settles over the wreck, and [b]10-15 Little Green Men[/b] are waiting when you get close. Salvage the Montauk plasma rifle from the wreckage, wear a tin foil hat to keep them out of your head, and skin what you kill for Area 51 Steaks and Roswell Hide.

It adds 1 UFO crash event, 1 creature, 1 plasma rifle, rechargeable plasma cartridges in 4 colours, 1 clothing item, 1 food, the Roswell Hide with 4 items you can make from it, and a specimen tank for admins and map makers. Vanilla helicopter crashes are untouched.

[b]Website (installation guide, searchable classnames and every setting explained):[/b] [url=https://packjc.github.io/alieninvasion/]packjc.github.io/alieninvasion[/url]

[h1]Requirements[/h1]
[list]
[*] DayZ 1.30.
[*] No other mods. Load it on both the server and every client.
[/list]

[h1]Features[/h1]
[list]
[*] [b]UFO crash event[/b] with green light, fire, smoke, mist, electrical sparks crackling from the hull, and a distant crash sound. It spawns through the Central Economy like a helicopter crash.
[*] [b]Toxic crash zone:[/b] an 18 m gas cloud over every wreck, using DayZ's own contaminated-area rules. NBC gear protects you, and the cloud goes with the wreck.
[*] [b]Crash salvage:[/b] a 35% chance of a Montauk Rifle with a cartridge, plus up to two partly charged spare cartridges.
[*] [b]Little Green Men:[/b] 1.9 m tall and built on the infected skeleton, so they move like the infected. They have their own voices and bare-foot steps, claw like infected up close, hit you with a psychic zap from 4-80 m when they can see you, bleed green, and die in a green burst and a scream. Shots count for the body part they hit.
[*] [b]On-approach spawning:[/b] aliens only appear when a player comes within 300 m, so crash sites nobody visits cost your server nothing.
[*] [b]Montauk Rifle:[/b] a semi-automatic rifle firing plasma bolts that do double damage to aliens. Every other firearm does 75%.
[*] [b]Montauk Cartridge:[/b] 15 rounds of plasma that can't be unloaded, in four colours: green, blue, lime and violet. The loaded cell decides how each shot looks, from a white-hot jet to blue lightning, a long lime lance or a violet vortex. Combine it with a 9V battery to recharge it.
[*] [b]Specimen Tank:[/b] a sleeping alien floating in a glowing 3.6 m containment tank, with bubbles, a humming console and a green glow. Nothing spawns it; place it with an admin tool or a map editor.
[*] [b]Tin Foil Hat:[/b] it works. Aliens can't track or zap anyone wearing an intact foil hat unless they're within 3 m.
[*] [b]Area 51 Steak:[/b] cooked steak gives 90 seconds of green night vision. Raw, burnt or rotten steak gives food poisoning.
[*] [b]Roswell Hide crafting:[/b] tan the hide with garden lime into green Roswell Leather and sew a Roswell Leather Backpack, or tie the hide into a Roswell Hide Courier Bag and turn that into a Roswell Hide Backpack.
[*] [b]Configurable:[/b] every number is in $profile:Gebs/alieninvasion.json, in the same Gebs folder Gebsfish uses, and the server writes ready-made economy files on first start.
[/list]

[h1]Classnames, Types, Events, Crash Positions[/h1]
Start the server once and the mod writes everything into $profile:Gebs/mpmissions/:
[list]
[*] [b]alieninvasion-types.xml[/b], [b]alieninvasion-spawnabletypes.xml[/b] and [b]alieninvasion-events.xml[/b]: copy them into mpmissions/<your mission>/alieninvasion/ and paste the block from alieninvasion-cfgeconomycore.xml into your cfgeconomycore.xml.
[*] [b]alieninvasion-cfgeventspawns.xml[/b]: crash positions (79 hand-picked sites on Chernarus; other maps reuse their helicopter crash spots). Merge it into cfgeventspawns.xml.
[*] [b]alieninvasion-mapgroupproto.xml[/b]: the wreck's loot points. Merge it into mapgroupproto.xml.
[/list]
Don't add the old InfectedAlien secondary event; every wreck spawns its own aliens. The files are only rewritten when the mod updates, and your mission folder is never edited for you. Every classname is listed on the website.

[h1]Quick Tuning[/h1]
In alieninvasion.json:
[list]
[*] [b]Alien numbers:[/b] CrashAliensMin and CrashAliensMax set how many aliens each wreck spawns (default 10-15). CrashAliensSpawnDistance is how close a player has to get before they appear (default 300 m); 0 spawns them with the wreck.
[*] [b]Loot and gas:[/b] CrashRifleChance is the chance of a Montauk Rifle in the wreckage (default 0.35) and CrashSpareCartridgesMax the number of spare cartridges (default 2). Set CrashGasZone to false to remove the toxic cloud.
[*] [b]Psychic zap:[/b] PsychicZapRange is how far aliens can zap you (default 80 m) and PsychicZapCooldown the seconds between zaps for each alien (default 60). Set PsychicZapEnabled to false to turn it off.
[/list]

[h1]Supported Languages[/h1]
English, Czech, German, Russian, Polish, Hungarian, Italian, Spanish, French, Chinese (Traditional), Chinese (Simplified), Japanese, Portuguese.

If you're a native speaker and spot a bad translation, please tell us in the Discord.

[h1]The Geb's Family[/h1]
Every mod by Geb stands on its own, so a server can run the ones it wants:
[list]
[*] [url=https://steamcommunity.com/sharedfiles/filedetails/?id=2757509117]Gebsfish[/url]: the fishing overhaul, with 94 catchable species, rods, lures, coolers, the jon boat and more
[*] [url=https://steamcommunity.com/sharedfiles/filedetails/?id=2764114664]Geb's Animals[/url]: 15 wildlife species
[*] [url=https://packjc.github.io/gebsplants/]Geb's Plants[/url]: 27 garden crops and wild ginseng
[*] [url=https://packjc.github.io/gebsmushrooms/]Geb's Mushrooms[/url]: 63 edible and poisonous fungi
[*] [b]Alien Invasion[/b] (this mod): UFO crash sites, Little Green Men and the Montauk plasma rifle
[*] [url=https://steamcommunity.com/sharedfiles/filedetails/?id=2812533524]Great Smoky Mountains[/url]: a 20 x 20 km map of the Smokies at real scale
[/list]

[h1]Contributors & Credits[/h1]
[list]
[*] Geb, creator and author
[*] The DayZ Modding Community <3
[/list]

[h1]Usage & Terms[/h1]
Alien Invasion's source is published on GitHub under the GNU General Public License v3.0: [url=https://github.com/PackJC/alieninvasion]github.com/PackJC/alieninvasion[/url]. You may study, modify, and share it under that license, as long as you keep the license and credit the original.

Please do not put this mod in a "server pack" or "mod pack". Use a Collection if you want to include it on your server, so your players always get the latest version from this page.

[h1]Monetization[/h1]
You are hereby given monetization approval under the conditions that you follow the DayZ Server Monetization agreement and have obtained permission from Bohemia. Read more here https://www.bohemia.net/monetization

[h1]Donations[/h1]
We accept donations at https://www.paypal.com/paypalme/packjc. Every dollar counts and we greatly appreciate any contributions!

Copyright © Smoky Mountain Software 2022-2026
