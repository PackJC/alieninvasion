<h1 align="center">
  <br>
  <a href="https://steamcommunity.com/sharedfiles/filedetails/?id=2840597308"><img src="https://i.imgur.com/p7Fv1Z6.gif" alt="Alien Invasion" width="500"></a>
  <br>
  <a href="https://discord.gg/G8uSGZ8yyf"><img src="https://i.imgur.com/4IyA522.png" alt="Join Our Discord" width="250"></a><img src="https://i.imgur.com/3rhti8A.gif" alt="Like and Favorite" width="500">
  <br>
</h1>

<h3 align="center">An Extraterrestrial Encounter Expansion for DayZ</h3>

<p align="center">
  <img alt="Mod Version" src="https://img.shields.io/badge/Mod-v1.0-blue?style=for-the-badge">
  <img alt="DayZ Version" src="https://img.shields.io/badge/DayZ-v1.29-teal?style=for-the-badge">
  <a href="https://steamcommunity.com/sharedfiles/filedetails/?id=2840597308"><img alt="Workshop Subscribers" src="https://img.shields.io/steam/subscriptions/2840597308?style=for-the-badge&color=purple&label=Workshop%20Subs"></a>
</p>

<p align="center">
  <a href="https://packjc.github.io/alieninvasion/">Website</a> •
  <a href="#information">Information</a> •
  <a href="#key-features">Key Features</a> •
  <a href="#content">Content</a> •
  <a href="#server-setup">Server Setup</a> •
  <a href="#classnames">Classnames</a> •
  <a href="#credits">Credits</a> •
  <a href="#license">License</a>
</p>

## Information

Alien Invasion brings a pulpy science-fiction encounter to DayZ. Server owners can place a burning UFO wreck that fills the surrounding area with hostile little green men. Survivors who make it through the encounter can recover alien materials and experimental plasma weaponry—and, naturally, protect themselves with a tin foil hat.

The mod includes custom models, textures, sounds, particles, localized item names, and Enforce Script behavior. It is designed for both the server and every connecting client.

## Key Features

- A custom UFO crash site with green light, fire, mist, wreck particles, and distant crash audio.
- Hostile **Little Green Men** with custom movement, attack, and impact sounds.
- **Montauk Rifle**, a semi-automatic plasma weapon with custom firing audio and muzzle particles.
- **Montauk Cartridge**, a 15-round plasma magazine that cannot be manually unloaded.
- Harvestable alien remains, including **Area 51 Steak** and **Roswell Hide**.
- A **Tin Foil Hat** with male, female, and ground models.
- A preparation recipe for harvesting alien meat with vanilla cutting tools and GebsFish knives.
- Complete string-table support for English, Czech, German, Russian, Polish, Hungarian, Italian, Spanish, French, Traditional and Simplified Chinese, Japanese, and Portuguese.

## The Encounter

When a `geb_Aliencrash` object is created through DayZ's Central Economy, it:

1. Plays the mod's distant UFO-crash sound.
2. Displays the wreck's custom fire, mist, glow, and particle effects for clients.
3. Spawns Little Green Men around the crash site in a 5–25 meter area.

The wreck is the encounter anchor. Add it to your mission economy or event configuration wherever you want UFO encounters to occur. The repository does not modify a mission's economy files automatically.

Each newly spawned wreck creates a random group of 10–15 aliens.

## Content

### Little Green Man

The alien is based on DayZ's infected behavior and uses its own model and sound set. It can be skinned with a supported knife or cutting tool, producing alien meat and hide alongside vanilla gutting materials.

### Montauk Rifle

The plasma rifle is a semi-automatic firearm derived from the Ruger 10/22 weapon base. It uses the dedicated Montauk Cartridge and fires custom plasma ammunition with a visible shot particle effect.

### Alien Materials

- **Area 51 Steak** inherits DayZ's meat preparation and cooking states.
- **Roswell Hide** is the alien's harvestable pelt.
- Preparing an alien produces one or two steaks and damages the cutting tool used.

### Tin Foil Hat

A lightweight novelty headpiece for survivors who prefer to take no chances with extraterrestrial mind control.

## Requirements

- DayZ
- The mod must be loaded on both the server and client.
- No third-party mod is declared as a runtime dependency.
- **[Subscribe on the Steam Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=2840597308)** to receive the published mod and updates.

## Server Setup

1. Build and sign the mod using your normal DayZ modding workflow.
2. Install the resulting mod folder and key on the server.
3. Add the mod to the server and client launch parameters.
4. Add the classnames you want to your mission economy, trader, event, or admin-tool configuration.
5. Spawn `geb_Aliencrash` through the Central Economy to activate the complete crash encounter.

This repository contains the source assets and scripts. It does not include ready-made mission `types.xml`, `events.xml`, or `cfgeventspawns.xml` entries because spawn balance and placement are server-specific.

## Classnames

| Classname | Type | Description |
| --- | --- | --- |
| `geb_Aliencrash` | Encounter object | UFO wreck with effects, audio, and alien spawning behavior |
| `StaticObj_geb_Aliencrash` | Static object | Visual UFO wreck without the scripted encounter behavior |
| `geb_GreenAlien` | Creature | Hostile Little Green Man |
| `geb_GreenAlienMeat` | Food | Area 51 Steak harvested from an alien |
| `geb_GreenAlienSkin` | Material | Roswell Hide harvested from an alien |
| `geb_PlasmaRifle` | Weapon | Semi-automatic Montauk Rifle |
| `geb_PlasmaCartridge` | Magazine | 15-round Montauk Cartridge |
| `geb_FoilHat` | Clothing | Tin Foil Hat |

## Project Structure

```text
alieninvasion/
|-- config.cpp          # Mod registration and script-module definitions
|-- data/               # Entity configs, models, textures, and sounds
|-- graphics/particles/ # Plasma and UFO particle effects
|-- languagecore/       # Localized display names and descriptions
`-- scripts/            # Encounter, recipe, weapon, and particle behavior
```

## Credits

- Geb — creator and author
- The DayZ Modding Community

Contributions and bug reports are welcome through this repository's issues and pull requests.

## License

Alien Invasion is distributed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.html). You may use, study, modify, and redistribute the source under the terms of that license.

DayZ and Bohemia Interactive are trademarks or registered trademarks of Bohemia Interactive. This community project is not affiliated with or endorsed by Bohemia Interactive.
