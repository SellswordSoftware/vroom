# Play Vroom on the web

The static site is in site/. It automatically starts the bundled Vroom 0.2.1
release in joyrider3774's CHGame WebAssembly emulator. There is no game list,
ROM picker, external ROM parameter, or embedded third-party page.

The site includes its own JavaScript, WebAssembly, game binary, styles and
licenses. Visitors do not need another server, CDN, account, emulator install
or build tool. There is no analytics or remote game download.

## Preview locally

From the vroom directory:

    python3 -m http.server 8000 --directory site

Open http://localhost:8000. The game starts automatically. First input enables
browser audio. Use X/Space for A, Z for B, arrows for the d-pad, Enter for START,
and right Shift/Backspace for SELECT. Onscreen controls support holding and
multiple simultaneous touches. Fullscreen keeps those controls visible.

The page needs to be served over HTTP/HTTPS; opening index.html as a local file
does not allow the emulator to fetch its assets.

## Publish on GitHub Pages

Both repository layouts have a workflow:

- If vroom is the repository root, use .github/workflows/pages.yml inside vroom.
- If this whole workspace is the repository, its root has
  .github/workflows/vroom-pages.yml.

Each workflow detects the layout and deploys only the site directory, not the
game sources or other games. All web asset paths are relative, so a project
URL such as https://USERNAME.github.io/REPOSITORY/ works.

1. Push to a GitHub repository's main or master branch.
2. In the repository's Settings → Pages, choose **GitHub Actions** as the source.
3. Run **Play Vroom on GitHub Pages** in Actions, or push a site change.
4. Open the URL reported by the deployment.

No GitHub repository or Pages deployment was created from this workspace.
The files and workflow are ready to push. GitHub requires workflows to live
at the repository root; the two copies cover the workspace and standalone
game layouts. Keep both copies synchronized if changing deployment behavior.

The workflow validates the committed game's version, ROM hash and emulator
hashes before uploading. It does not need Arduino or Emscripten on the runner.

Pages workflow reference:
https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages

## Update the game

Build the release using the existing game tools, then refresh the static copy:

    python3 tools/vroom.py build
    python3 tools/site.py
    python3 tools/site.py --check

Commit site/vroom.bin and site/game-config.js with the game update. Deployment
uses that exact committed release. Updating only C++ files does not rebuild or
deploy a different ROM automatically.

The ROM filename inside the emulator remains /roms/vroom.bin across releases,
so game updates retain saves. File request URLs include content hashes to
avoid mixing cached runtime or game versions.

## Saves

The original emulator persists flash saves in IndexedDB under
/chgame/saves/vroom.sav. The site waits for restored files before loading Vroom.
Browser progress is separate from the physical console and the upstream
emulator website. Reloading keeps progress on the same site/origin; private
browsing, clearing site data, or changing origins may remove that copy.

If browser storage cannot be restored, a visible message explains that play
can continue for the current session. The game itself still shows its normal
save feedback.

## Browser tests

From vroom:

    npm ci --prefix tools/web
    npx --prefix tools/web playwright install chromium
    npm test --prefix tools/web

The test starts a local server at a project-style subpath. It checks direct
boot, ignored ROM query strings, actual race inputs and finish payouts,
IndexedDB restore after reload, input release on blur, overlapping virtual
and physical controls, fullscreen, held START, mobile touch layout and missing-ROM errors. Screenshot\npixel checks also require the actual Vroom palette, catching blank rendering.
Screenshots are written to out/web/. These tests use fresh browser contexts
and do not modify personal game saves.

## Emulator provenance

The runtime files are unmodified copies from the user's reference:

https://joyrider3774.xyz/chgame_emulator/CHGame_Emulator.html

Source: https://github.com/joyrider3774/chgame_emulator

vendor/manifest.json records upstream build db57c51847c0 and SHA-256 hashes
of both files. The shell uses the same chg_web_load and FS.syncfs interfaces
as the original page. It disables the bootloader/SD setup, hides emulator
chrome, uses nearest-pixel integer scaling, and forwards only Vroom's controls. No loader UI or ROM URL from
the page query is passed to the emulator.

For a runtime update, replace the JavaScript and Wasm together, update the
manifest with their hashes/build provenance, run tools/site.py, and rerun
browser tests. Keep the emulator MIT, SDL zlib, Emscripten and Vroom license
notices distributed with the site.
