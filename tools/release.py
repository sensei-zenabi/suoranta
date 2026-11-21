#!/usr/bin/env python3
"""Refresh one application in the BUDOSTACK download catalog."""
import argparse
import hashlib
import json
import re
import subprocess
import tempfile
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo

SITE = Path(__file__).resolve().parents[1]
DOS_NAME = re.compile(r"[A-Z0-9_-]{1,8}(?:\.[A-Z0-9_-]{1,3})?")
APP_NAME = r"[A-Z][A-Z0-9]{0,7}"
VERSION = r"(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)"


def git(checkout, *args):
    result = subprocess.run(
        ["git", "-C", str(checkout), *args], capture_output=True, check=True
    )
    return result.stdout


def application_versions(checkout, commit):
    versions = {}
    for line in git(checkout, "show", f"{commit}:BUILD/VERSIONS.TXT").decode("ascii").splitlines():
        entry = re.fullmatch(rf"({APP_NAME}) ({VERSION})", line)
        if not entry or entry[1] in versions:
            raise ValueError(f"Invalid or duplicate entry in BUILD/VERSIONS.TXT: {line!r}")
        versions[entry[1]] = entry[2]

    files = git(checkout, "ls-tree", "--name-only", f"{commit}:BUILD").decode().splitlines()
    executables = {name[:-4] for name in files if name.upper().endswith(".EXE")}
    if versions.keys() != executables:
        missing = sorted(executables - versions.keys())
        extra = sorted(versions.keys() - executables)
        raise ValueError(f"BUILD/VERSIONS.TXT does not match BUILD/ executables (missing: {missing}, extra: {extra}).")
    return versions


def version_tuple(version):
    return tuple(map(int, version.split(".")))


def reviewed_assets(folder, app):
    assets = {}
    for path in sorted(folder.rglob("*")):
        relative = path.relative_to(folder)
        if path.is_symlink() or any(not DOS_NAME.fullmatch(p) for p in relative.parts):
            raise ValueError(f"Expected an uppercase DOS 8.3 path: {relative}")
        if path.is_file():
            assets[f"{app}/{relative.as_posix()}"] = path.read_bytes()
    return assets


def runtime_documents(binary):
    if b"Open Watcom C/C++16 Run-Time system" not in binary:
        return {}
    if b"Open Watcom runtime source code is available" not in binary:
        raise ValueError("Use a COMMANDS build with the Open Watcom source notice embedded.")
    return {
        name: (SITE / "vendor" / name).read_bytes()
        for name in ("NOTICE.TXT", "WATCOM.TXT")
    }


def release(args):
    app = args.application.upper()
    if not re.fullmatch(APP_NAME, app):
        raise ValueError("Use the DOS application name, for example WRITE.")

    archive = SITE / "releases" / f"{app.lower()}.zip"
    index = SITE / "index.html"
    page = index.read_text(encoding="utf-8")
    marker = re.compile(
        rf'(data-release="{app.lower()}" data-asset=")[^"]+(" data-version=")([^"]+)(">)(?:Version [0-9.]+ · )?\d+ KiB(</span>)'
    )
    entries = list(marker.finditer(page))
    if len(entries) != 1 or not archive.is_file():
        raise ValueError(f"{app} must have one catalog entry and an existing ZIP.")

    executable = f"{app}.EXE"
    commit = git(args.commands, "rev-parse", "--verify", "--end-of-options", f"{args.ref}^{{commit}}").decode().strip()
    version = application_versions(args.commands, commit).get(app)
    if version is None:
        raise ValueError(f"{app} is missing from BUILD/VERSIONS.TXT.")
    binary = git(args.commands, "show", f"{commit}:BUILD/{executable}")
    if not binary.startswith(b"MZ"):
        raise ValueError(f"BUILD/{executable} is not an MZ executable.")

    previous_zip = archive.read_bytes()
    with ZipFile(archive) as previous:
        previous_files = {name: previous.read(name) for name in previous.namelist()}
    assets = {
        name: data for name, data in previous_files.items()
        if name.startswith(f"{app}/") and not name.endswith("/")
    }
    if args.demo_dir is not None:
        if not args.demo_dir.is_dir():
            raise ValueError("--demo-dir must point to the reviewed companion folder.")
        assets = reviewed_assets(args.demo_dir, app)

    files = {
        executable: binary,
        "VERSION.TXT": f"{executable} {version}\r\n".encode("ascii"),
        **runtime_documents(binary),
        **assets,
    }
    previous_version = entries[0][3]
    already_packaged = False
    if "VERSION.TXT" in previous_files:
        if version_tuple(version) < version_tuple(previous_version):
            raise ValueError(f"{app} {version} is older than the published version {previous_version}.")
        if version == previous_version:
            if files != previous_files:
                raise ValueError(f"Bump {app} in BUILD/VERSIONS.TXT before changing package contents.")
            already_packaged = True

    with tempfile.TemporaryDirectory(dir=SITE, prefix=".release-") as temporary:
        staged_zip = Path(temporary) / archive.name
        staged_index = Path(temporary) / index.name
        zip_bytes = previous_zip
        if not already_packaged:
            with ZipFile(staged_zip, "w") as output:
                output.comment = json.dumps({"version": version, "commands_commit": commit}).encode()
                for name, data in files.items():
                    output.writestr(ZipInfo(name), data, compress_type=ZIP_DEFLATED, compresslevel=9)

            with ZipFile(staged_zip) as output:
                if {name: output.read(name) for name in output.namelist()} != files:
                    raise ValueError("The staged ZIP failed verification.")
            zip_bytes = staged_zip.read_bytes()

        size = (len(zip_bytes) + 1023) // 1024
        digest = hashlib.sha256(zip_bytes).hexdigest()
        asset = f"{app.lower()}-{digest}.zip"
        updated = marker.sub(
            lambda m: f"{m[1]}{asset}{m[2]}{version}{m[4]}Version {version} · {size} KiB{m[5]}", page
        )
        if already_packaged and page == updated:
            print(f"{app} {version} is already packaged.")
            return

        staged_index.write_text(updated, encoding="utf-8")
        if not already_packaged:
            staged_zip.replace(archive)
        try:
            staged_index.replace(index)
        except OSError:
            if not already_packaged:
                staged_zip.write_bytes(previous_zip)
                staged_zip.replace(archive)
            raise

    print(f"{app} {version} from {commit[:12]} → releases/{archive.name} ({size} KiB)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("commands", type=Path, help="Path to a local COMMANDS Git checkout")
    parser.add_argument("application", help="DOS executable name without .EXE")
    parser.add_argument("--ref", default="main", help="Source branch, tag, or commit (default: main)")
    parser.add_argument("--demo-dir", type=Path, help="Replace companion assets with this reviewed folder")
    args = parser.parse_args()
    try:
        release(args)
    except subprocess.CalledProcessError as error:
        parser.exit(1, error.stderr.decode(errors="replace").strip() + "\n")
    except (OSError, ValueError, KeyError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
