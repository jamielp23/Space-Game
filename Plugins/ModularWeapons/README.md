# Modular Weapons — UE5.6 Weapon Architecture

A **data-driven, fully modular** weapon framework for Unreal Engine 5.6.
C++ first (all behaviour lives in native code), Blueprint-friendly (designers
author weapons entirely from Data Assets), with every system independent and
composable.

It ships with everything needed for the seven weapon families in the design:

| Family | Firing strategy | Resource model | Typical config |
|---|---|---|---|
| **Pistol** | Hitscan | Magazine | Single shot, low spread, light recoil |
| **Assault Rifle** | Hitscan | Magazine | Full auto, bloom, climbing recoil |
| **Shotgun** | Hitscan (many pellets) | Magazine (shell-by-shell) | Wide spread, sharp damage falloff |
| **Sniper** | Hitscan | Magazine | Single shot, ~zero spread, high crit multiplier |
| **Energy** | Beam | Heat | High RPM, overheat lockout, no reload |
| **Heavy** | Projectile | Charges / Magazine | Slow rockets, radial damage |
| **Throwable** | Throwable | Charges | Arcing grenade, timed fuse, radial damage |

Nothing in that table is hard-coded — each family is just a **combination of
modules** on a `UWeaponDefinition` Data Asset. Designers create new weapons, or
whole new archetypes, without writing or compiling any code.

---

## 1. Installation

Drop the `ModularWeapons` folder into your project's `Plugins/` directory (or a
game feature's plugins folder), regenerate project files, and enable the plugin.
It depends only on `Engine`, `GameplayTags` and `Niagara`, so it drops into any
UE5.6 project.

---

## 2. Core concept: definition vs. instance vs. modules

The architecture separates **configuration**, **runtime state** and **behaviour**
into three cleanly decoupled layers:

```
UWeaponDefinition  (Data Asset — shared, immutable configuration)
   ├── FiringModule   : UWeaponFiringModule   (how a shot manifests)
   ├── AmmoModule     : UWeaponAmmoModule      (mag / heat / charges)
   ├── SpreadModule   : UWeaponSpreadModule    (accuracy & bloom)
   ├── RecoilModule   : UWeaponRecoilModule    (view kick)
   └── DamageModule   : UWeaponDamageModule    (damage, falloff, crits)

UWeaponInstance    (UObject — per-owner, mutable runtime state + orchestration)
   holds: current mag / reserve / heat / charges / bloom / cadence / reload timers
   drives: the modules above to fire, reload, cool down, recover

UWeaponComponent   (ActorComponent — the only actor-aware piece)
   holds: an inventory of UWeaponInstance, the equipped weapon
   does : input forwarding, ticking, firing geometry (aim + muzzle), recoil apply
```

Why this split matters:

- **Every system is independent.** Firing, ammo, spread, recoil and damage know
  nothing about each other. Any module may be left empty for a safe default
  (no spread module = perfect accuracy; no recoil module = no kick).
- **Definitions are shared and immutable.** One `UWeaponDefinition` backs every
  copy of that weapon in the world, so modules hold *no* mutable state — all
  per-shot state lives on the `UWeaponInstance`. This is memory-cheap and
  thread-safe to read.
- **Only `UWeaponComponent` touches the owning actor.** Everything else is
  engine- and actor-agnostic and trivially reusable across player and AI pawns.

### The firing strategy seam

`UWeaponFiringModule::ExecuteFire()` is the single extensibility point for how a
shot appears in the world. Four strategies ship in the box:

- `UHitscanFiringModule` — instant line trace, applies damage at the hit.
- `UProjectileFiringModule` — spawns a travelling `AWeaponProjectile`.
- `UBeamFiringModule` — energy beam trace with a rendered beam FX.
- `UThrowableFiringModule` — arcing, gravity-affected thrown ordnance.

To add a brand-new firing behaviour (e.g. a chaining lightning gun), an engineer
subclasses `UWeaponFiringModule` once and overrides `ExecuteFire()`. It
immediately becomes selectable in every weapon Data Asset. No other system
changes.

---

## 3. Designer workflow — create a weapon with zero code

1. **Create the Data Asset.** Content Browser → *Miscellaneous → Data Asset* →
   pick **Weapon Definition**. Name it e.g. `DA_Weapon_Pistol_Kestrel`.
2. **Set identity & presentation.** Display name, `Weapon Type`, mesh, icon,
   fire/reload montages, gameplay tags.
3. **Choose the fire cadence.** `Fire Mode` (Single / Burst / Full Auto),
   `Rounds Per Minute`, and `Burst Count` if applicable.
4. **Compose the modules.** Each module is an inline dropdown — pick a class and
   its properties expand right there in the details panel:
   - **Firing Module** → e.g. `Hitscan Firing` (set range, tracer/impact FX,
     `Projectiles Per Shot` for pellets).
   - **Ammo Module** → pick the `Model` (Magazine / Heat / Charges / Infinite);
     the relevant fields show automatically.
   - **Spread / Recoil / Damage Module** → optional; tune to taste.
