"""Build the Rust firmware currently open in VS Code for STM32F446RE."""

import json
from pathlib import Path
import re
import shutil
import subprocess
import sys


def main():
    root = Path(__file__).resolve().parent.parent
    source = Path(sys.argv[1]).resolve() if len(sys.argv) == 2 else None
    if source is None or not source.is_file() or source.suffix != ".rs":
        sys.exit("실행할 Rust 펌웨어 파일(.rs)을 열고 F5를 누르세요.")
    if not source.is_relative_to(root / "src"):
        sys.exit("이 프로젝트의 src 안에 있는 Rust 펌웨어를 선택하세요.")

    # Keep generated Cargo metadata separate from the project's source files.
    generated = root / "target" / "active-f446"
    generated.mkdir(parents=True, exist_ok=True)
    manifest = (root / "Cargo.toml").read_text()
    manifest = re.sub(r"(?ms)^\[\[bin\]\].*?(?=^\[|\Z)", "", manifest)
    manifest = re.sub(r'^default-run\s*=.*$', 'default-run = "active-f446"', manifest, flags=re.M)
    manifest += (
        '\n[[bin]]\nname = "active-f446"\n'
        f"path = {json.dumps(str(source), ensure_ascii=False)}\n"
        'required-features = ["f446"]\ntest = false\nbench = false\n'
        '\n[workspace]\n'
    )
    (generated / "Cargo.toml").write_text(manifest)
    shutil.copy2(root / "build.rs", generated / "build.rs")
    shutil.copytree(root / "memory", generated / "memory", dirs_exist_ok=True)
    shutil.copy2(root / "Cargo.lock", generated / "Cargo.lock")
    print(f"F446 실행 파일: {source.relative_to(root)}", flush=True)
    result = subprocess.run(
        ["cargo", "build", "--manifest-path", str(generated / "Cargo.toml"),
         "--bin", "active-f446", "--no-default-features", "--features", "f446",
         "--target-dir", str(root / "target" / "rust-f446")],
        cwd=root,
    )
    sys.exit(result.returncode)


if __name__ == "__main__":
    main()
