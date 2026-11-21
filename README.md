# BUDOSTACK website

GitHub Pages serves this static site at [www.budostack.com](https://www.budostack.com/). `index.html` is the About homepage. `applications.html` is the Applications tab. `games/index.html` is the Games tab. `news.html` is the News tab and renders `news.md`.

Preview from the website repository with `python3 -m http.server 8000`, then open <http://localhost:8000/>.

## Release a COMMANDS application

1. Bump the app's version in `COMMANDS/BUILD/VERSIONS.TXT` when its distributed files change. Merge the tested DOS build and version list into COMMANDS `main`. Fetch the release commit or tag in your local COMMANDS checkout.
2. In a branch of this website repository, run the release command with the app name and source ref. Without `--ref`, the tool uses the local COMMANDS `main`.

   ```sh
   python3 tools/release.py ../COMMANDS WRITE --ref origin/main
   ```

3. Extract the resulting `releases/write.zip` and run it in DOSBox. Update `screenshots/write.png` if the old screenshot no longer represents the application. Add the release notes to `news.md` and preview both tabs.
4. Commit the ZIP, the updated `applications.html`, and any screenshot or News edits in the same PR. Merge to the GitHub Pages branch to publish. Confirm the download from the live site. The GitHub Actions workflow uploads new ZIPs to the `budostack-packages` release after the merge. Check the workflow completed before verifying the upload date and download count.

The script reads the version and executable from the same Git commit. It keeps the currently published companion files, writes `VERSION.TXT` inside the ZIP, updates the visible catalog version, package hash, and size, and records the full COMMANDS commit in the ZIP comment. It rejects changed contents at the same version and version rollbacks. For binaries linked with Open Watcom, the script checks for the embedded source notice and includes the runtime license and attribution. To replace companion files, pass `--demo-dir path/to/reviewed/WRITE`. The folder should contain only public, generic files with uppercase DOS 8.3 names. Keep the public backup and library examples when updating FLOW and DBASE, unless replacing them with reviewed generic examples.

## Release a GAMES title

1. Build and test from the intended GAMES release commit, then run its packager with `python3 tools/package_game.py DUNGEON --output /tmp/games-release`.
2. Copy the versioned ZIP to `releases/`. Keep earlier versions available. Update the game’s download link, version, and size in `games/index.html`. Set `data-asset` to the ZIP stem, a hyphen, the ZIP’s full SHA-256 (from `sha256sum`), and `.zip`. Upload screenshots that represent the release and announce it in `news.md`.
3. Preview the Games page and test the download from a clean DOSBox install. Merge the website changes to `main` and verify the live page and ZIP. The upload workflow adds the immutable ZIP to `budostack-packages` and the page shows its upload date once the asset is available.

## Download dates and click counts

The catalog shows the linked GitHub release asset's upload date in Helsinki time.
Release ZIPs keep content-based filenames; older assets remain available.

The displayed counter measures trusted download-button activations (mouse, touch,
keyboard, and middle click), including repeated clicks. It does not measure unique
people or completed downloads. Page views and direct ZIP requests do not increment
it. JavaScript must be enabled; network failures or blocking can prevent recording.
Downloads proceed even when the counter service is unavailable.

`click-counters.mjs` posts one random event ID per activation to the BUDOSTACK
counter service. Retrying an event cannot double count. Counts use stable application
IDs (`data-release`), so replacing a ZIP does not reset them. New click counters
start at zero; historical GitHub download counts are not imported.

The counter service is maintained in the Sites source repository for project
`appgprj_6afc8ee34b8c81919e9693cb5e021091`, at
https://budostack-download-clicks.firmcheetah8.chatgpt.site.
Its D1 database stores only random click event IDs and application IDs. It has no
public reset endpoint. Do not change application IDs or replace the database when
publishing new packages. `GET /v1/clicks` is read-only; `POST /v1/clicks/{app}`
records an event from an allowed website origin.
