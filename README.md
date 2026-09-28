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
