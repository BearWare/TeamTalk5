use std::env;
use std::path::PathBuf;

fn main() {
    println!("cargo:rustc-link-search=../TeamTalk_DLL/");
    println!("cargo:rustc-link-lib=dylib=TeamTalk5");
    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header("../TeamTalk_DLL/TeamTalk.h")
        // Finish the builder and generate the bindings.
        .rustified_enum("AudioFileFormat")
        .rustified_enum("AudioPreprocessorType")
        .rustified_enum("BanType")
        .rustified_enum("BitmapFormat")
        .rustified_enum("ChannelType")
        .rustified_enum("ClientError")
        .rustified_enum("ClientEvent")
        .rustified_enum("ClientFlag")
        .rustified_enum("Codec")
        .rustified_enum("DesktopKeyState")
        .rustified_enum("DesktopProtocol")
        .rustified_enum("FileTransferStatus")
        .rustified_enum("FourCC")
        .rustified_enum("MediaFileStatus")
        .rustified_enum("MixerControl")
        .rustified_enum("ServerLogEvent")
        .rustified_enum("SoundDeviceFeature")
        .rustified_enum("SoundLevel")
        .rustified_enum("SoundSystem")
        .rustified_enum("StreamType")
        .rustified_enum("Subscription")
        .rustified_enum("TTKeyTranslate")
        .rustified_enum("TTType")
        .rustified_enum("TextMsgType")
        .rustified_enum("UserRight")
        .rustified_enum("UserState")
        .rustified_enum("UserType")
        .default_macro_constant_type(bindgen::MacroTypeVariation::Signed) //FALSE and TRUE becomes u32 otherwise
        .derive_default(true) //Default zero-init of structs
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
 }