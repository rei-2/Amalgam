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

## Progreso (config del usuario y funciones nuevas)

### Compilación
- El usuario compila en su PC con Visual Studio 2026 Community: clonó el repo en su carpeta `source\repos\Claudiosilvag\Amalgam`, redestinó la solución a v145 en local (ese cambio de `Amalgam.vcxproj` no se sube) y compila Release x64. La DLL queda en `output\x64\Release\Amalgamx64Release.dll`.
- GitHub Actions ya está activo: cada push o PR compila la DLL y la deja en Actions → Artifacts.
- La config del juego vive en `...\Team Fortress 2\Amalgam\`.

### Hecho
- **Código** (opciones nuevas, apagadas por defecto):
  - `Prioritize medic` (Aimbot → General): Medic enemigos primero. PR #2, fusionado.
  - `Body fallback` (Projectile → Hitboxes): Huntsman apunta a la cabeza y, si no puede, al cuerpo. PR #3, fusionado y compilado.
  - `Prioritize buildings` (Aimbot → General): construcciones enemigas primero, todas con la misma prioridad y por delante de los Medic. PR #3.
  - `Crit rage targets` (Crit Hack): fuerza crítico contra jugadores con la etiqueta llamada exactamente `Rage`. PR #3.
  - Detonator con radio de explosión 110 (como la Scorch Shot): el aimbot puede apuntar a paredes, suelo o aire cerca de un enemigo tapado y la auto detonación explota la flare; el círculo de Splash radius también la dibuja. PR #4.
  - `Extinguish team` (Auto airblast): apaga compañeros en llamas con apuntado silencioso, después de reflejar proyectiles. PR #4.
  - Lugares propios para objetivos prioritarios en Max targets: Rage/Cheater → construcciones → Medic se evalúan además de los objetivos normales, sin quitarles el lugar. PR #4.
  - `Auto rage` (Misc → Automation, interruptor + `Auto rage kills` 1–10): etiqueta Rage al jugador que te mata N veces en el mapa actual (se reinicia al cambiar de mapa, ignora Dead Ringer); crea la etiqueta Rage (prioridad 2, roja) si no existe. PR #4.
- **Triggerbot** (Aimbot → General, sección Triggerbot): `Enabled`, `Hitboxes` (Head/Body/Arms/Legs), `Delay` 0–500 ms y `Wait for charge` (Sniper). Solo hitscan; traza un rayo hacia donde va el disparo (`viewangles` + punch) y dispara si golpea a un enemigo válido (respeta Target/Ignore del aimbot); el rifle de Sniper solo dispara con zoom; Classic, Medigun y Wrangler no están soportados. Código: `RunTriggerbot` en `AimbotHitscan.cpp`, llamado al final de `CAimbot::RunMain`. PR #8.
- **Autodet delay** (Aimbot → Projectile, 0–2000 ms): tiempo extra que espera una sticky después de armarse antes de auto detonar. Solo stickies. PR #8.
- **Extinguish team con Jarate (Sydney Sleeper)** (Hitscan → Modifiers → Extinguish team, ya existía): ahora los compañeros quemándose tienen la prioridad de la etiqueta Cheater (1), solo son objetivo con zoom (sin zoom no aplica Jarate) y se ignoran Wait for charge / Wait for headshot al dispararles. Solo sin friendly fire. PR #9.
- **Config v6** (`default-nueva-v6.json`): igual que la v5 + Extinguish team en el bind `Sniper → - Hitscan` (14): Modifiers 6 → 70.
- **Config v5** (`default-nueva-v5.json`): igual que la v4 + Triggerbot activado (todas las hitboxes, Delay 0, Wait for charge) y Autodet delay 150 ms.
- **Firmas tras la actualización de TF2 (nov 2026)**: `CTFClientScoreBoardDialog_OnCommand` (client.dll, cambió `sub rsp` a imm32) y `KeyValues_Initialize` (engine.dll: ahora es el constructor `KeyValues(name, bUseGrowableTable)`, se llama con `false`). Verificadas contra las DLL x64 del usuario (únicas). La firma del PR #326 de rei-2 apunta a `SetName` y no inicializa el objeto: no usarla. Para futuras actualizaciones, pedir al usuario `bin\x64\engine.dll` y `tf\bin\x64\client.dll` (x64, no las de 32 bits).
  - Ojo: una actualización también puede mover offsets fijos sin romper firmas (la DLL carga pero la opción no hace nada). Ej.: `Extend queue` (`CMatchInviteNotification_OnTick`) usa el tiempo de auto-unión en `rcx + 648` (0x288; antes 616). Se verifica desensamblando la función: lee un double en esa posición y lo compara con `Plat_FloatTime`.
- GitHub puede desactivar el workflow; el usuario lo reactiva en Actions. El workflow tiene `workflow_dispatch` (botón "Run workflow").
- **Config v4** (`default-nueva-v4.json`): igual que la v3 + en el bind Medic `- Projectile` (12) Heal priority = Prioritize team (1) y Max targets 3. Arregla que la ballesta no curara a compañeros: con Max targets 1 y prioridad igual, el único lugar se lo llevaba un enemigo (o un Medic enemigo por Prioritize medic). Ojo: Auto arrow nunca dispara a compañeros con la vida llena (es normal).
- **Config v3** (`default-nueva-v3.json`): igual que la v2 + bind `Pyro → - Projectile` (Splash Include, Modifiers 24, Max targets 3, Ignore 1051), Auto airblast 15 (agrega Extinguish team) y Auto rage encendido con 3 kills.
- **Config v2** (`default-nueva-v2.json`): igual que la primera + Max targets 3 en el Sniper `- Projectile` (bind 10). Arregló que el Huntsman casi no disparara.
- **Primera config nueva** (`default-nueva.json`, generada desde la config original del usuario sin modificarla):
  - Global: Prioritize medic y Crit rage targets activados.
  - Soldier: hitboxes de proyectil 47 (Prioritize feet).
  - Demoman: Prioritize buildings solo en pipas y stickies (bind `- Projectile`), Max simulation time 2.5 s.
  - Sniper: Huntsman hitboxes 87; bind nuevo `- Hitscan` con Modifiers 6 y Hitboxes 39.
  - Medic: splash en Off en proyectiles; Auto heal en el bind de clase.
  - Heavy (nuevo): `- Hitscan` con Auto rev (32) y Hitboxes 6.
  - Engineer (nuevo): `- Projectile` con Ignore 1051 y splash Off; Auto repair en el bind de clase.
  - Scout (nuevo): `- Projectile` con Ignore 1051 y splash Off.
  - Spy (nuevo): `- Hitscan` con Wait for headshot (2).
  - Se quitó Hitscan Modifiers del bind `Hitscan` (2) y de `[AIMBOT]` (1) para que los binds de Hitscan por clase funcionen.

### Descartado por el usuario
- Ignore 1051 en `[AIMBOT]`, quitar Ignore FOV del airblast, limpieza de opciones inútiles, Prioritize feet en Demoman, Hit chance 50% para Huntsman, separar melee de Shift, mover auto detonate solo al Demoman.
- Huntsman: quitar Target dormant y bajar Hit chance (bastó con Max targets 3).
- Auto detonator jump: no por ahora.

### Pendiente
- Probado y funcionando (24 nov 2026, build #13 de `master`, PR #5, #6 y #7): la DLL carga tras la actualización de TF2 y Extend queue vuelve a funcionar.
- Falta confirmar en el juego la Detonator, Extinguish team, los lugares prioritarios, Auto rage y la ballesta del Medic con la config v4 (curar compañeros heridos manteniendo Shift).
- Opcional: activar Visuals → UI → Scoreboard utility para poner o quitar la etiqueta Rage con clic derecho en la tabla de puntajes.
