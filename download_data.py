#!/usr/bin/env python3
"""Download station_database source data for CardputerADV-DensyaNavi.

Downloads the repository ZIP from GitHub and extracts only:
  - out/
  - README.md
  - LICENSE.md

No data conversion or microSD writing is performed.
"""

from __future__ import annotations

import io
import os
import shutil
import sys
import tempfile
import urllib.error
import urllib.request
import zipfile
from pathlib import Path, PurePosixPath

SOURCE_ZIP_URL = (
    "https://github.com/Seo-4d696b75/station_database/"
    "archive/refs/heads/main.zip"
)
USER_AGENT = "CardputerADV-DensyaNavi-data-downloader/1.0"
KEEP_ROOT_FILES = {"README.md", "LICENSE.md"}
KEEP_DIRECTORY = "out"


def is_kept_path(relative_path: str) -> bool:
    """Return True for out/** and the selected root documentation files."""
    path = PurePosixPath(relative_path)
    if path.is_absolute() or ".." in path.parts:
        return False
    if not path.parts:
        return False
    if path.parts[0] == KEEP_DIRECTORY:
        return True
    return len(path.parts) == 1 and path.name in KEEP_ROOT_FILES


def download_zip() -> bytes:
    request = urllib.request.Request(
        SOURCE_ZIP_URL,
        headers={"User-Agent": USER_AGENT},
    )
    print("駅データのZIPをダウンロードしています...")
    try:
        with urllib.request.urlopen(request, timeout=60) as response:
            data = response.read()
    except (urllib.error.URLError, TimeoutError, OSError) as exc:
        raise RuntimeError(f"ダウンロードに失敗しました: {exc}") from exc

    if not data:
        raise RuntimeError("ダウンロードしたZIPが空です。")
    return data


def extract_selected(zip_bytes: bytes, destination: Path) -> None:
    """Extract selected files safely, stripping the ZIP's top-level folder."""
    try:
        archive = zipfile.ZipFile(io.BytesIO(zip_bytes))
    except zipfile.BadZipFile as exc:
        raise RuntimeError("取得したファイルは正しいZIPではありません。") from exc

    with archive:
        members = archive.infolist()
        if not members:
            raise RuntimeError("ZIPにファイルがありません。")

        # GitHub source ZIPs contain one top-level directory.
        roots = {
            PurePosixPath(info.filename).parts[0]
            for info in members
            if PurePosixPath(info.filename).parts
        }
        if len(roots) != 1:
            raise RuntimeError("ZIP内のフォルダ構成を認識できません。")
        root = next(iter(roots))

        extracted_any = False
        for info in members:
            raw = PurePosixPath(info.filename)
            if not raw.parts or raw.parts[0] != root or len(raw.parts) < 2:
                continue
            relative = PurePosixPath(*raw.parts[1:]).as_posix()
            if not is_kept_path(relative) or info.is_dir():
                continue

            # Reject suspicious archive paths before writing.
            if raw.is_absolute() or ".." in raw.parts:
                continue

            target = destination.joinpath(*PurePosixPath(relative).parts)
            target.parent.mkdir(parents=True, exist_ok=True)
            with archive.open(info, "r") as source, target.open("wb") as output:
                shutil.copyfileobj(source, output)
            extracted_any = True

        if not extracted_any:
            raise RuntimeError("必要なデータがZIP内に見つかりませんでした。")
        if not (destination / "out").is_dir():
            raise RuntimeError("ZIP内に out/ ディレクトリが見つかりません。")


def main() -> int:
    output_dir = Path(__file__).resolve().parent / "station_data"
    output_dir.parent.mkdir(parents=True, exist_ok=True)

    try:
        zip_bytes = download_zip()
        # Build in a temporary directory first so failed downloads/extractions
        # do not leave a half-written dataset behind.
        with tempfile.TemporaryDirectory(
            prefix="station_data_", dir=str(output_dir.parent)
        ) as temp_name:
            staged = Path(temp_name) / "new"
            staged.mkdir()
            extract_selected(zip_bytes, staged)

            backup = output_dir.with_name(output_dir.name + ".old")
            if backup.exists():
                shutil.rmtree(backup)
            if output_dir.exists():
                output_dir.rename(backup)
            try:
                staged.rename(output_dir)
            except OSError:
                if backup.exists() and not output_dir.exists():
                    backup.rename(output_dir)
                raise
            if backup.exists():
                shutil.rmtree(backup)

    except (RuntimeError, OSError, zipfile.BadZipFile) as exc:
        print(f"エラー: {exc}", file=sys.stderr)
        return 1

    print(f"完了しました: {output_dir}")
    print("取得した内容: station_data/out/、README.md、LICENSE.md")
    print("データ変換・microSDへの書き込みは行っていません。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
