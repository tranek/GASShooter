# GASShooter

## Introduction

GASShooter is an advanced FPS/TPS Sample Project for Unreal Engine 5's GameplayAbilitySystem (GAS) plugin. This is a sister project to the [GASDocumentation](https://github.com/tranek/GASDocumentation) and information about the techniques demonstrated here will be discussed in detail in the README there.

This is not production-ready code but a starting point for evaluating different techniques in GAS relating to using weapons. TargetActors with persistent hit results and ReticleActors particularly do a lot of code on `Tick()`.

Assets included come from Epic Games' ShooterGame learning project, Epic Games' Infinity Blade assets, or made by myself.

GASShooter is current with **Unreal Engine 5.0**. There are branches of this Sample Project for older versions of Unreal Engine, but they are no longer supported and are liable to have bugs or out of date information.

| Keybind             | Action                                                      |
| ------------------- | ----------------------------------------------------------- |
| T                   | Toggles between first and third person.                     |
| Left Mouse Button   | Activates the weapon's primary ability. Confirms targeting. |
| Middle Mouse Button | Activates the weapon's alternate ability.                   |
| Right Mouse Button  | Activates the weapon's secondary ability.                   |
| Mouse Wheel Up      | Swaps to next weapon in inventory.                          |
| Mouse Wheel Down    | Swaps to previous weapon in inventory.                      |
| R                   | Reloads the weapon.                                         |
| Left Ctrl           | Cancels targeting.                                          |
| Left Shift          | Sprint.                                                     |
| E                   | Interact with interactable objects.                         |

| Console Command | Action                  |
| --------------- | ----------------------- |
| `kill`          | Kills the local player. |

The Hero character does have mana but no abilities currently use it. This project's inception started when the new BioShock was announced and the idea was to include BioShock-like upgradeable abilities. That made the scope too large, but it is something that may be revisited in the future.

Secondary ammo is not used. It would be used for things like rifle grenades.

## Concepts covered

* [Ability Batching](https://github.com/tranek/GASDocumentation#concepts-ga-batching)
* Equippable weapons that grant abilities
* Predicting weapon switching
* [Weapon ammo](https://github.com/tranek/GASDocumentation#concepts-as-design-itemattributes)
* Simple weapon inventory
* Headshot bonus damage
* [Reusable, custom TargetActors](https://github.com/tranek/GASDocumentation#concepts-targeting-actors)
* [GameplayAbilityWorldReticles](https://github.com/tranek/GASDocumentation#concepts-targeting-reticles)
* Play replicated montages on multiple Skeletal Mesh Components **belonging to the AvatarActor** in an ability
* [Subclassing `FGameplayEffectContext`](https://github.com/tranek/GASDocumentation#concepts-ge-context) to send additional information to GameplayCues
* Character shield that drains before health is removed by damage
* Item pickups
* Single button interaction system. Press or Hold 'E' to interact with interactable objects including player reviving, a weapon chest, and a sliding door.

This project does not show predicting projectiles. I refer you to the Unreal Tournament source code for how to do that using a fake projectile on the owning client.

| Weapon          | Primary Ability (Left Mouse Button)                  | Secondary Ability (Right Mouse Button)                                                                     | Alternate Ability (Middle Mouse Button)                     |
| --------------- | ---------------------------------------------------- | ---------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------- |
| Rifle           | Fire hitscan bullets based on the current fire mode. | Aim down sights, reduces firing spread.                                                                    | Changes fire modes between full auto, semi auto, and burst. |
| Rocket Launcher | Fire a rocket.                                       | Aim down sights. Starts lock-on targeting for homing rockets. Press LMB to fire homing rockets at targets. | None                                                        |
| Shotgun         | Fire hitscan pellets based on the current fire mode. | Aim down sights, reduces firing spread for pellets.                                                        | Changes fire modes between semi auto and full auto.         |

## Acknowledgements

[KaosSpectrum](https://github.com/KaosSpectrum) provided significant contributions to figuring out how the ability batching system works and general feedback. Check out his game development [blog](https://www.thegames.dev/).