# BUDOSTACK website

GitHub Pages serves this static site at [www.budostack.com](https://www.budostack.com/). `index.html` is the Applications & Install tab. `news.html` is the News tab and renders `news.md`.

Preview from the website repository with `python3 -m http.server 8000`, then open <http://localhost:8000/>.

## Release a COMMANDS application

1. Merge and test the ready-to-run DOS build in COMMANDS. Fetch the release commit or tag in your local COMMANDS checkout.
2. In a branch of this website repository, run the release command. Substitute the app, version, and source ref. Without `--ref`, the tool uses the local COMMANDS `main`.

   ```sh
   python3 tools/release.py ../COMMANDS WRITE 1.2.0 --ref origin/main
   ```

3. Extract the resulting `releases/write.zip` and run it in DOSBox. Update `screenshots/write.png` if the old screenshot no longer represents the application. Add the release notes to `news.md` and preview both tabs.
4. Commit the ZIP, the updated `index.html`, and any screenshot or News edits in the same PR. Merge to the GitHub Pages branch to publish. Confirm the download from the live site. The GitHub Actions workflow uploads new ZIPs to the `budostack-packages` release after the merge. Check the workflow completed before verifying the upload date and download count.

The script reads the executable from the selected Git commit. It keeps the currently published companion files, updates the catalog version, package hash, and size, and records the full COMMANDS commit in the ZIP comment. For binaries linked with Open Watcom, the script checks for the embedded source notice and includes the runtime license and attribution. To replace companion files, pass `--demo-dir path/to/reviewed/WRITE`. The folder should contain only public, generic files with uppercase DOS 8.3 names. Keep the public backup and library examples when updating FLOW and DBASE, unless replacing them with reviewed generic examples.

## Download dates and counts

Each uploaded ZIP has a content-based filename. The catalog displays the upload
date of the linked asset in Helsinki time and sums GitHub's download counts for
every published asset of that application. Keep older assets in the package
archive to preserve those totals. Re-running the publisher skips ZIPs that have
already been uploaded.

Counts begin with GitHub release downloads. Direct downloads of the GitHub
Pages copies, including downloads before this change, are outside those totals.
The Pages copy remains available if an asset is awaiting upload or JavaScript
is disabled. When GitHub's statistics API is unavailable, a published ZIP can
still be downloaded using its asset URL.
