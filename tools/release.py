#!/usr/bin/env python3
"""Refresh one application in the BUDOSTACK download catalog."""
import argparse
import json
import re
import subprocess
import tempfile
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo

SITE = Path(__file__).resolve().parents[1]
DOS_NAME = re.compile(r"[A-Z0-9_-]{1,8}(?:\.[A-Z0-9_-]{1,3})?")


def git(checkout, *args):
    result = subprocess.run(
        ["git", "-C", str(checkout), *args], capture_output=True, check=True
    )
    return result.stdout


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
    if not re.fullmatch("[A-Z][A-Z0-9]{0,7}", app):
        raise ValueError("Use the DOS application name, for example WRITE.")
    if not re.fullmatch(r"[0-9][A-Za-z0-9._-]{0,31}", args.version):
        raise ValueError("Use a release label such as 1.2.0 or 2026.11.15.")

    archive = SITE / "releases" / f"{app.lower()}.zip"
    index = SITE / "index.html"
    page = index.read_text(encoding="utf-8")
    marker = re.compile(
        rf'(data-release="{app.lower()}">)([^<]+) · \d+ KiB(</span>)'
    )
    entries = list(marker.finditer(page))
    if len(entries) != 1 or not archive.is_file():
        raise ValueError(f"{app} must have one catalog entry and an existing ZIP.")

    executable = f"{app}.EXE"
    commit = git(args.commands, "rev-parse", "--verify", "--end-of-options", f"{args.ref}^{{commit}}").decode().strip()
    binary = git(args.commands, "show", f"{commit}:BUILD/{executable}")
    if not binary.startswith(b"MZ"):
        raise ValueError(f"BUILD/{executable} is not an MZ executable.")

    previous_zip = archive.read_bytes()
    with ZipFile(archive) as previous:
        assets = {
            name: previous.read(name)
            for name in previous.namelist()
            if name.startswith(f"{app}/") and not name.endswith("/")
        }
        previous_binary = previous.read(executable)
        previous_comment = previous.comment
    previous_assets = assets
    if args.demo_dir is not None:
        if not args.demo_dir.is_dir():
            raise ValueError("--demo-dir must point to the reviewed companion folder.")
        assets = reviewed_assets(args.demo_dir, app)

    if entries[0][2] == args.version and (
        previous_binary != binary or previous_assets != assets
    ):
        raise ValueError("Use a new release label when changing package contents.")

    provenance = json.dumps({"version": args.version, "commands_commit": commit}).encode()
    files = {executable: binary, **runtime_documents(binary), **assets}
    with tempfile.TemporaryDirectory(dir=SITE, prefix=".release-") as temporary:
        staged_zip = Path(temporary) / archive.name
        staged_index = Path(temporary) / index.name
        with ZipFile(staged_zip, "w") as output:
            output.comment = provenance
            for name, data in files.items():
                output.writestr(ZipInfo(name), data, compress_type=ZIP_DEFLATED, compresslevel=9)

        with ZipFile(staged_zip) as output:
            if output.testzip() is not None or {name: output.read(name) for name in output.namelist()} != files:
                raise ValueError("The staged ZIP failed verification.")

        size = (staged_zip.stat().st_size + 1023) // 1024
        updated = marker.sub(lambda m: f"{m[1]}{args.version} · {size} KiB{m[3]}", page)
        if previous_comment == provenance and previous_zip == staged_zip.read_bytes() and page == updated:
            print(f"{app} {args.version} is already packaged from {commit[:12]}.")
            return

        staged_index.write_text(updated, encoding="utf-8")
        staged_zip.replace(archive)
        try:
            staged_index.replace(index)
        except OSError:
            staged_zip.write_bytes(previous_zip)
            staged_zip.replace(archive)
            raise

    print(f"{app} {args.version} from {commit[:12]} → releases/{archive.name} ({size} KiB)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("commands", type=Path, help="Path to a local COMMANDS Git checkout")
    parser.add_argument("application", help="DOS executable name without .EXE")
    parser.add_argument("version", help="Public release label, for example 1.2.0")
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
