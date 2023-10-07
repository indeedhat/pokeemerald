# Pokémon Emerald Rom hack
Basic changes for a more interesting nuzlocke

## Added Features
- max item qty increased to 999 in bag
- start with 999 rare candies
- text speed set to fast by default
- options menu is now scrolable with more nuzlocke specific settings
- added option for random encounters
    - on: all encounters will be randomly selected at battle time
    - seeded: all wild encounters will have their pokemon species offset by a seeded random number  
    meaning that each route will still have fixed pokemon avalable but they will be randomly offset from the defaults
- added option for random trainers
    - on: trainers mons will be randomly generated each time you battle them
    - seeded: trainer mons will be generated based on a seed, they will be random to each playthrough but consistent between battles
- added option for random starters
    - this option must be selected from the options menu befor picking your starter mon... obviously
- added option to cap rare candy levels
    - can cap to highest level mon player has defeated
    - can cap to level of next gym/elite 4's ace
- added option to track route encounters
    - When set a the pokeball icon will be displayed on any wild encounter if you have already had your encounter for this route (even if you haven't caught the mon you are battling)  
    Encounters will only be tracked if you haven't already caught the pokemon
- you are forced into the nickname screen on captuning a mon
    - you can still skip this by exiting the screen without setting a name

## Wishlist

## under consideration
- [ ] auto move pokemon to the box when they die
- [ ] add uber repels that last longer
- [ ] limit water pokemon to fishing encounters... maybe

## known issues
- [ ] instant text speed option breaks text engine
- [ ] there are some edge cases where the options meun scroll gets wonky, still works tho

---

This is a decompilation of Pokémon Emerald.
It builds the following ROM:

* [**pokeemerald.gba**](https://datomatic.no-intro.org/index.php?page=show_record&s=23&n=1961) `sha1: f3ae088181bf583e55daf962a92bb46f4f1d07b7`

To set up the repository, see [INSTALL.md](INSTALL.md).


## See also

Other disassembly and/or decompilation projects:
* [**Pokémon Red and Blue**](https://github.com/pret/pokered)
* [**Pokémon Gold and Silver (Space World '97 demo)**](https://github.com/pret/pokegold-spaceworld)
* [**Pokémon Yellow**](https://github.com/pret/pokeyellow)
* [**Pokémon Trading Card Game**](https://github.com/pret/poketcg)
* [**Pokémon Pinball**](https://github.com/pret/pokepinball)
* [**Pokémon Stadium**](https://github.com/pret/pokestadium)
* [**Pokémon Gold and Silver**](https://github.com/pret/pokegold)
* [**Pokémon Crystal**](https://github.com/pret/pokecrystal)
* [**Pokémon Ruby and Sapphire**](https://github.com/pret/pokeruby)
* [**Pokémon Pinball: Ruby & Sapphire**](https://github.com/pret/pokepinballrs)
* [**Pokémon FireRed and LeafGreen**](https://github.com/pret/pokefirered)
* [**Pokémon Mystery Dungeon: Red Rescue Team**](https://github.com/pret/pmd-red)
* [**Pokémon Diamond and Pearl**](https://github.com/pret/pokediamond)
* [**Pokémon Platinum**](https://github.com/pret/pokeplatinum) 
* [**Pokémon HeartGold and SoulSilver**](https://github.com/pret/pokeheartgold)
* [**Pokémon Mystery Dungeon: Explorers of Sky**](https://github.com/pret/pmd-sky)

## Contacts

You can find us on:

* [Discord (PRET, #pokeemerald)](https://discord.gg/d5dubZ3)
* [IRC](https://web.libera.chat/?#pret)
