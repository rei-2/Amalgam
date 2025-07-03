# Amalgam - Competitive Fork

[![GitHub Repo stars](https://img.shields.io/github/stars/coffeegrind123/Amalgam-Comp)](/../../stargazers)
[![GitHub Workflow Status (with event)](https://img.shields.io/github/actions/workflow/status/coffeegrind123/Amalgam-Comp/msbuild.yml?branch=master)](/../../actions)
[![GitHub commit activity (branch)](https://img.shields.io/github/commit-activity/m/coffeegrind123/Amalgam-Comp)](/../../commits/)

A competitive-focused fork featuring enhanced information gathering and advantage tracking systems.

<p align="center">
  <a href="https://nightly.link/coffeegrind123/Amalgam-Comp/workflows/msbuild/master/Amalgamx64Release.zip">
    <img src=".github/assets/download.png" alt="Download Button" width="400" height="auto" align="center">
  </a>
  <br/><br/>
  <a href="https://nightly.link/coffeegrind123/Amalgam-Comp/workflows/msbuild/master/Amalgamx64ReleaseAVX2.zip">
    <img src=".github/assets/download_avx2.png" alt="Download AVX2" width="auto" height="auto">
  </a>
  <a href="https://nightly.link/coffeegrind123/Amalgam-Comp/workflows/msbuild/master/Amalgamx64ReleaseFreetype.zip">
    <img src=".github/assets/freetype.png" alt="Download Freetype" width="auto" height="auto">
  </a>
  <a href="https://nightly.link/coffeegrind123/Amalgam-Comp/workflows/msbuild/master/Amalgamx64ReleaseFreetypeAVX2.zip">
    <img src=".github/assets/freetype_avx2.png" alt="Download Freetype AVX2" width="auto" height="auto">
  </a>
</p>

###### AVX2 may be faster than SSE2 though not all CPUs support it (`Steam > Help > System Information > Processor Information > AVX2`). Freetype uses freetype as the text rasterizer and includes some custom fonts, which results in better looking text but larger DLL sizes. PDBs are for developer use.
###### If nightly.link is down, you can still download through [github](https://github.com/coffeegrind123/Amalgam-Comp/actions) with an account.

## Added Features

### Always-On Information Systems
- **UberTracker** - Comprehensive uber advantage tracking with detailed medic information, weapon types, and advantage calculations
- **HealthBarESP** - Health bars for visible-only players. Includes a medic mode (health bars for teammates when playing medic), health-responsive visibility, and overheal display
- **CritHeals Indicator** - Triangle indicators above players eligible for crit heals (medic-only feature) with uber build rate warnings
- **PlayerTrails** - Colored movement trails for enemy players showing their recent paths with visibility-based display and fade-out effects
- **StickyESP** - Always-on stickybomb ESP with 2D/3D boxes, visibility-based coloring (green=visible, red=invisible), and distance filtering
- **FocusFire** - Multi-targeting detection system that highlights enemies being focused by multiple teammates with red corner boxes (2+ attackers within 4.5 seconds)
- **Match HUD Enhancement** - Enemy health and class information displayed on match scoreboard/HUD for instant target assessment

## Installation

[Xenos](https://github.com/DarthTon/Xenos/releases) recommended for injection.