5. **Add it to a pawn.** Put a `Weapon Component` on your character and add the
   Data Asset to its `Default Weapons` array (or call `GrantWeapon` at runtime).

That is the entire authoring loop. New weapons and new archetypes are pure data.

### Recommended module recipe per family

- **Pistol** — Hitscan (range 15000) · Magazine (mag 12, reload 1.4s) · Spread
  (base 1°) · Recoil (light) · Damage (25, small crit).
- **Assault Rifle** — Hitscan · Magazine (mag 30, reload 2.2s) · Spread (bloom
  per shot 0.5°, max 5°) · Recoil (`VerticalClimbOverShots` ramping) · Damage
  (22, distance falloff).
- **Shotgun** — Hitscan with `Projectiles Per Shot = 8` · Magazine with
  `bReloadPerRound = true` (mag 6) · Spread (base 5–7°) · Damage (12/pellet with
  a steep falloff curve).
- **Sniper** — Hitscan (range 30000) · Magazine (mag 5, reload 3s) · Spread
  (~0°, no bloom) · Recoil (strong single kick) · Damage (120, crit ×3).
- **Energy** — Beam · **Heat** ammo (heat/shot 6, max 100, cooling 30/s,
  overheat lockout 2.5s) · high RPM (~900) · Damage (small per-shot).
- **Heavy (rocket)** — Projectile (slow speed, `ProjectileParams` with explosion
  radius/damage/impulse) · Charges or small Magazine · Damage handled by the
  projectile's radial blast.
- **Throwable (grenade)** — Throwable (`UpwardArc`, `GravityScale = 1`,
  `ThrowableParams` fuse + explosion) · **Charges** ammo (Max Charges 3).

---

## 4. Wiring the component to input (C++ example)

```cpp
// In your character:
UPROPERTY(VisibleAnywhere) TObjectPtr<UWeaponComponent> Weapons;

// Construction:
Weapons = CreateDefaultSubobject<UWeaponComponent>(TEXT("Weapons"));

// After the weapon mesh exists (e.g. BeginPlay):
Weapons->SetWeaponMesh(FirstPersonWeaponMesh); // supplies the "Muzzle" socket

// Input bindings:
void AMyCharacter::OnFirePressed()  { Weapons->StartFire(); }
void AMyCharacter::OnFireReleased() { Weapons->StopFire();  }
void AMyCharacter::OnReload()       { Weapons->Reload();    }
void AMyCharacter::OnAim(bool bDown){ Weapons->SetAiming(bDown); }
void AMyCharacter::OnNextWeapon()   { Weapons->EquipNextWeapon(); }
```

Blueprint gets the same API: `Weapon Component` exposes `Start Fire`, `Stop
Fire`, `Reload`, `Set Aiming`, `Equip Next/Previous Weapon`, `Grant Weapon`, plus
`On Equipped Weapon Changed`. The equipped `Weapon Instance` exposes `On Ammo
Changed`, `On Heat Changed`, `On State Changed`, `On Fired` and `On Recoil` for
HUD and feedback binding.

Recoil is emitted as a `(pitch, yaw)` impulse the owner consumes. By default the
component applies it to a player's control rotation; set `bApplyRecoilToController
= false` to drive your own spring/curve recovery instead.

---

## 5. Source layout

```
Source/ModularWeapons/
  Public/
    Core/
      WeaponTypes.h          Enums (type, fire mode, state, ammo model) + fire context
      WeaponDefinition.h     The Data Asset designers author
      WeaponInstance.h       Per-owner runtime state + orchestration
      WeaponComponent.h      Actor component: inventory, input, firing geometry
    Modules/
      WeaponModule.h         Base for all composable modules
      WeaponAmmoModule.h     Magazine / Heat / Charges / Infinite
      WeaponSpreadModule.h   Accuracy cone + dynamic bloom
      WeaponRecoilModule.h   Per-shot view kick
      WeaponDamageModule.h   Damage, distance falloff, critical hits
      Firing/
        WeaponFiringModule.h Abstract firing strategy (the extensibility seam)
        HitscanFiringModule.h
        ProjectileFiringModule.h
        BeamFiringModule.h
        ThrowableFiringModule.h
    Projectiles/
      WeaponProjectile.h     Reusable bullet / rocket / grenade actor
  Private/  (mirrors Public with the implementations)
```

Every public class carries doxygen-style documentation explaining its role and
its independence from the rest of the system.

---

## 6. Extending the system

- **New firing behaviour** → subclass `UWeaponFiringModule`, override
  `ExecuteFire`. Selectable in every weapon immediately.
- **New resource rule** → add an `EAmmoModel` case and handle it in
  `UWeaponAmmoModule` (all five hooks: init / can-fire / consume / reload /
  cool). Nothing else changes.
- **Custom projectiles** → subclass `AWeaponProjectile` in Blueprint for
  bespoke meshes, trails and `On Detonate` FX; reference it from the projectile
  or throwable module.
- **Attachments / mods** → because a weapon is a module graph, an attachment
  system can swap or layer modules on a per-instance copy of the definition.
```
