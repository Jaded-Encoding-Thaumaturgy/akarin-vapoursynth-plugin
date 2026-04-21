import os
import shlex
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any

from hatchling.builders.hooks.plugin.interface import BuildHookInterface
from packaging import tags


class CustomHook(BuildHookInterface[Any]):
    source_dir = Path("build")
    target_dir = Path("vapoursynth/plugins/akarin")

    def initialize(self, version: str, build_data: dict[str, Any]) -> None:
        if self.source_dir.exists():
            shutil.rmtree(self.source_dir)

        build_data["pure_python"] = False
        build_data["tag"] = f"py3-none-{next(tags.platform_tags())}"

        meson_args = shlex.split(os.getenv("MESON_ARGS", ""))

        subprocess.run([sys.executable, "-m", "mesonbuild.mesonmain", "setup", "build", *meson_args], check=True)
        subprocess.run([sys.executable, "-m", "mesonbuild.mesonmain", "compile", "-C", "build"], check=True)

        self.target_dir.mkdir(parents=True, exist_ok=True)
        for file_path in self.source_dir.glob("*"):
            if file_path.is_file() and file_path.suffix in [".dll", ".so", ".dylib"]:
                shutil.copy2(file_path, self.target_dir)

        if sys.platform == "win32":
            dll_path = shutil.which("libzstd.dll")

            if not dll_path:
                for prefix in [os.getenv("MSYSTEM_PREFIX"), "C:/msys64/clang64", "C:/msys64/ucrt64"]:
                    if not prefix:
                        continue
                    if (candidate := Path(prefix) / "bin" / "libzstd.dll").exists():
                        dll_path = candidate
                        break

            if dll_path:
                shutil.copy2(dll_path, self.target_dir)
                print(f"Bundled dependency: {dll_path}", file=sys.stderr)
            else:
                print("Warning: Could not find libzstd.dll in PATH or common MSYS2 locations.", file=sys.stderr)

    def finalize(self, version: str, build_data: dict[str, Any], artifact_path: str) -> None:
        shutil.rmtree(self.target_dir.parent, ignore_errors=True)
