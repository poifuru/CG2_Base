Upgrade assessment for C++ build tools upgrade

Solution: C:\MagosuyaEngine_vs2026\project\CG2_00_01.sln
Project: C:\MagosuyaEngine_vs2026\project\CG2_00_01.vcxproj

Build summary (from rebuild):
- Errors: 0
- Warnings: 1

In-scope issues (to fix now):
1) Shader compiler warning (fxc) in project C:\MagosuyaEngine_vs2026\project\CG2_00_01.vcxproj
   - Location reported: C:\MagosuyaEngine_vs2026\project\fxc
   - Message: "no output provided for debug - embedding PDB in shader container.  Use -Qembed_debug to silence this warning."
   - Impact: Warning only; no build errors. It indicates shader PDBs are not emitted separately and fxc embeds debug info in the shader container by default. This is harmless for release but noisy during builds.
   - Proposed fix: Add the flag "-Qembed_debug" to shader compilation options so the warning is silenced and PDB is embedded explicitly. Implementation paths:
     a) Add <AdditionalOptions>-Qembed_debug %(AdditionalOptions)</AdditionalOptions> inside the <FxCompile> element of the ItemDefinitionGroup(s) in `C:\MagosuyaEngine_vs2026\project\CG2_00_01.vcxproj` so it applies to all shader items.
     b) Or update the specific <FxCompile Include=...> items to include an <AdditionalOptions> entry if you prefer per-shader control.
   - Verification: Unload project -> edit .vcxproj -> validate with cppupgrade_validate_vcxproj_file -> reload project -> run cppupgrade_rebuild_and_get_issues. Expect 0 warnings/errors.

Out-of-scope issues (not changing now):
- None (no other warnings or errors reported by the rebuild).

Execution plan (high level):
1) Create a new git branch for changes.
2) Unload project `C:\MagosuyaEngine_vs2026\project\CG2_00_01.vcxproj`.
3) Edit the .vcxproj: add <AdditionalOptions>-Qembed_debug %(AdditionalOptions)</AdditionalOptions> under the <FxCompile> node in each ItemDefinitionGroup (Debug/Release/Development).
4) Run cppupgrade_validate_vcxproj_file on the edited .vcxproj.
5) Reload the project.
6) Run cppupgrade_rebuild_and_get_issues to verify the warning is gone and build is clean.
7) Commit the change to the new branch.

Please confirm if you want me to proceed with the planned changes and verification. If you prefer a different fix (for example, leave PDBs external or silence differently), specify it now.
