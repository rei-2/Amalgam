# Instrucciones para Claude

## Regla principal

**Siempre pregunta antes de proceder.** Antes de editar archivos, hacer commits, hacer push, abrir PRs o modificar la config del usuario, explica lo que piensas hacer y espera su confirmación explícita. Revisar y analizar está permitido sin preguntar.

- **Antes de programar, primero acordamos un listado de cambios.** Presenta la lista de lo que se va a hacer, espera a que el usuario la apruebe y recién entonces programa. No adelantes código ni cambios de config fuera de lo aprobado.
- Responde en español.
- Nunca modifiques el archivo de config original del usuario: entrega la versión corregida en un archivo nuevo.
- Avanzamos clase por clase en la config; no adelantes cambios de otras clases.

## Proyecto

- Amalgam es un proyecto C++ de Visual Studio (`Amalgam.sln`, MSVC, Windows). No se puede compilar en Linux; el workflow `.github/workflows/msbuild.yml` compila en Windows si GitHub Actions está activado en el repo.
- Las variables del menú se definen en `Amalgam/src/SDK/Vars.h` y se guardan solas en la config; el menú está en `Amalgam/src/Features/ImGui/Menu/Menu.cpp`.
- Orden de objetivos del aimbot: `Amalgam/src/Features/Aimbot/AimbotGlobal/AimbotGlobal.cpp` (`SortTargetsPre` / `SortTargetsPost`).
- Hitboxes de proyectiles (Huntsman, etc.): `GetHitboxPriority` en `Amalgam/src/Features/Aimbot/AimbotProjectile/AimbotProjectile.cpp`.
- Crit hack: `Amalgam/src/Features/CritHack/CritHack.cpp` (`GetCritRequest`).
- Etiquetas de jugadores (Cheater, etc.): se guardan en `Players.json`, no en la config; ver `Amalgam/src/Features/Players/`.

## Config y binds

- Los binds forman un árbol (`Parent` = índice del bind padre, `-1` = raíz). El índice es la posición en el JSON, no la clave.
- Cuando dos binds activos cambian la misma variable, gana el bind hijo; entre hermanos gana el que aparece primero.
- En `Vars`, la clave `-1` es el valor global y las demás claves son el índice del bind que lo cambia.
- Tipos de bind: 0 Tecla (Info: 0 Hold, 1 Toggle, 2 DoubleClick), 1 Clase (Info: 0 Scout, 1 Soldier, 2 Pyro, 3 Demoman, 4 Heavy, 5 Engineer, 6 Medic, 7 Sniper, 8 Spy), 2 Tipo de arma (Info: 0 Hitscan, 1 Projectile, 2 Melee, 3 Throwable), 3 Slot, 4 Misc.
- El auto airblast y el auto detonate funcionan aunque no se mantenga la tecla del aimbot.
