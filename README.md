<div align="center">

  ## <img src=".github/assets/amalgam666.svg" alt="Amalgam 666" height="100">

  [![Workflow status](https://img.shields.io/github/actions/workflow/status/Claudiosilvag/Amalgam/msbuild.yml?branch=master&style=for-the-badge)](/../../actions)
  [![Commit activity](https://img.shields.io/github/commit-activity/m/Claudiosilvag/Amalgam?style=for-the-badge)](/../../commits/)
  
  [![Download](.github/assets/download.svg)](https://nightly.link/Claudiosilvag/Amalgam/workflows/msbuild/master/Amalgamx64Release.zip)
  [![PDB](.github/assets/pdb.svg)](https://nightly.link/Claudiosilvag/Amalgam/workflows/msbuild/master/Amalgamx64ReleasePDB.zip)
  [![Download AVX2](.github/assets/download_avx2.svg)](https://nightly.link/Claudiosilvag/Amalgam/workflows/msbuild/master/Amalgamx64ReleaseAVX2.zip)
  [![PDB AVX2](.github/assets/pdb.svg)](https://nightly.link/Claudiosilvag/Amalgam/workflows/msbuild/master/Amalgamx64ReleaseAVX2PDB.zip)
  <br>
  [![Freetype](.github/assets/freetype.svg)](https://nightly.link/Claudiosilvag/Amalgam/workflows/msbuild/master/Amalgamx64ReleaseFreetype.zip)
  [![PDB Freetype](.github/assets/pdb.svg)](https://nightly.link/Claudiosilvag/Amalgam/workflows/msbuild/master/Amalgamx64ReleaseFreetypePDB.zip)
  [![Freetype AVX2](.github/assets/freetype_avx2.svg)](https://nightly.link/Claudiosilvag/Amalgam/workflows/msbuild/master/Amalgamx64ReleaseFreetypeAVX2.zip)
  [![PDB Freetype AVX2](.github/assets/pdb.svg)](https://nightly.link/Claudiosilvag/Amalgam/workflows/msbuild/master/Amalgamx64ReleaseFreetypeAVX2PDB.zip)

  <sub>AVX2 puede ser más rápido, pero no todas las CPU lo soportan (`Steam > Ayuda > Información del sistema > Información del procesador > AVX2`). Freetype dibuja el texto con freetype e incluye fuentes propias: se ve mejor, pero la DLL pesa más. Los PDB son para desarrolladores. </sub>
  <br>
  <sub>AVX2 may be faster, though not all CPUs support it (`Steam > Help > System Information > Processor Information > AVX2`). Freetype uses freetype as the text rasterizer and includes custom fonts: better looking text, larger DLL. PDBs are for developer use. </sub>
  <br><br>
  <sub>Si nightly.link no funciona, puedes descargar desde [GitHub Actions](/../../actions) con una cuenta. · If nightly.link is down, you can download from [GitHub Actions](/../../actions) with an account. </sub>

</div>

---

<details open>
<summary><b>🇪🇸 Español</b></summary>

## Novedades de Amalgam 666

- **Prioritize medic** (Aimbot → General): los Medic enemigos van primero.
- **Prioritize buildings** (Aimbot → General): las construcciones enemigas (sentry, dispenser, teleporter) van primero, por delante de los Medic.
- **Lugares prioritarios en Max targets**: los objetivos prioritarios (etiquetas Rage/Cheater → construcciones → Medic) se evalúan además de los normales, sin quitarles el lugar.
- **Body fallback** (Projectile → Hitboxes): el Huntsman apunta a la cabeza y, si no puede, al cuerpo.
- **Detonator con explosión**: si el enemigo está tapado, apunta a una pared, al suelo o al aire cerca de él y la auto detonación explota la flare. También se dibuja su radio de explosión.
- **Extinguish team** (Auto airblast): el Pyro apaga con airblast a los compañeros en llamas.
- **Crit rage targets** (Crit Hack): críticos forzados contra los jugadores con la etiqueta `Rage`.
- **Auto rage** (Misc → Automation): pone la etiqueta `Rage` a quien te mata varias veces en el mismo mapa (cantidad configurable).

## Funciones

**Aimbot**
- **General**: modos Plain, Smooth, Silent, Locking y Assistive; selección por FOV, distancia o híbrida; objetivos (jugadores, construcciones, stickies, NPCs, bombas); filtros de a quién ignorar (amigos, party, invulnerables, invisibles, Dead Ringer, Vaccinator, disfrazados, taunts); Aim FOV y círculo; auto shoot; no spread.
- **Hitscan**: hitboxes, multipoint, bodyaim si es letal, headshot only, tapfire, esperar headshot o carga del rifle, auto scope, auto rev del minigun.
- **Projectile**: predicción de strafe, hitboxes automáticas, predicción de splash, auto detonate de stickies y flares, auto airblast con redirect, carga automática (Huntsman, stickies), tiros en parábola, hit chance.
- **Melee**: auto backstab, predicción del golpe, Whip team.
- **Healing**: auto heal, auto arrow (ballesta), auto repair, auto sandvich, auto vaccinator, über al pedido de voz.

**Crit Hack**: forzar críticos, evitar críticos aleatorios, críticos siempre en melee.

**Backtrack**: fake latency, fake interp y ventana de backtrack.

**Doubletap / Warp**: doubletap, warp, recarga de ticks y anti-warp.

**Fakelag**: plain, random y adaptive.

**Antiaim y Resolver**: pitch y yaw real/falso, spin, minwalk; resolver automático y manual.

**Visuals**
- **ESP por grupos**: nombre, caja, vida, über, clase, arma, prioridad, etiquetas, buffs, ping y más; chams, glow, flechas fuera de pantalla, backtrack, trayectorias y líneas de visión de Snipers.
- Tercera persona, FOV, removals (scope, disfraces, retroceso visual, etc.), trazadores y efectos, viewmodel, colores del mundo y del cielo, predicción de trayectorias y cámara del proyectil.

**Misc**: auto strafe, bunnyhop, edge jump, auto rocket jump y otros saltos, fast stop, anti-backstab, control de taunts y karts, anti-autobalance, anti-AFK, votos automáticos, pure bypass, unlock CVars, colas por región, opciones de MvM y bloqueo de sonidos.

**Lista de jugadores**: etiquetas con prioridad (Cheater, Ignored, propias), alias y detección de cheaters.

**Logs**: votos, cambios de clase, daño, detecciones, etiquetas y resolver, con notificaciones en pantalla.

**Binds**: cualquier opción puede cambiar según tecla, clase, tipo de arma, slot o situación (espectado, con zoom, etc.).

</details>

<details>
<summary><b>🇬🇧 English</b></summary>

## What's new in Amalgam 666

- **Prioritize medic** (Aimbot → General): enemy Medics go first.
- **Prioritize buildings** (Aimbot → General): enemy sentries, dispensers and teleporters go first, ahead of Medics.
- **Priority slots in Max targets**: prioritized targets (Rage/Cheater tags → buildings → Medics) are evaluated on top of the regular ones instead of pushing them out.
- **Body fallback** (Projectile → Hitboxes): the Huntsman aims for the head and falls back to the body when the head can't be hit.
- **Detonator splash**: when the enemy is behind cover, it aims at a nearby wall, floor or air point and auto detonate explodes the flare. Its explosion radius is drawn too.
- **Extinguish team** (Auto airblast): the Pyro airblasts burning teammates.
- **Crit rage targets** (Crit Hack): forced crits against players tagged `Rage`.
- **Auto rage** (Misc → Automation): tags `Rage` whoever kills you a set number of times in the same map.

## Features

**Aimbot**
- **General**: Plain, Smooth, Silent, Locking and Assistive modes; FOV, distance or hybrid target selection; targets (players, buildings, stickies, NPCs, bombs); ignore filters (friends, party, invulnerable, invisible, Dead Ringer, Vaccinator, disguised, taunting); aim FOV and circle; auto shoot; no spread.
- **Hitscan**: hitboxes, multipoint, bodyaim if lethal, headshot only, tapfire, wait for headshot or rifle charge, auto scope, auto rev minigun.
- **Projectile**: strafe prediction, automatic hitboxes, splash prediction, sticky and flare auto detonate, auto airblast with redirect, charged weapons (Huntsman, stickies), lob angles, hit chance.
- **Melee**: auto backstab, swing prediction, whip team.
- **Healing**: auto heal, auto arrow (crossbow), auto repair, auto sandvich, auto vaccinator, uber on voice command.

**Crit Hack**: force crits, avoid random crits, always melee crit.

**Backtrack**: fake latency, fake interp and backtrack window.

**Doubletap / Warp**: doubletap, warp, tick recharge and anti-warp.

**Fakelag**: plain, random and adaptive.

**Antiaim & Resolver**: real/fake pitch and yaw, spin, minwalk; automatic and manual resolver.

**Visuals**
- **Group-based ESP**: name, box, health, uber, class, weapon, priority, tags, buffs, ping and more; chams, glow, offscreen arrows, backtrack, trajectories and Sniper sightlines.
- Thirdperson, FOV, removals (scope, disguises, view punch, etc.), tracers and effects, viewmodel, world and sky colors, path prediction and projectile camera.

**Misc**: auto strafe, bunnyhop, edge jump, auto rocket jump and other jumps, fast stop, anti-backstab, taunt and kart control, anti-autobalance, anti-AFK, auto votes, pure bypass, unlock CVars, region queueing, MvM options and sound blocking.

**Playerlist**: priority tags (Cheater, Ignored, custom), aliases and cheater detection.

**Logs**: votes, class changes, damage, detections, tags and resolver, with on-screen notifications.

**Binds**: any option can change by key, class, weapon type, slot or situation (spectated, zoomed, etc.).

</details>

---

<div align="center">
  <sub>Basado en / Based on <a href="https://github.com/rei-2/Amalgam">Amalgam</a> de / by rei-2 · <a href="https://github.com/rei-2/Amalgam/wiki">Wiki</a></sub>
</div>
