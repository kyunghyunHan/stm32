use std::{env, fs, path::PathBuf};

fn main() {
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-changed=memory/f446.x");
    println!("cargo:rerun-if-changed=memory/f767.x");

    let memory = match (
        env::var_os("CARGO_FEATURE_F446").is_some(),
        env::var_os("CARGO_FEATURE_F767").is_some(),
    ) {
        (true, false) => "memory/f446.x",
        (false, true) => "memory/f767.x",
        _ => panic!("Select exactly one MCU: --no-default-features --features f446 (or f767)"),
    };

    let target = env::var("TARGET").unwrap();
    assert_eq!(
        target, "thumbv7em-none-eabihf",
        "Build for STM32 using thumbv7em-none-eabihf"
    );

    let out = PathBuf::from(env::var_os("OUT_DIR").unwrap());
    fs::copy(memory, out.join("memory.x")).expect("Cannot copy MCU memory layout");
    println!("cargo:rustc-link-search={}", out.display());
}
