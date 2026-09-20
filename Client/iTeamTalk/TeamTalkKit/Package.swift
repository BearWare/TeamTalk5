// swift-tools-version: 6.0

import PackageDescription

let package = Package(
    name: "TeamTalkKit",
    platforms: [
        .macOS(.v10_15),
        .iOS(.v18)
    ],
    products: [
        .library(
            name: "TeamTalkKit",
            targets: ["TeamTalkKit"])
    ],
    targets: [
        .binaryTarget(
            name: "TeamTalkNativeiOS",
            path: "Vendor/TeamTalkNativeiOS.xcframework"),
        .binaryTarget(
            name: "TeamTalkNativemacOS",
            path: "Vendor/TeamTalkNativemacOS.xcframework"),
        .target(
            name: "TeamTalkC",
            dependencies: [
                .target(name: "TeamTalkNativeiOS", condition: .when(platforms: [.iOS])),
                .target(name: "TeamTalkNativemacOS", condition: .when(platforms: [.macOS]))
            ],
            publicHeadersPath: "include"),
        .target(
            name: "TeamTalkKit",
            dependencies: ["TeamTalkC"]),
        .executableTarget(
            name: "TeamTalkKitExample",
            dependencies: ["TeamTalkKit"],
            path: "Examples/TeamTalkKitExample"),
        .testTarget(
            name: "TeamTalkKitTests",
            dependencies: ["TeamTalkKit", "TeamTalkC"])
    ],
    // Keep pre-Swift-6 language semantics; only the newer manifest API (needed
    // for `.v18`) requires tools-version 6.0. Adopting strict concurrency is a
    // separate, larger effort, not a side effect of this deployment-target bump.
    swiftLanguageModes: [.v5]
)
