# clickreplay
a very coolio Dear Imgui "CLICK REPLAY" engine w random seed and dataset import. 
i will eventually release a click recorder or add one to the current ui, there is a .csv file you can download, to test this out, in the meanwhile.
some things may be broken, but fixing them is not my main concern atm. 
be free.
- "AppState" owns the application components and current page.
- "ReplayConfig" contains settings for one replay run.
- "ReplayEngine" owns the replay thread and scheduler.
- "ReplaySnapshot" is the read-only state exposed to the UI.
- "Dataset" loads and shuffles CSV intervals.
- "Input" handles keyboard state and the low-level mouse hook.
- "Platform"  handles "SendInput" and the "javaw.exe" foreground check.
- "Statistics" stores interval/CPS history and logs.
- "gui/*" contains the Dear ImGui pages and DirectX 11 setup.
The replay engine does not include or reference "AppState". When "start()" is called, it copies the supplied "ReplayConfig". The GUI reads replay state through "ReplayEngine::snapshot()" rather than accessing individual runtime fields.
<img width="1920" height="1002" alt="{58C20724-4B1F-43FE-8874-BCA7FA534CF7}" src="https://github.com/user-attachments/assets/905e603a-5c37-49ea-852d-607718ebf417" />
<img width="1914" height="1000" alt="{3C6E7E1F-D590-4E8A-B77B-6E71CF90737E}" src="https://github.com/user-attachments/assets/0dcaa77a-b15c-4eb7-bb48-1ab0d7dc5086" />


## License

Copyright (c) 2026 Emi.

This project is released under the **Click Replay GUI Non-Commercial Source
License 1.0**. You may use, modify, and redistribute the project for
non-commercial purposes, including free redistribution of modified versions.
Selling the software, licensing it for payment, bundling it into a paid
product or service, or other commercial use requires prior written permission
from the copyright holder.

See [`LICENSE`](LICENSE) for the complete terms.

**Important:** this is a custom source-available, non-commercial license. It is
not an OSI-approved Open Source license.

## Third-party software

This project uses [Dear ImGui](https://github.com/ocornut/imgui), which is
licensed under the MIT License. Dear ImGui is fetched by CMake at configure
time rather than vendored in this repository.

Third-party software remains subject to its own license terms. This project’s
non-commercial license does not change the license of any third-party
component.

## Status

This is a Windows desktop project. The repository is intended to remain small
and straightforward rather than provide a general-purpose replay framework.
